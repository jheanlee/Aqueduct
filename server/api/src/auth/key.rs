use jsonwebtoken::{DecodingKey, EncodingKey};
use crate::error::ApiError;

pub struct JwtKeys {
  pub encoding_key: EncodingKey,
  pub decoding_key: DecodingKey
}

pub async fn init_jwt_keys(key_path: String) -> Result<JwtKeys, ApiError> {
  let bytes = tokio::fs::read(key_path).await?;

  let encoding_key = EncodingKey::from_rsa_pem(bytes.as_slice())
    .or_else(|_| EncodingKey::from_ec_pem(bytes.as_slice()))
    .or_else(|_| EncodingKey::from_ed_pem(bytes.as_slice()))?;

  let decoding_key = DecodingKey::from_rsa_pem(bytes.as_slice())
    .or_else(|_| DecodingKey::from_ec_pem(bytes.as_slice()))
    .or_else(|_| DecodingKey::from_ed_pem(bytes.as_slice()))?;

  Ok(JwtKeys { encoding_key, decoding_key })
}

#[derive(serde::Serialize, serde::Deserialize)]
struct Claims {
  sub: String,
  exp: usize,
  iat: usize,
  iss: String,
}

