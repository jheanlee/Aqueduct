mod domain;
mod error;
mod state;
mod core;
mod message;
mod console;
mod opt;
mod orm;
mod auth;

use std::collections::VecDeque;
use std::sync::Arc;
use tokio::sync::{Mutex, Notify};
use axum::routing::{get, post};
use clap::Parser;
use sea_orm::{DatabaseConnection};
use tokio::net::UnixStream;
use crate::console::{console, Code, Level};
use crate::core::core::{client_thread_func, connect_core, core_io_read_thread_func, get_status, status_thread_func};
use crate::domain::clients::{get_client_db, get_connected_clients, update_clients};
use crate::domain::login::{authenticate, check_web_user, modify_web_user};
use crate::domain::tokens::{check_token, delete_token_item, get_tokens, modify_token_item};
use crate::orm::connection::connect_database;
use crate::state::{CoreStatus};

pub struct TokenChannel {
  pub token_queue: Mutex<VecDeque<(String, String)>>,
  pub notify: Notify
}

pub struct SharedResources {
  pub verbose_level: u8,
  pub daemon_mode: bool,
  pub socket_core: Arc<Mutex<UnixStream>>,
  pub database_connection: Option<DatabaseConnection>,
  pub token_channel: TokenChannel,
}

static SHARED_CELL: once_cell::sync::OnceCell<SharedResources> = once_cell::sync::OnceCell::new();

#[tokio::main]
async fn main() {
  let args = crate::opt::Args::parse();

  let shared_resources = SharedResources {
    verbose_level: args.verbose,
    daemon_mode: args.daemon_mode,
    socket_core: Arc::new(Mutex::new(connect_core().await.unwrap_or_else(|e| {
      console(Level::Critical, Code::SockConnectionFailed, e.to_string().as_str(), "main");
      panic!();
    }))),
    database_connection: Some(connect_database(args.database).await.unwrap_or_else(|e| {
      console(Level::Critical, Code::DatabaseConnectionFailed, e.to_string().as_str(), "main");
      panic!();
    })),
    token_channel: TokenChannel{ token_queue: Mutex::new(VecDeque::new()), notify: Notify::new() },
  };

  SHARED_CELL.set(shared_resources).unwrap_or_else(|_| {
    console(Level::Critical, Code::SharedResourcesSetFailed, "", "main");
    panic!();
  });

  let state = state::AppState {
    socket_core: Arc::clone(&SHARED_CELL.get().unwrap().socket_core),
    core_status: Arc::new(Mutex::new((false, CoreStatus{ uptime: 0, tunnel_service_up: false, api_service_up: false, connected_clients: 0 }))),
    clients: Arc::new(Mutex::new((false, vec![]))),
  };

  let frontend_server_dir = tower_http::services::ServeDir::new("frontend");

  let arc_state = Arc::new(state);
  let status_thread = tokio::spawn(status_thread_func(Arc::clone(&arc_state.socket_core)));
  let client_thread = tokio::spawn(client_thread_func(Arc::clone(&arc_state.socket_core)));
  let core_io_read_thread = tokio::spawn(core_io_read_thread_func(Arc::clone(&arc_state)));

  let app = axum::Router::new()
    .route("/api/status", get(get_status))
    .route("/api/clients/connected", get(get_connected_clients))
    .route("/api/clients/update", post(update_clients))
    .route("/api/clients/db", get(get_client_db))
    .route("/api/tokens/list", get(get_tokens))
    .route("/api/tokens/check", get(check_token))
    .route("/api/tokens/modify", post(modify_token_item))
    .route("/api/tokens/delete", post(delete_token_item))
    .route("/api/users/check", get(check_web_user))
    .route("/api/users/modify", post(modify_web_user))
    .route("/api/users/auth", post(authenticate))
    .fallback_service(frontend_server_dir)
    .with_state(arc_state);

  let listener = tokio::net::TcpListener::bind("0.0.0.0:30331").await.unwrap_or_else(|e| {
    console(Level::Critical, Code::SockBindFailed, e.to_string().as_str(), "main");
    panic!();
  });
  axum::serve(listener, app).await.unwrap_or_else(|e| {
    console(Level::Critical, Code::SockServeFailed, e.to_string().as_str(), "main");
    panic!();
  });
}
