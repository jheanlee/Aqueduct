use jsonwebtoken::{get_current_timestamp, Algorithm, Header, Validation};
use crate::error::ApiError;
use crate::SHARED_CELL;

#[derive(serde::Serialize, serde::Deserialize, Debug)]
struct Claims {
  sub: String,
  exp: u64,
  iat: u64,
}
pub async fn generate_jwt(sub: String) -> Result<String, ApiError> {
  let claims = Claims {
    sub: sub,
    iat: get_current_timestamp(),
    exp: get_current_timestamp() + 600,
  };
  let token = jsonwebtoken::encode(&Header::new(Algorithm::RS256), &claims, &SHARED_CELL.get().unwrap().jwt_keys.encoding_key)?;
  verify_jwt(token.clone()).await?;
  Ok(token)
}

pub async fn verify_jwt(token: String) -> Result<bool, ApiError> {
  let mut validation = Validation::new(Algorithm::RS256);
  validation.set_required_spec_claims(&["sub", "iat", "exp"]);

  match jsonwebtoken::decode::<Claims>(token.as_str(), &SHARED_CELL.get().unwrap().jwt_keys.decoding_key, &validation) {
    Ok(_) => Ok(true),
    Err(ref e) if e.kind() == &jsonwebtoken::errors::ErrorKind::InvalidToken => Ok(false),
    Err(e) => Err(e.into())
  }
}
