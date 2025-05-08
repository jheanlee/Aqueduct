use std::sync::Arc;
use axum::body::Body;
use axum::extract::{Query, State};
use axum::{http, Json};
use axum::http::Response;
use crate::error::ApiError;
use crate::orm::tokens::{if_token_exists, list_tokens, token_remove, token_update};
use crate::state::AppState;

#[derive(serde::Serialize, serde::Deserialize, Clone)]
pub struct TokenModification {
  pub name: String,
  pub token_update: bool,
  pub notes: Option<String>,
  pub expiry_days: Option<i64>,
}

#[derive(serde::Serialize, serde::Deserialize)]
pub struct TokenDeletion {
  name: String,
}

#[derive(serde::Serialize, serde::Deserialize)]
pub struct TokenModificationBody {
  token: String,
}

#[derive(serde::Serialize, serde::Deserialize)]
pub struct TokenDeletionBody {
  rows_affected: u64
}

pub async fn get_tokens(State(_state): State<Arc<AppState>>) -> Result<Response<Body>, ApiError> {
  let response_builder = Response::builder().header(http::header::CONTENT_TYPE, "application/json");
  let tokens: Vec<entity::entities::auth::Model> = list_tokens().await?;
  let response_body = Body::from(serde_json::to_string(&tokens)?);
  let response = response_builder.body(response_body)?;

  Ok(response)
}

#[derive(serde::Serialize, serde::Deserialize)]
pub struct CheckTokenParams {
  name: String
}

pub async fn check_token(Query(params): Query<CheckTokenParams>) -> Result<Response<Body>, ApiError> {
  let response_builder = Response::builder().header(http::header::CONTENT_TYPE, "application/json");
  let response_body = Body::from(serde_json::json!({
    "available" : !if_token_exists(params.name).await?
  }).to_string());
  let response = response_builder.body(response_body)?;
  Ok(response)
}

pub async fn modify_token_item(Json(modification): Json<TokenModification>) -> Result<Json<TokenModificationBody>, ApiError> {
  let res = token_update(modification.clone()).await?;

  if modification.token_update {
    Ok(TokenModificationBody {
      token: res
    }.into())
  } else {
    Ok(TokenModificationBody {
      token: "".to_string()
    }.into())
  }
}

pub async fn delete_token_item(Json(deletion): Json<TokenDeletion>) -> Result<Json<TokenDeletionBody>, ApiError> {
  let rows_affected = token_remove(deletion.name).await?;
  Ok(Json(TokenDeletionBody{ rows_affected }))
}