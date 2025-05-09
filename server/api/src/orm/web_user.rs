use data_encoding::{BASE32, BASE64};
use openssl::rand::rand_bytes;
use openssl::sha::Sha256;
use sea_orm::{EntityTrait, Set};
use entity::entities::web_auth;
use crate::error::ApiError;
use crate::SHARED_CELL;

pub async fn if_user_exists(username: String) -> Result<bool, ApiError> {
  let cell = SHARED_CELL.get().unwrap();
  let db = cell.database_connection.clone().unwrap();
  Ok(web_auth::Entity::find_by_id(username).one(&db).await?.is_some())
}

pub async fn user_update(username: String, password: String) -> Result<(), ApiError> {
  let cell = SHARED_CELL.get().unwrap();
  let db = cell.database_connection.clone().unwrap();

  let mut salt : [u8; 8] = [0; 8];
  rand_bytes(&mut salt)?;
  let encoded_salt = BASE64.encode(&salt);

  let mut hasher = Sha256::new();
  hasher.update(&encoded_salt.as_bytes());
  hasher.update(password.as_bytes());
  let sha_hash = hasher.finish();
  let hashed_password = BASE32.encode(&sha_hash);

  let user = web_auth::ActiveModel{
    username: Set(username),
    hashed_password: Set(hashed_password),
    salt: Set(encoded_salt),
  };

  web_auth::Entity::insert(user).on_conflict(
    sea_orm::sea_query::OnConflict::column(web_auth::Column::Username)
      .update_column(web_auth::Column::HashedPassword)
      .update_column(web_auth::Column::Salt)
      .to_owned()
  ).exec(&db).await?;

  Ok(())
}

pub async fn user_authenticate(username: String, password: String) -> Result<Option<bool>, ApiError> {
  let cell = SHARED_CELL.get().unwrap();
  let db = cell.database_connection.clone().unwrap();

  let user = web_auth::Entity::find_by_id(username).one(&db).await?;

  if let Some(user) = user {
    let mut hasher = Sha256::new();
    hasher.update(&user.salt.as_bytes());
    hasher.update(&password.as_bytes());
    let hashed_password = BASE32.encode(&hasher.finish());

    Ok(Some(user.hashed_password == hashed_password))
  } else {
    Ok(None)
  }
}