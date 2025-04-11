use axum::http::StatusCode;
use axum::response::Response;
use crate::console::{console, Code, Level};

#[derive(Debug)]
pub enum ApiError {
  Error(anyhow::Error),
  MessageError(MessageError),
}

#[derive(Debug)]
pub enum MessageError {
  InvalidType,
  InvalidStringLength,
  InvalidString,
  InvalidBufferLength,
}

impl axum::response::IntoResponse for ApiError {
  fn into_response(self) -> Response {
    match self {
      ApiError::Error(e) => {
        console(Level::Warning, Code::ApiError, e.to_string().as_str(), "error::anyhow");
        StatusCode::INTERNAL_SERVER_ERROR.into_response()
      },
      ApiError::MessageError(..) => StatusCode::INTERNAL_SERVER_ERROR.into_response(),
    }
  }
}

impl<E> From<E> for ApiError where E: Into<anyhow::Error> {
  fn from(error: E) -> Self {
    Self::Error(error.into())
  }
}