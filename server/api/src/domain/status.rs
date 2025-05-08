use std::sync::Arc;
use axum::body::Body;
use axum::extract::State;
use axum::http;
use axum::http::{Response, StatusCode};
use crate::error::ApiError;
use crate::state::AppState;

pub async fn get_status(State(state): State<Arc<AppState>>) -> Result<Response<Body>, ApiError> {
  let response_builder = Response::builder().header(http::header::CONTENT_TYPE, "application/json");
  if state.core_status.lock().await.0 {
    return Ok(response_builder.status(StatusCode::SERVICE_UNAVAILABLE).body(Body::from(""))?);
  }
  let response_body = Body::from(state.core_status.lock().await.1.to_string()?);
  let response = response_builder.body(response_body)?;

  Ok(response)
}