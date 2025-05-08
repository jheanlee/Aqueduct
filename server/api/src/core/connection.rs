use std::sync::Arc;
use tokio::io::AsyncWriteExt;
use tokio::net::UnixStream;
use tokio::sync::Mutex;

pub async fn connect_core() -> Result<UnixStream, anyhow::Error> {
  Ok(UnixStream::connect("/tmp/aqueduct-server-core.sock").await?)
}


pub async fn reconnect(socket_core: Arc<Mutex<UnixStream>>) -> Result<(), anyhow::Error> {
  let mut sock = socket_core.lock().await;
  sock.shutdown().await?;
  *sock = connect_core().await?;
  Ok(())
}