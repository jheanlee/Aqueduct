# Aqueduct

Aqueduct is a simple TCP Tunneling service that enables users to connect to a service in a private network without exposing other ports or devices.

## Quick Start

You can directly download [precompiled binaries](#precompiled-binaries) for your platform, or [build from source](#build-from-source).

**Note: This software is not designed to, and will not, run on Windows. However, you can still tunnel services on the Windows machines via a Unix/Unix-like machine.**

### Precompiled Binaries

Download precompiled binaries from the [release page](https://github.com/jheanlee/Aqueduct/releases/latest)

Currently, there are binaries for two platforms: 

| File         | Description                       |
|--------------|-----------------------------------|
| *linux-amd64 | For Linux on x86-64 based systems |
| *mac-apple   | For macOS on Apple sillicon       |

Alternatively, you can directly build from source.

### Build from Source

To build from source, you will need the following installed:
1. A C++ compiler supporting at least C++17 (gcc recommended for linux, Apple clang recommended for macOS)
2. [CMake](https://cmake.org)
3. [OpenSSL](https://openssl.org) (some distros also need the dev packages: libssl-dev (apt), openssl-devel (dnf), etc.)
4. [SQLite3](https://www.sqlite.org) (some distros also need the dev packages: libsqlite3-dev (apt), sqlite-devel (dnf), etc.)
5. uuid-lib (pre-installed on macOS, uuid-dev (apt), libuuid-devel (dnf), etc.)
6. [CLI11](https://github.com/CLIUtils/CLI11) (cli11 (brew), libcli11-dev (apt), cli11-devel(dnf))

#### 0. Install the requirements
#### Debian/Ubuntu (apt)
```
sudo apt update && \
sudo apt install -y git gcc g++ cmake openssl libssl-dev sqlite3 libsqlite3-dev uuid-dev libcli11-dev
```
#### Fedora (dnf)
```
sudo dnf install -y git gcc g++ cmake openssl openssl-devel sqlite3 sqlite-devel libuuid-devel cli11-devel
```

For other distros, or if you have issues installing the packages, please refer to their documentation.

#### 1. Clone the source from our repo
```
git clone https://github.com/jheanlee/Aqueduct.git
```

#### 2. Switch to the directory and create a build directory
```
cd Aqueduct && mkdir build
```

#### 3. Run CMake
```
cmake . ..
```

#### 4. Run make
```
make
```

Now you should see the binaries `aqueduct-server` and `aqueduct-client` in the directory.

### Tokens

To (re)generate a token, you can use the command below:
```
./aqueduct-server token new --name [NAME]
```
Optionally, you can add some notes using the `--notes` option.

To get a list of all tokens, use:
```
./aqueduct-server token list
```
Please notice that the tokens listed are hashed and are not usable for security.

You can also remove a token using:
```
./aqueduct-server token remove --name
```

### Key

Before you can start tunneling your service, you need a pair of map_key and certificate on the server for secure (TLS) connection.

If you already have those, you can [skip](#connection) this step. Here, we are going to use OpenSSL to generate them:
```
openssl req -x509 -newkey rsa:4096 -keyout map_key.pem -out cert.pem -sha256 -days 365 -nodes
```
Fill out the prompted instructions.

The connection between the server and the clients will be encrypted using this pair.

### Connection
#### Server

To start the server service up, use:
```
aqueduct-server run --tls-key [KEY] --tls-cert [CERT]
```
Replace `[KEY]` and `[CERT]` with the path to the map_key and certificate that we generated earlier.

#### Client
After the server is running, we can now tunnel our services.

Go to the client machine and run:
```
aqueduct-client -H [SERVER ADDR] -s [SERVICE ADDR] -p [SERVICE PORT]
```
Replace the blanks with the corresponding values. `[SERVER ADDR]` can be either an ipv4 address (like `192.168.1.1`) or a domain (like `www.example.com`)

Now you should be prompted a Message like this: 
```
(2025-02-10 10:24:01) Please enter your token:
```
Enter the [token generated](#tokens) on the server earlier, and your service will be tunneled to the server.

For more details or options, see the [Usage](#usage) section below

## Usage

### Host (server)

```
aqueduct-server [OPTIONS]  SUBCOMMAND
```

#### Options

```
Options:
  -h,--help                        Print this help message and exit
  -v,--verbose INT [20]            Output information detail level (inclusive). 10 for Debug or above, 50 for Critical only. Daemon logs have mask of max(30, verbose_level)
  -d,--database TEXT [./aqueduct.sqlite] 
                                   The path to database file

Subcommands:
  run                              Run the tunneling service
  token                            Token management
```

##### run options
```
Options:
  -h,--help                        Print this help message and exit
  -D,--daemon-mode                 Disables stdout and use syslog or os_log instead
  -k,--tls-key TEXT REQUIRED   The path to a private key file used for TLS encryption
  -c,--tls-cert TEXT REQUIRED      The path to a certification file used for TLS encryption
  -p,--control INT [30330]         Client will connect via 0.0.0.0:<port>
  -s,--port-start INT [51000]      The proxy port of the first client will be <port>, the last being (<port> + port-limit - 1)
  -l,--port-limit INT [200]        Proxy ports will have a limit of <count> ports
  --proxy-timeout INT [1]          The time(ms) poll() waits each call during proxying. See `man poll` for more information
  --client-db-interval INT [1]     The interval(min) between automatic writes of client's proxied data to database
```


##### token options
```
Options:
  -h,--help                        Print this help Message and exit

Subcommands:
  new                              Create or regenerate a token
    Options:
      -h,--help                        Print this help message and exit
      -n,--name TEXT REQUIRED          The name (id) of the token you want to modify
      --notes TEXT                     Some notes for this token
      --expiry INT:INT in [0 - 3650] [100] 
                                       Days until the expiry of the token. 0 for no expiry
                                       
  remove                           Remove a token
    Options:
      -n,--name TEXT REQUIRED          The name (id) of the token you want to modify
      
  list                             List all tokens
```

#### Notes

The host's control port and proxy port range must be accessable to client and external users

### Client

```
aqueduct-client [OPTIONS]
```

#### Options

```
Options:
  -h,--help                        Print this help message and exit
  -v,--verbose INT [20]            Output information detail level (inclusive). 10 for Debug or above, 50 for Critical only. Daemon logs have mask of max(30, verbose_level)
  -t,--token TEXT                  Token for accessing server. Only use this option on trusted machine
  -H,--host-addr TEXT [0.0.0.0]    The host to stream to. Accepts ipv4 or domain
  -P,--host-port INT [30330]       The control port of host
  -s,--service-addr TEXT [0.0.0.0] 
                                   The address of the service to be tunneled
  -p,--service-port INT REQUIRED   The port of the service to be tunneled
  --proxy-timeout INT [1]          The time(ms) poll() waits each call during proxying. See `man poll` for more information
```