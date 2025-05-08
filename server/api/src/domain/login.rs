use axum::body::Body;
use axum::extract::Query;
use axum::{http, Json};
use axum::response::{IntoResponse, Response};
use crate::auth::jwt_tokens::generate_jwt;
use crate::error::ApiError;
use crate::orm::web_user::{if_user_exists, user_authenticate, user_update};

#[derive(serde::Serialize, serde::Deserialize)]
pub struct CheckUserParams {
  username: String
}
pub async fn check_web_user(Query(params): Query<CheckUserParams>) -> Result<Response<Body>, ApiError> {
  let response_builder = Response::builder().header(http::header::CONTENT_TYPE, "application/json");
  let response_body = Body::from(serde_json::json!({
    "available": !if_user_exists(params.username).await?
  }).to_string());
  let response = response_builder.body(response_body)?;
  Ok(response)
}

#[derive(serde::Serialize, serde::Deserialize)]
pub struct UserModification {
  username: String,
  password: String
}
pub async fn modify_web_user(Json(modification): Json<UserModification>) -> Result<impl IntoResponse, ApiError> {
  user_update(modification.username, modification.password).await?;
  Ok(())
}

#[derive(serde::Serialize, serde::Deserialize)]
pub struct UserAuthentication {
  username: String,
  password: String
}
pub async fn login(Json(authentication): Json<UserAuthentication>) -> Result<Response<Body>, ApiError> {
  let res = user_authenticate(authentication.username.clone(), authentication.password).await?;
  let status = match res {
    Some(auth) => if auth { "success" } else { "failed" },
    None => "not found"
  };

  let response_builder = Response::builder().header(http::header::CONTENT_TYPE, "application/json");
  let response_body = Body::from(serde_json::json!({
    "status": status,
    "token": generate_jwt(authentication.username).await?
  }).to_string());
  let response = response_builder.body(response_body)?;
  Ok(response)
}