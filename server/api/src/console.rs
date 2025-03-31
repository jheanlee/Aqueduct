use crate::console::color_code::{RED, RESET, YELLOW};

pub enum Level {
  CRITICAL = 50,
  ERROR = 40,
  WARNING = 30,
  INFO = 20,
  DEBUG = 10,
}

mod color_code {
  pub const RESET: &str = "\033[0m";
  pub const RED: &str = "\033[31m";
  pub const YELLOW: &str = "\033[33m";
  pub const FAINT_GRAY: &str = "\033[2;90m";
  pub const CYAN: &str = "\033[36m";
}


pub async fn console(level: Level) {
  let mut output: String = String::new();
  
  match level {
    Level::CRITICAL => {
      output += RED;
      output += "[Critical] ";
    }
    Level::ERROR => {
      output += RED;
      output += "[Error] ";
    }
    Level::WARNING => {
      output += YELLOW;
      output += "[Warning] ";
    }
    Level::INFO => {
      output += "[Info] ";
    }
    Level::DEBUG => {
      output += "[Debug] ";
    }
  }
  output += RESET;
  
  //  TODO
  
  
  println!("{}", output);
}