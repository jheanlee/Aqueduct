use crate::error::ApiError;
use crate::orm::web_user::{if_user_exists, list_web_users, user_remove, user_update};
use axum::body::Body;
use axum::extract::Query;
use axum::http::StatusCode;
use axum::response::{IntoResponse, Response};
use axum::{http, Json};

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

pub async fn get_web_users() -> Result<Response<Body>, ApiError> {
  let response_builder = axum::http::Response::builder().header(http::header::CONTENT_TYPE, "application/json");
  let tokens: Vec<entity::entities::web_auth::PartialModel> = list_web_users().await?;
  let response_body = Body::from(serde_json::to_string(&tokens)?);
  let response = response_builder.body(response_body)?;

  Ok(response)
}

#[derive(serde::Serialize, serde::Deserialize)]
pub struct UserModification {
  username: String,
  password: String
}
pub async fn modify_web_user(Json(modification): Json<UserModification>) -> Result<impl IntoResponse, ApiError> {
  let username_regex = regex::Regex::new(r"^[a-zA-Z][a-zA-Z0-9]{0,31}$")?;
  let password_regex = regex::Regex::new(r"^[!-~]{1,32}$")?;
  
  if username_regex.is_match(modification.username.as_str()) && password_regex.is_match(modification.password.as_str()) {
    user_update(modification.username, modification.password).await?;
    Ok(StatusCode::OK)
  } else {
    Ok(StatusCode::BAD_REQUEST)
  }
}

#[derive(serde::Serialize, serde::Deserialize)]
pub struct UserDeletion {
  username: String
}
#[derive(serde::Serialize, serde::Deserialize)]
pub struct UserDeletionBody {
  rows_affected: u64
}
pub async fn delete_web_user(Json(deletion): Json<UserDeletion>) -> Result<impl IntoResponse, ApiError> {
  let rows_affected = user_remove(deletion.username).await?;
  Ok(Json(UserDeletionBody{ rows_affected }))
}