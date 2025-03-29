mod domain;
mod error;
mod state;
mod core_io;
mod message;
mod console;

use std::sync::Arc;
use tokio::sync::Mutex;
use axum::routing::{get, post};
use crate::core_io::core_io::{client_thread_func, connect_core, core_io_read_thread_func, get_connected_clients, get_status, status_thread_func, update_clients};
use crate::state::{ConnectedClients, CoreStatus};

#[tokio::main]
async fn main() {
  let state = state::AppState {
    socket_core: Arc::new(Mutex::new(connect_core().await.unwrap())),
    core_status: Arc::new(Mutex::new(CoreStatus{ uptime: 0, tunnel_service_up: false, api_service_up: false, connected_clients: 0 })),
    clients: Arc::new(Mutex::new(vec![])),
  };

  let arc_state = Arc::new(state);
  let status_thread = tokio::spawn(status_thread_func(Arc::clone(&arc_state.socket_core)));
  let client_thread = tokio::spawn(client_thread_func(Arc::clone(&arc_state.socket_core)));
  let core_io_read_thread = tokio::spawn(core_io_read_thread_func(Arc::clone(&arc_state)));

  let app = axum::Router::new()
    .route("/status", get(get_status))
    .route("/client/connected", get(get_connected_clients))
    .route("/client/update", post(update_clients))
    // .route("/client/db", get())
    // .route("/token/list", get())
    // .route("/token/modify", post())
    .with_state(arc_state)
    .layer(tower_http::cors::CorsLayer::permissive());
  let listener = tokio::net::TcpListener::bind("0.0.0.0:30331").await.unwrap();
  axum::serve(listener, app).await.unwrap();
}
