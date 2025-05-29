use crate::error::ApiError;
use crate::SHARED_CELL;
use axum::extract::Request;
use axum::http::{HeaderMap, StatusCode};
use axum::middleware::Next;
use axum::response::Response;
use jsonwebtoken::{get_current_timestamp, Algorithm, Header, Validation};

#[derive(serde::Serialize, serde::Deserialize, Debug)]
pub struct Claims {
  pub sub: String,
  pub exp: u64,
  pub iat: u64,
}

pub async fn generate_refresh_token(sub: String) -> Result<String, ApiError> {
  let claims = Claims {
    sub: sub,
    iat: get_current_timestamp(),
    exp: get_current_timestamp() + 86400,
  };
  let token = jsonwebtoken::encode(&Header::new(Algorithm::RS256), &claims, &SHARED_CELL.get().unwrap().jwt_keys_refresh.encoding_key)?;
  Ok(token)
}

pub async fn generate_access_token(sub: String) -> Result<String, ApiError> {
  let claims = Claims {
    sub: sub,
    iat: get_current_timestamp(),
    exp: get_current_timestamp() + 600,
  };
  let token = jsonwebtoken::encode(&Header::new(Algorithm::RS256), &claims, &SHARED_CELL.get().unwrap().jwt_keys_access.encoding_key)?;
  Ok(token)
}

pub async fn verify_jwt(header_map: HeaderMap, request: Request, next: Next) -> Result<Response, StatusCode> {
  if let Some(token) = header_map.get("authorization") {
    let mut validation = Validation::new(Algorithm::RS256);
    validation.set_required_spec_claims(&["sub", "iat", "exp"]);

    match jsonwebtoken::decode::<Claims>(token.to_str().unwrap_or_else(|_| {""}), &SHARED_CELL.get().unwrap().jwt_keys_access.decoding_key, &validation) {
      Ok(_) => {
        let response = next.run(request).await;
        Ok(response)
      },
      Err(_) => Err(StatusCode::UNAUTHORIZED)
    }
  } else {
    Err(StatusCode::UNAUTHORIZED)
  }
}
