mod domain;
mod error;
mod state;
mod core;
mod message;
mod console;
mod opt;
mod orm;

use std::sync::Arc;
use tokio::sync::Mutex;
use axum::routing::{get, post};
use clap::Parser;
use sea_orm::{DatabaseConnection};
use crate::console::{console, Code, Level};
use crate::core::core::{client_thread_func, connect_core, core_io_read_thread_func, get_status, status_thread_func};
use crate::domain::clients::{get_client_db, get_connected_clients, update_clients};
use crate::orm::connection::connect_database;
use crate::state::{CoreStatus};

pub struct SharedResources {
  pub verbose_level: u8,
  pub daemon_mode: bool,
  pub database_connection: Option<DatabaseConnection>
}

static SHARED_CELL: once_cell::sync::OnceCell<SharedResources> = once_cell::sync::OnceCell::new();

#[tokio::main]
async fn main() {
  let args = crate::opt::Args::parse();

  let shared_resources = SharedResources {
    verbose_level: args.verbose,
    daemon_mode: args.daemon_mode,
    database_connection: Some(connect_database(args.database).await.unwrap_or_else(|e| {
      console(Level::Critical, Code::DatabaseConnectionFailed, e.to_string().as_str(), "main");
      std::process::exit(1);
    }))
  };

  SHARED_CELL.set(shared_resources).unwrap_or_else(|_| {
    console(Level::Critical, Code::SharedResourcesSetFailed, "", "main");
    std::process::exit(1);
  });

  let state = state::AppState {
    socket_core: Arc::new(Mutex::new(connect_core().await.unwrap_or_else(|e| {
      console(Level::Critical, Code::SockConnectionFailed, e.to_string().as_str(), "main");
      std::process::exit(1);
    }))),
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
    // .route("/api/token/list", get())
    // .route("/api/token/modify", post())
    .fallback_service(frontend_server_dir)
    .with_state(arc_state);

  let listener = tokio::net::TcpListener::bind("0.0.0.0:30331").await.unwrap_or_else(|e| {
    console(Level::Critical, Code::SockBindFailed, e.to_string().as_str(), "main");
    std::process::exit(1);
  });
  axum::serve(listener, app).await.unwrap_or_else(|e| {
    console(Level::Critical, Code::SockServeFailed, e.to_string().as_str(), "main");
    std::process::exit(1);
  });
}
