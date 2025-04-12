use sea_orm::EntityTrait;
use entity::entities::client;
use crate::error::ApiError;
use crate::SHARED_CELL;

pub async fn list_client_db() -> Result<Vec<client::Model>, ApiError> {
  let cell = SHARED_CELL.get().unwrap();
  let db = cell.database_connection.clone().unwrap();
  Ok(client::Entity::find().all(&db).await?)
}