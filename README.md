# Aqueduct

Aqueduct is a simple TCP Tunneling service that enables users to connect to a service in a private network without exposing other ports or devices.

## Quick Start

You can directly download [precompiled binaries](#precompiled-binaries) for your platform, or [build from source](https://github.com/jheanlee/Aqueduct/wiki/Installation#build-from-source).

**Note: This software is not designed to, and will not, run on Windows.**
**However, you can still tunnel services on the Windows machines via a Unix/Unix-like machine.**

### Installation

#### Precompiled Binaries

Download precompiled binaries from the [release page](https://github.com/jheanlee/Aqueduct/releases/latest)

Currently, there are binaries for the following platforms: 

| Suffix      | Target                            |
|-------------|-----------------------------------|
| linux-amd64 | For Linux on x86-64 based systems |
| linux-arm64 | For Linux on arm based systems    |
| mac-arm64   | For macOS on Apple silicon        |

#### Build from Source

For instructions for building from source, please refer to our [wiki](https://github.com/jheanlee/Aqueduct/wiki/Installation#build-from-source).

### Quick Start

For complete information on every command and feature, please refer to our [wiki](https://github.com/jheanlee/Aqueduct/wiki/Usage-(CLI)).

#### Server

1. Create a webui user using CLI.

```
./aqueduct-server web-user new
```

2. Follow the instructions to create a user.

```
admin@example:~/aqueduct-server$ ./aqueduct-server web-user new
(2025-05-28 12:24:15) Please enter the username of the new user
username
(2025-05-28 12:24:18) Please set a password for this user
password
(2025-05-28 12:24:21) [Info] User created: username
(2025-05-28 12:24:21) [Info] Closing with signal: 0
```

3. Run `aqueduct-server`.

```
./aqueduct-server run
```

```
(2025-05-28 12:30:10) [Info] Streaming host: 0.0.0.0:30330
...

Commands:
Press L for a list of connected clients
Press U for uptime status
Press H for this help message

(2025-05-28 12:30:10) [Info] API service has started
(2025-05-28 12:30:10) [Info] Listening for connection
(2025-05-28 12:30:10) [Info] API connection accepted
(2025-05-28 12:30:10) [Info] Webui running on: 0.0.0.0:30331
```

Your clients can now connect to the server via `0.0.0.0:30330`.

You can now navigate to `http://0.0.0.0:30331` for the web management page.

4. Go to `http://0.0.0.0:30331` and click `Login`. Use the username and password that we created earlier.


5. Go to the `Access` page and click the `Tokens` tab. 


6. Click the `+` button located on the right. Fill in the information and click `Create`.

7. Copy the new token.

#### Client

```
Configuration used as an example:
host (aqueduct-server): 10.0.0.1:30330
service: 10.0.0.2:80
```

1. Run `aqueduct-client` (please replace the example configuration to yours)
```
./aqueduct-client --host-addr 10.0.0.1 --host-port 30330 --service-addr 10.0.0.2 --service-port 80
```

2. Enter your token (you can also use the --token option)
```
(2025-05-28 12:59:11) Please enter your token:
AQ_ReplaceThisWithTheGeneratedToken
```

3. You can now access your service through the tunnelled address
```
(2025-05-28 12:59:19) [Info] Authentication success
(2025-05-28 12:59:19) [Info] Service is now available at: 10.0.0.1:51000
```