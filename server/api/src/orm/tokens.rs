use std::sync::Arc;
use std::time::{SystemTime, UNIX_EPOCH};
use sea_orm::{ActiveModelTrait, EntityTrait, Set};
use entity::entities::{auth};
use entity::entities::prelude::Auth;
use crate::core::core::send_new_token_message;
use crate::domain::tokens::TokenModification;
use crate::error::ApiError;
use crate::SHARED_CELL;

pub async fn list_tokens() -> Result<Vec<auth::Model>, ApiError> {
  let cell = SHARED_CELL.get().unwrap();
  let db = cell.database_connection.clone().unwrap();
  Ok(auth::Entity::find().all(&db).await?)
}

pub async fn if_token_exists(name: String) -> Result<bool, ApiError> {
  let cell = SHARED_CELL.get().unwrap();
  let db = cell.database_connection.clone().unwrap();
  Ok(auth::Entity::find_by_id(name).one(&db).await?.is_some())
}

pub async fn token_update(modification: TokenModification) -> Result<String, ApiError> {
  let cell = SHARED_CELL.get().unwrap();
  let db = cell.database_connection.clone().unwrap();

  let expiry = |days: Option<i64>| {
    if let Some(d) = days {
      Some(SystemTime::now().duration_since(UNIX_EPOCH).ok()?.as_secs() as i64 + (d * 86400))
    } else {
      None
    }
  };

  if let Some(token_item) = auth::Entity::find_by_id(modification.name.clone()).one(&db).await? {
    let mut token: auth::ActiveModel = token_item.into();
    if modification.token_update {
      send_new_token_message(Arc::clone(&SHARED_CELL.get().unwrap().socket_core)).await?;
      SHARED_CELL.get().unwrap().token_channel.notify.notified().await;
      let (new_token, hashed_token) = SHARED_CELL.get().unwrap().token_channel.token_queue.lock().await.pop_front().unwrap_or_else(|| { ("".to_string(), "".to_string()) });

      if new_token.is_empty() {
        Err(ApiError::JsonDumpError)
      } else {
        token.token = Set(hashed_token);
        token.notes = Set(modification.notes);
        token.expiry = Set(expiry(modification.expiry_days));

        token.update(&SHARED_CELL.get().unwrap().database_connection.clone().unwrap()).await?;
        Ok(new_token)
      }
    } else {
      token.notes = Set(modification.notes);
      token.expiry = Set(expiry(modification.expiry_days));
      token.update(&SHARED_CELL.get().unwrap().database_connection.clone().unwrap()).await?;
      Ok("".to_string())
    }
  } else {
    if modification.token_update {
      send_new_token_message(Arc::clone(&SHARED_CELL.get().unwrap().socket_core)).await?;
      SHARED_CELL.get().unwrap().token_channel.notify.notified().await;
      let (new_token, hashed_token) = SHARED_CELL.get().unwrap().token_channel.token_queue.lock().await.pop_front().unwrap_or_else(|| { ("".to_string(), "".to_string()) });
      if new_token.is_empty() {
        Err(ApiError::JsonDumpError)
      } else {

        let token = auth::ActiveModel {
          name: Set(modification.name),
          token: Set(hashed_token),
          notes: Set(modification.notes),
          expiry: Set(expiry(modification.expiry_days)),
        };

        token.insert(&SHARED_CELL.get().unwrap().database_connection.clone().unwrap()).await?;
        Ok(new_token)
      }
    } else {
      Ok("".to_string())
    }
  }
}

pub async fn token_remove(name: String) -> Result<u64, ApiError>{
  let res = Auth::delete_by_id(name).exec(&SHARED_CELL.get().unwrap().database_connection.clone().unwrap()).await?;
  Ok(res.rows_affected)
}