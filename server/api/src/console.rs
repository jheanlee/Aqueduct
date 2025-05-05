use crate::console::color_code::{CYAN, FAINT_GRAY, RED, RESET, YELLOW};
use crate::{SHARED_CELL};

mod color_code {
  pub const RESET: &str = "\x1b[0m";
  pub const RED: &str = "\x1b[31m";
  pub const YELLOW: &str = "\x1b[33m";
  pub const FAINT_GRAY: &str = "\x1b[2;90m";
  pub const CYAN: &str = "\x1b[36m";
}

#[repr(u8)]
#[derive(Copy, Clone)]
pub enum Level {
  Critical = 50,
  Error = 40,
  Warning = 30,
  Info = 20,
  Debug = 10,
}

pub enum Code {
  SharedResourcesSetFailed,
  
  DatabaseConnectionFailed,
  
  KeyInitFailed,
  
  SockConnectionFailed,
  SockBindFailed,
  SockServeFailed,
  SockConnectionLost,
  SockReadError,
  SockSendError,
  SockSelectError,
  
  MessageInvalid,
  ApiDumpFailed,
  ApiError,
  DebugMsg,
}

pub fn console(level: Level, code: Code, detail: &str, function: &str) {
  if SHARED_CELL.get().is_some() && (level.clone() as u8) <  SHARED_CELL.get().unwrap().verbose_level {
    return;
  }
  let mut output: String = String::new();

  output += chrono::Utc::now().format("(%F %T) ").to_string().as_str();
  
  match level {
    Level::Critical => {
      output += RED;
      output += "[Critical] ";
    }
    Level::Error => {
      output += RED;
      output += "[Error] ";
    }
    Level::Warning => {
      output += YELLOW;
      output += "[Warning] ";
    }
    Level::Info => {
      output += "[Info] ";
    }
    Level::Debug => {
      output += "[Debug] ";
    }
  }
  output += RESET;
  
  match code {
    Code::SharedResourcesSetFailed => {
      output += "Failed to set shared resources";
    }
    Code::DatabaseConnectionFailed => {
      output += "Failed to connect to database";
    }
    Code::KeyInitFailed => {
      output += "Failed to initialise keys for JWT"
    }
    Code::SockConnectionFailed => {
      output += "Failed to connect to core";
    }
    Code::SockBindFailed => {
      output += "Failed to bind socket";
    }
    Code::SockServeFailed => {
      output += "Failed to serve service";
    }
    Code::SockConnectionLost => {
      output += "Connection with core has ended";
    }
    Code::SockReadError => {
      output += "Failed to read from socket";
    }
    Code::SockSendError => {
      output += "Failed to write to socket";
    }
    Code::SockSelectError => {
      output += "select error on socket";
    }
    Code::MessageInvalid => {
      output += "Invalid message";
    }
    Code::ApiDumpFailed => {
      output += "Failed to resolve api message";
    }
    Code::ApiError => {
      output += "Error";
    }
    Code::DebugMsg => {
      output += CYAN;
      output += "Debug message";
      output += CYAN;
      output += RESET;
    }
  }

  if detail.is_empty() {
    output += ": ";
    output += detail;
  }
  
  if SHARED_CELL.get().unwrap().verbose_level <= Level::Debug as u8 {
    output += FAINT_GRAY;
    output += format!(" (API::{})", function).as_str();
    output += RESET;
  }
  
  println!("{}", output);
  //TODO daemon mode logging
}