use log::{debug, error, info, trace, warn};
use crate::console::color_code::{CYAN, FAINT_GRAY, RED, RESET, YELLOW};
use crate::{SHARED_CELL};

mod color_code {
  pub const RESET: &str = "\x1b[0m";
  pub const RED: &str = "\x1b[31m";
  pub const YELLOW: &str = "\x1b[33m";
  pub const FAINT_GRAY: &str = "\x1b[2;90m";
  pub const CYAN: &str = "\x1b[36m";
}

#[derive(Copy, Clone)]
pub enum Level {
  Critical,
  Error,
  Warning,
  Notice,
  Info,
  Debug
}

impl Level {
  fn as_u8(self) -> u8 {
    match self {
      Level::Critical => 50,
      Level::Error => 40,
      Level::Warning => 30,
      Level::Notice => 30,
      Level::Info => 20,
      Level::Debug => 10
    }
  }
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
  
  #[allow(dead_code)]
  DebugMsg,
}

pub fn console(level: Level, code: Code, detail: &str, function: &str) {
  if SHARED_CELL.get().is_some() && level.as_u8() < SHARED_CELL.get().unwrap().verbose_level {
    return;
  }
  
  let mut output: String = String::new();
  let mut msg: String = String::new();

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
    Level::Notice => {
      output += "[Info] ";
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
      msg += "Failed to set shared resources";
    }
    Code::DatabaseConnectionFailed => {
      msg += "Failed to connect to database";
    }
    Code::KeyInitFailed => {
      msg += "Failed to initialise keys for JWT"
    }
    Code::SockConnectionFailed => {
      msg += "Failed to connect to core";
    }
    Code::SockBindFailed => {
      msg += "Failed to bind socket";
    }
    Code::SockServeFailed => {
      msg += "Failed to serve service";
    }
    Code::SockConnectionLost => {
      msg += "Connection with core has ended";
    }
    Code::SockReadError => {
      msg += "Failed to read from socket";
    }
    Code::SockSendError => {
      msg += "Failed to write to socket";
    }
    Code::SockSelectError => {
      msg += "select error on socket";
    }
    Code::MessageInvalid => {
      msg += "Invalid message";
    }
    Code::ApiDumpFailed => {
      msg += "Failed to resolve api message";
    }
    Code::ApiError => {
      msg += "Error";
    }
    Code::DebugMsg => {
      output += CYAN;
      output += "Debug message";
      output += CYAN;
      output += RESET;
    }
  }

  if !detail.is_empty() {
    msg += ": ";
    msg += detail;
  }
  
  output += msg.as_str();
  
  if SHARED_CELL.get().unwrap().verbose_level <= Level::Debug.as_u8() {
    output += FAINT_GRAY;
    output += format!(" (API::{})", function).as_str();
    output += RESET;
  }
  
  
  if SHARED_CELL.get().is_some() && SHARED_CELL.get().unwrap().daemon_mode {
    if level.as_u8() >= std::cmp::max(SHARED_CELL.get().unwrap().verbose_level, Level::Warning.as_u8()) {
      if cfg!(target_os = "macos") {
        match level {
          Level::Critical => error!("{}" ,msg), //  OS_LOG_TYPE_FAULT
          Level::Error => warn!("{}", msg),     //  OS_LOG_TYPE_ERROR
          Level::Warning => info!("{}", msg),   //  OS_LOG_TYPE_DEFAULT
          Level::Notice => info!("{}", msg),    //  OS_LOG_TYPE_DEFAULT
          Level::Info => debug!("{}", msg),     //  OS_LOG_TYPE_INFO
          Level::Debug => trace!("{}", msg)     //  OS_LOG_TYPE_DEBUG
        }
      } else {
        match level {
          Level::Critical => error!("{}", msg), //  syslog::logger::crit may be better in the future
          Level::Error => error!("{}", msg),
          Level::Warning => warn!("{}", msg),
          Level::Notice => warn!("{}", msg),
          Level::Info => info!("{}", msg),
          Level::Debug => debug!("{}", msg)
        }
      }
    }
  } else {
    println!("{}", output);
  }
}