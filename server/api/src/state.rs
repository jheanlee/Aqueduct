use std::sync::Arc;
use tokio::net::UnixStream;

#[derive(serde::Serialize, serde::Deserialize, Clone, Copy, Debug)]
pub struct CoreStatus {
  pub api_service_up: bool,
  pub connected_clients: u64,
  pub tunnel_service_up: bool,
  pub uptime: u64,
}

impl CoreStatus {
  pub fn to_string(&self) -> Result<String, serde_json::Error> {
    serde_json::to_string(self)
  }
}

#[derive(serde::Serialize, serde::Deserialize, Clone, Debug)]
pub struct ConnectedClients {
  pub key: u64,
  pub ip_addr: String,
  pub port: i32,
  #[serde(rename="type")]
  pub client_type: u8,
  pub stream_port: i32,
  pub user_addr: String,
  pub user_port: i32,
  pub main_addr: String,
  pub main_port: i32,
}

#[derive(Clone)]
pub struct AppState {
  pub socket_core: Arc<tokio::sync::Mutex<UnixStream>>,
  pub core_status: Arc<tokio::sync::Mutex<CoreStatus>>,
  pub clients: Arc<tokio::sync::Mutex<Vec<ConnectedClients>>>,
}

impl AppState {
  pub async fn set_core_status(self: Arc<Self>, new_status: CoreStatus) {
    let this = Arc::clone(&self);
    let mut status = this.core_status.lock().await;
    status.uptime = new_status.uptime;
    status.connected_clients = new_status.connected_clients;
    status.api_service_up = new_status.api_service_up;
    status.tunnel_service_up = new_status.tunnel_service_up;
  }

  pub async fn set_clients(self: Arc<Self>, new_clients: Vec<ConnectedClients>) {
    let this = Arc::clone(&self);
    this.clients.lock().await.clear();
    this.clients.lock().await.extend(new_clients);
  }
}
