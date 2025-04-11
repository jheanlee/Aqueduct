use sea_orm::{Database, DatabaseConnection, DbErr, DeriveIden};
use crate::error::ApiError;
use crate::SHARED_CELL;

pub struct DbClient {
  pub ip: String,
  pub sent: u64,
  pub received: u64,
}

pub async fn connect_database(path: String) -> Result<DatabaseConnection, DbErr> {
  Ok(Database::connect(format!("sqlite://{}", path)).await?)
}

pub async fn list_client_db() -> Result<(), ApiError> {
  
  Ok(())
}