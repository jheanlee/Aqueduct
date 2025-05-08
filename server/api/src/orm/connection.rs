use sea_orm::{Database, DatabaseConnection, DbErr};

pub async fn connect_database(path: String) -> Result<DatabaseConnection, DbErr> {
  Ok(Database::connect(format!("sqlite://{}", path)).await?)
}