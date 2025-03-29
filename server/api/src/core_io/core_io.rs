use std::collections::HashMap;
use std::sync::Arc;
use axum::body::Body;
use tokio::net::UnixStream;
use axum::extract::State;
use axum::http;
use axum::http::{Response, StatusCode};
use axum::response::IntoResponse;
use tokio::sync::Mutex;
use crate::error::{ApiError, MessageError};
use crate::state::{AppState, ConnectedClients, CoreStatus};
use crate::message::{api_message_type, Message};

pub async fn connect_core() -> Result<UnixStream, ApiError> {
  Ok(UnixStream::connect("/tmp/sphere-linked-server-core.sock").await?)
}

pub async fn core_io_read_thread_func(state: Arc<AppState>) -> Result<(), ()>{
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
        if let Ok(_) = r {
          match socket_core_guard.try_read(&mut buffer) {
            //  pipe closed
            Ok(n) if n == 0 => return Ok(()),
            //  data read
            Ok(_n) => break,
            //  no data available
            Err(ref e) if e.kind() == tokio::io::ErrorKind::WouldBlock => continue,
            //  closed
            Err(ref e) if e.kind() == tokio::io::ErrorKind::BrokenPipe => return Err(()), //  TODO log + kill flag
            //  other errors
            Err(e) => {
              println!("{}", e);
              return Err(())
            }, //  TODO log + kill flag,
          }
        } else {
          println!("select error")
          // log
        }
      }
    }

    if message.load(&buffer).is_err() {
      println!("Invalid message") //  TODO log
    }
    
    let state_clone = Arc::clone(&state);
    match message.message_type {
      api_message_type::API_HEARTBEAT => {
        send_heartbeat_message(Arc::clone(&state_clone.socket_core)).await.unwrap();  //  TODO log + kill flag,
      },
      api_message_type::API_GET_SERVICE_INFO => {
        let new_status = serde_json::from_str::<CoreStatus>(message.message_string.as_str()).unwrap();
        println!("{:#?}", new_status);
        state_clone.set_core_status(new_status).await;
      },
      api_message_type::API_GET_CURRENT_CLIENTS => {
        let new_clients = serde_json::from_str::<HashMap<String, Vec<ConnectedClients>>>(message.message_string.as_str()).unwrap();
        println!("{:#?}", new_clients);
        state_clone.set_clients(new_clients["clients"].clone()).await;
      }
      _ => println!("Invalid message type {:#?}", message.message_type) //  TODO log
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
      Ok(ref c) if c == &StatusCode::SERVICE_UNAVAILABLE => break,
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

pub async fn get_status(State(state): State<Arc<AppState>>) -> Result<Response<Body>, ApiError> {
  let response_builder = Response::builder().header(http::header::CONTENT_TYPE, "application/json");
  let response_body = Body::from(state.core_status.lock().await.to_string()?);
  let response = response_builder.body(response_body)?;
  
  println!("{}", state.core_status.lock().await.uptime);

  Ok(response)
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

pub async fn get_connected_clients(State(state): State<Arc<AppState>>) -> Result<Response<Body>, ApiError> {
  let response_builder = Response::builder().header(http::header::CONTENT_TYPE, "application/json");
  let clients = state.clients.lock().await.clone();
  let response_body = Body::from(serde_json::to_string(&clients)?);
  let response = response_builder.body(response_body)?;

  Ok(response)
}

pub async fn update_clients(State(state): State<Arc<AppState>>) -> Result<impl IntoResponse, ApiError> {
  let status = send_client_message(Arc::clone(&state.socket_core)).await;
  match status {
    Ok(ref c) if c == &StatusCode::SERVICE_UNAVAILABLE => Ok(StatusCode::SERVICE_UNAVAILABLE),
    Ok(_) => Ok(StatusCode::OK),
    Err(_) => Ok(StatusCode::INTERNAL_SERVER_ERROR),
  }
}