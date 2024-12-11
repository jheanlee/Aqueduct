# Sphere Linked

Sphere Linked is a simple TCP Tunneling service that enables user to connect to a service in a private network without exposing other ports or devices.

## Usage

### Host (server)

```
sphere-linked-server [OPTIONS]
```

#### Options

```
    -h, --help                      Prints usage
    -p, --control-port <port>       Client will connect to localhost:<port>
                                    Should be identical with --host-port of client
                                    Default is 3000
    -s, --port-start <port>         The proxy port of the first client will be <port>, the last being (<port> + port-limit - 1)
                                    Default is 51000
    -l, --port-limit <count>        Proxy ports will have a limit of <count> ports
                                    Default is 200
```

#### Notes

The host's control port and proxy port range should be accessable to client and external users

### Client

```
sphere-linked-client [OPTIONS]
```

#### Options

```
    -h, --help                      Prints usage
    -H, --host-addr <ipv4|domain>   Sets host to <ipv4|domain>
                                    Default is 0.0.0.0
    -P, --host-port <port>          Uses host:<port> as control port (see --control-port of server)
                                    Default is 3000
    -s, --service-addr <ipv4>       Sets the address of service to be tunneled to <ipv4>
                                    Default is 0.0.0.0
    -p, --service-port <port>       Tunnels service:<port> to host
                                    This option is required
```

## Use Example

```
sphere-linked-server --control-port 63000 --port-start 61000 --port-limit 300
```
This opens the control port at host:63000, at most tunnels 300 services. (first being host:61000, and last being host:61299)

```
sphere-linked-client --host-addr test.example.com --host-port 63000 --service-addr 192.168.1.100 --service-port 8080
```
This tunnels 192.168.1.100:8080 to test.example.com, and connects to server via test.example.com:63000.