use jsonwebtoken::{DecodingKey, EncodingKey};

pub struct JwtKeys {
  pub encoding_key: EncodingKey,
  pub decoding_key: DecodingKey
}

pub async fn init_jwt_keys(private_key_path: String, public_key_path: String) -> Result<JwtKeys, anyhow::Error> {
  let priv_bytes = tokio::fs::read(private_key_path).await?;

  let encoding_key = EncodingKey::from_rsa_pem(priv_bytes.as_slice())
    .or_else(|_| EncodingKey::from_ec_pem(priv_bytes.as_slice()))
    .or_else(|_| EncodingKey::from_ed_pem(priv_bytes.as_slice()))?;

  let pub_bytes = tokio::fs::read(public_key_path).await?;

  let decoding_key = DecodingKey::from_rsa_pem(pub_bytes.as_slice())
    .or_else(|_| DecodingKey::from_ec_pem(pub_bytes.as_slice()))
    .or_else(|_| DecodingKey::from_ed_pem(pub_bytes.as_slice()))?;

  Ok(JwtKeys { encoding_key, decoding_key })
}