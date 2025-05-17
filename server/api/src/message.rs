use crate::error::ApiError;
use crate::error::MessageError;

const MESSAGE_MAX_STRING_SIZE: usize = 126;

pub mod api_message_type {
  pub const API_HEARTBEAT: u8 = 'B' as u8;
  pub const API_EXIT: u8 = 'C' as u8;
  pub const API_GET_SERVICE_INFO: u8 = 'D' as u8;
  pub const API_GET_CURRENT_CLIENTS: u8 = 'E' as u8;
  pub const API_GENERATE_NEW_TOKEN: u8 = 'F' as u8;
}

pub struct Message {
  pub message_type: u8,
  pub message_string: String,
}

impl Message {
  pub fn dump(&mut self) -> Result<[u8; 128], ApiError> {
    if self.message_type == ('\0' as u8) {
      return Err(ApiError::MessageError(MessageError::InvalidType));
    }
    if self.message_string.len() > MESSAGE_MAX_STRING_SIZE {
      return Err(ApiError::MessageError(MessageError::InvalidStringLength));
    }
    if !self.message_string.is_ascii() {
      return Err(ApiError::MessageError(MessageError::InvalidString));
    }

    let mut buffer: [u8; 128] = [0; 128];

    buffer[0] = self.message_type.into();
    buffer[1..self.message_string.len() + 1].copy_from_slice(self.message_string.as_bytes());
    Ok(buffer)
  }

  pub fn load(&mut self, buffer: &[u8]) -> Result<(), ApiError> {
    if buffer.len() == 0 {
      return Err(ApiError::MessageError(MessageError::InvalidBufferLength));
    }

    let str = core::str::from_utf8(buffer);
    if let Err(_) = str {
      return Err(ApiError::MessageError(MessageError::InvalidString));
    }

    self.message_type = str?.chars().nth(0).unwrap_or('\0') as u8;
    self.message_string = str?.chars().skip(1).filter(|c| c != &'\0').collect();

    Ok(())
  }
}