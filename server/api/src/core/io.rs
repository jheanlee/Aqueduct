use std::collections::HashMap;
use std::sync::Arc;
use tokio::net::UnixStream;
use axum::http::StatusCode;
use tokio::sync::Mutex;
use crate::console::{console, Code, Level};
use crate::core::connection::reconnect;
use crate::error::{ApiError};
use crate::state::{AppState, ConnectedClients, CoreStatus};
use crate::message::{api_message_type, Message};
use crate::SHARED_CELL;

#[derive(serde::Serialize, serde::Deserialize)]
pub struct NewToken {
  token: String,
  hashed: String,
}

pub async fn core_io_read_thread_func(state: Arc<AppState>) -> Result<(), ()> {
  let mut buffer = ['\0' as u8; 32768];
  let mut message = Message{ message_type: '\0' as u8, message_string: "".to_owned() };
  
  loop {
    loop {
      let state_clone = Arc::clone(&state);
      let socket_core_guard = state_clone.socket_core.lock().await;

      let s = tokio::select! {
        s = socket_core_guard.readable() => Some(s),
        _ = tokio::time::sleep(std::time::Duration::from_millis(100)) => None
      };
      
      if let Some(r) = s {
        match r { 
          Ok(_) => {
            match socket_core_guard.try_read(&mut buffer) {
              //  pipe closed
              Ok(n) if n == 0 => return Ok(()),
              //  data read
              Ok(_n) => break,
              //  no data available
              Err(ref e) if e.kind() == tokio::io::ErrorKind::WouldBlock => continue,
              //  closed
              Err(ref e) if e.kind() == tokio::io::ErrorKind::BrokenPipe => {
                if reconnect(Arc::clone(&state_clone.socket_core)).await.is_err() {
                  console(Level::Critical, Code::SockConnectionLost, "", "core::core_io_read");
                  panic!()
                }
              },
              //  other errors
              Err(e) => {
                console(Level::Error, Code::SockReadError, format!("{}", e).as_str(), "core::core_io_read");
                return Err(())
              },
            }
          },
          Err(e) => {
            console(Level::Error, Code::SockSelectError, format!("{}", e).as_str(), "core::core_io_read");
          }
        }
      }
    }

    if message.load(&buffer).is_err() {
      console(Level::Info, Code::MessageInvalid, "", "core::core_io_read");
    }
    
    let state_clone = Arc::clone(&state);
    
    match message.message_type {
      api_message_type::API_HEARTBEAT => {
        send_heartbeat_message(Arc::clone(&state_clone.socket_core)).await.unwrap_or_else(|_| {
          console(Level::Warning, Code::SockSendError, "", "core::core_io_read::heartbeat");
          StatusCode::INTERNAL_SERVER_ERROR
        });
      },
      api_message_type::API_GET_SERVICE_INFO => {
        let mut flag_error = false;
        let new_status = serde_json::from_str::<CoreStatus>(message.message_string.as_str()).unwrap_or_else(|e| {
          console(Level::Error, Code::ApiDumpFailed, e.to_string().as_str(), "core::core_io_read");
          flag_error = true;
          CoreStatus {
            api_service_up: false,
            connected_clients: 0,
            tunnel_service_up: false,
            uptime: 0,
          }
        });
        state_clone.set_core_status(flag_error, new_status).await;
      },
      api_message_type::API_GET_CURRENT_CLIENTS => {
        let mut flag_error = false;
        let new_clients = serde_json::from_str::<HashMap<String, Vec<ConnectedClients>>>(message.message_string.as_str()).unwrap_or_else(|e| {
          console(Level::Error, Code::ApiDumpFailed, e.to_string().as_str(), "core::core_io_read");
          flag_error = true;
          HashMap::new()
        });
        state_clone.set_clients(flag_error, new_clients["clients"].clone()).await;
      },
      api_message_type::API_GENERATE_NEW_TOKEN => {
        let new_token = serde_json::from_str::<NewToken>(message.message_string.as_str()).unwrap_or_else(|e| {
          console(Level::Error, Code::ApiDumpFailed, e.to_string().as_str(), "core::core_io_read");
          NewToken{ token: "".to_string(), hashed: "".to_string() }
        });

        SHARED_CELL.get().unwrap().token_channel.token_queue.lock().await.push_back((new_token.token, new_token.hashed));
        SHARED_CELL.get().unwrap().token_channel.notify.notify_one();
      },
      _ => {}
    }

  }
}

