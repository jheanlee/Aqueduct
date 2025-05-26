use axum::http::StatusCode;
use axum::{http, Json};
use axum::body::Body;
use axum::response::{IntoResponse, Response};
use jsonwebtoken::{get_current_timestamp, Algorithm, Validation};
use crate::auth::jwt_tokens::{generate_access_token, generate_refresh_token};
use crate::error::ApiError;
use crate::orm::web_user::user_authenticate;
use crate::SHARED_CELL;

#[derive(serde::Serialize, serde::Deserialize)]
pub struct UserAuthentication {
  username: String,
  password: String
}
pub async fn login(Json(authentication): Json<UserAuthentication>) -> Result<Response, ApiError> {
  let res = user_authenticate(authentication.username.clone(), authentication.password).await?;
  if res.is_none() || !res.unwrap() {
    Ok(StatusCode::UNAUTHORIZED.into_response())
  } else {
    let response_builder = Response::builder().header(http::header::CONTENT_TYPE, "application/json");
    let response_body = Body::from(serde_json::json!({
      "refresh_token": generate_refresh_token(authentication.username.clone()).await?,
      "access_token": generate_access_token(authentication.username).await?
    }).to_string());
    let response = response_builder.body(response_body)?;
    Ok(response)
  }
}

#[derive(serde::Serialize, serde::Deserialize)]
pub struct Tokens {
  refresh_token: String,
  access_token: String
}
pub async fn refresh_token(Json(tokens): Json<Tokens>) -> Result<Response, ApiError> {
  let mut validation_refresh = Validation::new(Algorithm::RS256);
  validation_refresh.set_required_spec_claims(&["sub", "iat", "exp"]);
  
  let refresh: Result<jsonwebtoken::TokenData<crate::auth::jwt_tokens::Claims>, jsonwebtoken::errors::Error> = 
    jsonwebtoken::decode(tokens.refresh_token.as_str(), &SHARED_CELL.get().unwrap().jwt_keys_refresh.decoding_key, &validation_refresh);
  
  if let Ok(refresh_token_data) = refresh {
    let mut validation_access = Validation::new(Algorithm::RS256);
    validation_access.set_required_spec_claims(&["sub", "iat"]);
    validation_access.validate_exp = false;
    
    let access: Result<jsonwebtoken::TokenData<crate::auth::jwt_tokens::Claims>, jsonwebtoken::errors::Error> =
      jsonwebtoken::decode(tokens.access_token.as_str(), &SHARED_CELL.get().unwrap().jwt_keys_access.decoding_key, &validation_access);
    
    if let Ok(access_token_data) = access {
      if access_token_data.claims.exp >= get_current_timestamp() {
        let new_access_token = generate_access_token(refresh_token_data.claims.sub).await;

        let response_builder = Response::builder().header(http::header::CONTENT_TYPE, "application/json");
        let response_body = Body::from(serde_json::json!({
          "access_token" :new_access_token?
        }).to_string());
        let response = response_builder.body(response_body)?;
        Ok(response)
      } else {
        Ok(StatusCode::UNAUTHORIZED.into_response())
      }
    } else {
      Ok(StatusCode::UNAUTHORIZED.into_response())
    }
  } else {
    Ok(StatusCode::UNAUTHORIZED.into_response())
  }
}