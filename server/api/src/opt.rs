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
  pub private_key: String,
  #[arg(long, required = true)]
  pub public_key: String,
}