pub async fn send_heartbeat_message(socket_core: Arc<Mutex<UnixStream>>) -> Result<StatusCode, ApiError> {
  let mut message = Message{ message_type: api_message_type::API_HEARTBEAT, message_string: "".to_owned() };

  loop {
    socket_core.lock().await.writable().await?;
    match socket_core.lock().await.try_write(message.dump()?.as_ref()) {
      Ok(_n) => break,
      Err(ref e) if e.kind() == tokio::io::ErrorKind::WouldBlock => continue,
      Err(ref e) if e.kind() == tokio::io::ErrorKind::BrokenPipe => return Ok(StatusCode::SERVICE_UNAVAILABLE),
      Err(e) => return Err(e.into()),
    }
  }

  Ok(StatusCode::OK)
}

pub async fn status_thread_func(socket_core: Arc<Mutex<UnixStream>>) {
  loop {
    let status = send_status_message(Arc::clone(&socket_core)).await;
    match status {
      Ok(ref c) if c == &StatusCode::SERVICE_UNAVAILABLE => {
        if reconnect(Arc::clone(&socket_core)).await.is_err() {
          console(Level::Critical, Code::SockConnectionLost, "", "core::status_thread");
          panic!()
        }
      },
      Ok(_) => {},
      Err(_) => break,
    }
    tokio::time::sleep(std::time::Duration::from_millis(5000)).await;
  }
}

pub async fn send_status_message(socket_core: Arc<Mutex<UnixStream>>) -> Result<StatusCode, ApiError> {
  let mut message = Message{ message_type: api_message_type::API_GET_SERVICE_INFO, message_string: "".to_owned() };

  loop {
    socket_core.lock().await.writable().await?;
    match socket_core.lock().await.try_write(message.dump()?.as_ref()) {
      Ok(_n) => break,
      Err(ref e) if e.kind() == tokio::io::ErrorKind::WouldBlock => continue,
      Err(ref e) if e.kind() == tokio::io::ErrorKind::BrokenPipe => return Ok(StatusCode::SERVICE_UNAVAILABLE),
      Err(e) => return Err(e.into()),
    }
  }

  Ok(StatusCode::OK)
}


pub async fn client_thread_func(socket_core: Arc<Mutex<UnixStream>>) {
  loop {
    let status = send_client_message(Arc::clone(&socket_core)).await;
    match status {
      Ok(ref c) if c == &StatusCode::SERVICE_UNAVAILABLE => break,
      Ok(_) => {},
      Err(_) => break,
    }
    tokio::time::sleep(std::time::Duration::from_millis(60000)).await;
  }
}

pub async fn send_client_message(socket_core: Arc<Mutex<UnixStream>>) -> Result<StatusCode, ApiError> {
  let mut message = Message{ message_type: api_message_type::API_GET_CURRENT_CLIENTS, message_string: "".to_owned() };

  loop {
    socket_core.lock().await.writable().await?;
    match socket_core.lock().await.try_write(message.dump()?.as_ref()) {
      Ok(_n) => break,
      Err(ref e) if e.kind() == tokio::io::ErrorKind::WouldBlock => continue,
      Err(ref e) if e.kind() == tokio::io::ErrorKind::BrokenPipe => return Ok(StatusCode::SERVICE_UNAVAILABLE),
      Err(e) => return Err(e.into()),
    }
  }

  Ok(StatusCode::OK)
}

pub async fn send_new_token_message(socket_core: Arc<Mutex<UnixStream>>) -> Result<StatusCode, ApiError> {
  let mut message = Message{ message_type: api_message_type::API_GENERATE_NEW_TOKEN, message_string: "".to_owned() };

  loop {
    socket_core.lock().await.writable().await?;
    match socket_core.lock().await.try_write(message.dump()?.as_ref()) {
      Ok(_n) => break,
      Err(ref e) if e.kind() == tokio::io::ErrorKind::WouldBlock => continue,
      Err(ref e) if e.kind() == tokio::io::ErrorKind::BrokenPipe => return Ok(StatusCode::SERVICE_UNAVAILABLE),
      Err(e) => return Err(e.into()),
    }
  }

  Ok(StatusCode::OK)
}