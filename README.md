# Sphere Linked

Sphere Linked is a simple TCP Tunneling service that enables user to connect to a service in a private network without exposing other ports or devices.

## Quick Start

You can directly download [precompiled binaries](#precompiled-binaries) for your platform, or [build from source](#build-from-source).

**Note: This software is not designed to, and will not, work on Windows (even you build it from source)**

### Precompiled Binaries

Download precompiled binaries from the [release page](https://github.com/jheanlee/Sphere-Linked/releases/latest)

Currently, there are binaries for two platforms: 

| File         | Description                       |
|--------------|-----------------------------------|
| *linux-amd64 | For Linux on x86-64 based systems |
| *mac-apple   | For macOS on Apple sillicon       |

Alternatively, you can directly build from source.

### Build from Source

To build from source, you will need the following installed:
1. A C++ compiler supporting at least C++17 (gcc recommanded)
2. [CMake](https://cmake.org)
3. [OpenSSL](https://openssl.org) (some distros also need the dev packages: libssl-dev (apt), openssl-devel (dnf), etc.)
4. [SQLite3](https://www.sqlite.org) (some distros also need the dev packages: libsqlite3-dev (apt), sqlite-devel (dnf), etc.)
5. uuid-lib (pre-installed on macOS, uuid-dev (apt), uuid-devel (dnf), etc.)
6. [CLI11](https://github.com/CLIUtils/CLI11) (cli11 (brew), libcli11-dev (apt), cli11-devel(dnf))

#### 0. Install the requirements
#### Debian/Ubuntu (apt)
```
sudo apt update && \
sudo apt install -y gcc g++ cmake openssl libssl-dev sqlite3 libsqlite3-dev uuid-dev libcli11-dev
```
#### Fedora (dnf)
```
sudo dnf install -y gcc g++ cmake openssl openssl-devel sqlite3 sqlite-devel uuid-devel cli11-devel
```

For other distros, please refer to their documentation.

#### 1. Clone the source from our repo
```
git clone https://github.com/jheanlee/Sphere-Linked.git
```

#### 2. Switch to the directory and create a build directory
```
cd Sphere-Linked && mkdir build
```

#### 3. Run CMake
```
cmake . ..
```

#### 4. Run make
```
make
```

Now you should see the binaries `sphere-linked-server` and `sphere-linked-client` in the directory.

### Tokens

To (re)generate a token, you can use the command below:
```
./sphere-linked-server token new --name [NAME]
```
Optionally, you can add some notes using the `--notes` option.

To get a list of all tokens, use:
```
./sphere-linked-server token list
```
Please notice that the tokens listed are hashed and are not usable for security.

You can also remove a token using:
```
./sphere-linked-server token remove --name
```

### Key

Before you can start tunneling your service, you need a pair of key and certificate on the server for secure (TLS) connection.

If you already have those, you can [skip](#connection) this step. Here, we are going to use OpenSSL to generate them:
```
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 365 -nodes
```
Fill out the prompted instructions.

The connection between the server and the clients will be encrypted using this pair.

### Connection
#### Server

To start the server service up, use:
```
sphere-linked-server run --tls-key [KEY] --tls-cert [CERT]
```
Replace `[KEY]` and `[CERT]` with the path to the key and certificate that we generated earlier.

#### Client
After the server is running, we can now tunnel our services.

Go to the client machine and run:
```
sphere-linked-client -H [SERVER ADDR] -s [SERVICE ADDR] -p [SERVICE PORT]
```
Replace the blanks with the corresponding values. `[SERVER ADDR]` can be either an ipv4 address (like `192.168.1.1`) or a domain (like `www.example.com`)

Now you should be prompted a message like this: 
```
(2025-02-10 10:24:01) Please enter your token:
```
Enter the [token generated](#tokens) on the server earlier, and your service will be tunneled to the server.

For more details or options, see the [Usage](#usage) section below

## Usage

### Host (server)

```
sphere-linked-server [OPTIONS]  SUBCOMMAND
```

#### Options

```
Options:
  -h,--help                        Print this help message and exit
  -v,--verbose                     Output detailed information
  -d,--database TEXT [./sphere-linked.sqlite] 
                                   The path to database file

Subcommands:
  run                              Run the main tunneling service
  token                            Operations related to tokens
```

##### run options
```
Options:
  -h,--help                        Print this help message and exit
  -k,--tls-key TEXT REQUIRED       The path to a private key file used for TLS encryption
  -c,--tls-cert TEXT REQUIRED      The path to a certification file used for TLS encryption
  -p,--control INT [30330]         Client will connect via 0.0.0.0:<port>
  -s,--port-start INT [51000]      The proxy port of the first client will be <port>, the last being (<port> + port-limit - 1)
  -l,--port-limit INT [200]        Proxy ports will have a limit of <count> ports
  --session-timeout INT [10]       The time(ms) poll() waits each call when accepting connections. See `man poll` for more information
  --proxy-timeout INT [1]         The time(ms) poll() waits each call during proxying. See `man poll` for more information
```


##### token options
```
Options:
  -h,--help                        Print this help message and exit

Subcommands:
  new                              Create or regenerate a token
    Options:
      -n,--name TEXT REQUIRED          The name (id) of the token you want to modify
      --notes TEXT                     Some notes for this token
  remove                           Remove a token
    Options:
      -n,--name TEXT REQUIRED          The name (id) of the token you want to modify
  list                             List all tokens
```

#### Notes

The host's control port and proxy port range must be accessable to client and external users

### Client

```
sphere-linked-client [OPTIONS] SUBCOMMAND
```

#### Options

```
Options:
  -h,--help                        Print this help message and exit
  -v,--verbose                     Output detailed information
  -t,--token TEXT                  Token for accessing server. Only use this option on trusted machine
  -H,--host-addr TEXT [0.0.0.0]    The host to stream to. Accepts ipv4 or domain
  -P,--host-port INT [30330]       The control port of host
  -s,--service-addr TEXT [0.0.0.0] The address of the service to be tunneled
  -p,--service-port INT REQUIRED   The port of the service to be tunneled
  --session-timeout INT [10]       The time(ms) poll() waits each call when accepting connections. See `man poll` for more information
  --proxy-timeout INT [1]         The time(ms) poll() waits each call during proxying. See `man poll` for more information
```

### Use Examples

```
sphere-linked-server --control 63000 --port-start 61000 --port-limit 300 --tls-key /tls/privkey.pem --tls-cert /tls/cert
```
This example opens the control port at `host:63000`, at most tunnels `300` services. (the first being `host:61000`, and the last being `host:61299`),   
using `/tls/privkey.pem` and `/tls/cert` to establish TLS connections between server and client.

```
sphere-linked-client --host-addr test.example.com --host-port 63000 --service-addr 192.168.1.100 --service-port 8080
```
This example connects to server via `test.example.com:63000`, and tunnels `192.168.1.100:8080` to `test.example.com`.