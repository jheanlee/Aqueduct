pub enum Level {
  CRITICAL,
  ERROR,
  WARNING,
  INFO,
  DEBUG,
}


pub async fn console(level: Level) {
  match level {
    Level::CRITICAL => {}
    Level::ERROR => {}
    Level::WARNING => {}
    Level::INFO => {}
    Level::DEBUG => {}
  }
}