# Sphere Linked

Sphere Linked is a simple TCP Tunneling service that enables user to connect to a service in a private network without exposing other ports or devices.

## Usage

### Host (server)

```
sphere-linked-server [OPTIONS]
```

#### Options

```
    -h, --help                          Prints this page
    -p, --control-port <port>           Client will connect to localhost:<port>
                                        Should be identical with --host-port of client
                                        Default is 30330
    -s, --port-start <port>             The proxy port of the first client will be <port>, the last being (<port> + port-limit - 1)
                                        Default is 51000
    -l, --port-limit <count>            Proxy ports will have a limit of <count> ports
                                        Default is 200
    -k, --tls-key <path>                The path to a private key file used for TLS/SSL encryption
                                        This option is REQUIRED
    -c, --tls-cert <path>               The path to a certification file used for TLS/SSL encryption
                                        This certification must match the key
                                        This option is REQUIRED
    --session-select-timeout <time>     The time select() waits each call when accepting connections, see `man select` for more information
                                        timeval.sec would be (<time> / 1000), and timeval.usec would be (<time> % 1000)
                                        Default is 10
    --proxy-select-timeout <time>       The time select() waits each call during proxying, see `man select` for more information
                                        timeval.sec would be (<time> / 1000), and timeval.usec would be (<time> % 1000)
                                        Default is 1
```

#### Notes

The host's control port and proxy port range should be accessable to client and external users

### Client

```
sphere-linked-client [OPTIONS]
```

#### Options

```
    -h, --help                          Prints this page
    -H, --host-addr <ipv4|domain>       Sets host to <ipv4|domain>
                                        Default is 0.0.0.0
    -P, --host-port <port>              Uses host:<port> as control port (see --control-port of server)
                                        Default is 3000
    -s, --service-addr <ipv4>           Sets the address of service to be tunneled to <ipv4>
                                        Default is 0.0.0.0
    -p, --service-port <port>           Tunnels service:<port> to host
                                        This option is REQUIRED
    --session-select-timeout <time>     The time select() waits each call when accepting connections, see `man select` for more information
                                        timeval.sec would be (<time> / 1000), and timeval.usec would be (<time> % 1000)
                                        Default is 10
    --proxy-select-timeout <time>       The time select() waits each call during proxying, see `man select` for more information
                                        timeval.sec would be (<time> / 1000), and timeval.usec would be (<time> % 1000)
                                        Default is 1
```

## Use Examples

```
sphere-linked-server --control-port 63000 --port-start 61000 --port-limit 300 --tls-key /tls/privkey.pem --tls-cert /tls/cert
```
This example opens the control port at `host:63000`, at most tunnels `300` services. (the first being `host:61000`, and the last being `host:61299`),   
using `/tls/privkey.pem` and `/tls/cert` to establish TLS connections between server and client.

```
sphere-linked-client --host-addr test.example.com --host-port 63000 --service-addr 192.168.1.100 --service-port 8080
```
This example connects to server via `test.example.com:63000`, and tunnels `192.168.1.100:8080` to `test.example.com`.