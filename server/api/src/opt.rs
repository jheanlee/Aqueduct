#[derive(clap::Parser)]
#[derive(Debug)]
pub struct Args {
  #[arg(short = 'D', long, default_value_t = false)]
  pub daemon_mode: bool,
  #[arg(short, long, default_value_t = 20)]
  pub verbose: u8,
  #[arg(short = 'd', long, required = true)]
  pub database: String,
  #[arg(long, required = true)]
  pub refresh_private_key: String,
  #[arg(long, required = true)]
  pub refresh_public_key: String,
  #[arg(long, required = true)]
  pub access_private_key: String,
  #[arg(long, required = true)]
  pub access_public_key: String,
}