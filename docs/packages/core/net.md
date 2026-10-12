<!-- dadoc 1.0.0-rc.4 -->
<!-- commit 549ca2ce6a97 -->
# core:net

core:net: IPv4 and IPv6 addresses, DNS, and TCP and UDP sockets.

    import "core:net"

    try net.Endpoint at = net.parse_endpoint("127.0.0.1:8080") else why:
        fmt.println(net.message(why))
        return 1
    try net.Socket listener = net.tcp_listen(at, 16)
    defer net.close(listener)
    try net.Socket conn = net.accept(listener)
    defer net.close(conn)
    [512]u8 buf
    try i64 n = net.read(conn, buf[:])
    try net.write_all(conn, buf[0..<n])

An address is an `Ip`, IPv4 or IPv6 by its `family`, and a socket address
is an `Endpoint`: an `Ip` and a port. `parse_ip`, `parse_endpoint`,
`format_ip` and `format_endpoint` convert them to and from text, and
`resolve` asks DNS for a name's addresses. A `Socket` is a TCP listener, a
TCP connection or a UDP socket, opened by `tcp_listen`, `accept`,
`tcp_connect`, `dial` (a name or an address, resolved and tried in turn)
or `udp_bind`, and ended by `close`.

Sockets block until they can finish, unless `set_nonblocking` says
otherwise or a timeout from `set_read_timeout` or `set_write_timeout` runs
out first; `wait` asks whether a socket is ready, and `tcp_connect_timeout`
gives up on a peer that does not answer. An IPv6 socket here carries IPv6
alone (`IPV6_V6ONLY` is set), so a program serving both families opens one
socket for each, the same on every system.

**Tier N W H; sockets H only.** The address types, their parsing and
formatting, and `message` are pure Dado and work on every target. Sockets
and DNS run on Linux, macOS, the BSDs and Windows, each with an arm of its
own where the systems differ (`posix.dado`, `windows.dado`). On `#WEB` and
`#NONE` every function builds, and every socket call and every DNS lookup
fails `Error.UNSUPPORTED` (`stub.dado`): neither a page nor node gives a
WebAssembly module raw TCP or UDP, and bare metal has no network stack.
`resolve` of an address literal answers on every target, because it asks no
one.

**Errors.** Every failable function fails with a member of `Error`, the
same member for the same cause on every system: each arm translates its
own number (`errno`, a `getaddrinfo` code, `WSAGetLastError`) at the moment
of failure, and a number the table does not know is `Error.UNKNOWN`.
`message(why)` is a sentence for a person.

**What raises into a script.** The pure half: `ipv4`, `ipv6`, `loopback`,
`unspecified`, `is_loopback`, `same_ip`, `same_endpoint`, the parsers and
formatters, and `message`, each promised by the `#assert_raises` above it.
Every socket function and `resolve` calls the system through a `foreign`
declaration, which is the C boundary a script does not cross; on the web,
where they are Dado stubs, the promise still could not be made, because a
promise holds for every target the package builds for.

**Thread-safety.** No function here shares state but one: the first socket
call on Windows starts Winsock (`WSAStartup`) and records it. Two threads
doing that at once each start it, which Winsock counts and allows. A
`Socket` may be read on one thread and written on another; two threads
reading one socket get the bytes in an order nobody chose.

## Declarations

368 declarations, 44 public.

* `distinct enum i32 Error` — Why a call failed. The numbers start at 1001, apart from `core:os`'s.
* `string8 message(Error why)` — A sentence for `why`, for a person reading a tool's output.
* `enum i32 Family` — An address family.
* `type Ip: (Family family, [16]u8 bytes, u32 zone)` — An IP address, IPv4 or IPv6 by `family`. `bytes` are in network order: an…
* `type Endpoint: (Ip ip, u16 port)` — An IP address and a port: where a socket is bound or connected.
* `type Datagram: (i64 size, Endpoint from)` — What `recv_from` answers: how many bytes arrived, and who sent them.
* `Ip ipv4(u8 a, u8 b, u8 c, u8 d)` — The IPv4 address `a.b.c.d`.
* `Ip ipv6([16]u8 bytes, u32 zone)` — The IPv6 address of these sixteen bytes, in network order, in scope `zone`
* `Ip loopback(Family family)` — The loopback address of a family: 127.0.0.1 or ::1.
* `Ip unspecified(Family family)` — The unspecified address of a family, 0.0.0.0 or ::. Bound, it means every…
* `bool is_loopback(Ip ip)` — Whether `ip` is a loopback address: anything in 127.0.0.0/8, or ::1.
* `bool same_ip(Ip a, Ip b)` — Whether two addresses are the same: one family, the same bytes and, for…
* `bool same_endpoint(Endpoint a, Endpoint b)` — Whether two endpoints are the same address and port.
* `Error!Ip parse_ipv4(string8 text)` — A dotted-quad IPv4 address: four decimal numbers from 0 to 255, separated by…
* `Error!Ip parse_ipv6(string8 text)` — An IPv6 address in the text form of RFC 4291: eight groups of one to four…
* `Error!Ip parse_ip(string8 text)` — An IPv4 or IPv6 address, told apart by whether the text has a colon. Fails…
* `Error!u16 parse_port(string8 text)` — A port: a decimal number from 0 to 65535 with no sign and no spaces. Fails…
* `Error!Endpoint parse_endpoint(string8 text)` — An endpoint: `192.0.2.1:80` for IPv4, `[2001:db8::1]:80` or…
* `Error!ref []char8 format_ip(Ip ip)` — `ip` as text: `192.0.2.1`, or IPv6 in the canonical form of RFC 5952…
* `Error!ref []char8 format_endpoint(Endpoint at)` — `at` as text: `192.0.2.1:80`, or `[2001:db8::1]:80` for IPv6, which…
* `distinct type Socket: i64` — An open socket: a TCP listener, a TCP connection or a UDP socket. It is the…
* `enum i32 Shut` — Which half of a connection `shutdown` ends.
* `enum i32 Ready` — What `wait` waits for.
* `Error!Socket tcp_listen(Endpoint at, i32 backlog)` — A TCP socket listening at `at`, with room for `backlog` connections that…
* `Error!Socket accept(Socket listener)` — The next connection waiting on `listener`, as a new socket. Blocks until one…
* `Error!Socket tcp_connect(Endpoint to)` — A TCP connection to `to`. Blocks until the peer accepts or refuses, as long…
* `Error!Socket tcp_connect_timeout(Endpoint to, i32 timeout_ms)` — A TCP connection to `to`, giving up with `Error.TIMED_OUT` when the peer has…
* `Error!Socket dial(string8 host, u16 port, i32 timeout_ms = 5000)` — A TCP connection to `host:port`, where `host` is a name `resolve` looks up…
* `Error!i64 read(Socket s, []u8 into)` — Read what has arrived on `s` into `into`, up to its length, and answer how…
* `Error!i64 write(Socket s, []u8 from)` — Write some of `from` to `s` and answer how many bytes went, which can be…
* `Error!void write_all(Socket s, []u8 from)` — Write all of `from` to `s`, as many writes as it takes. On a failure, some…
* `Error!void shutdown(Socket s, Shut how)` — End one or both halves of a TCP connection while keeping the socket open:
* `void close(Socket s)` — Close `s`, giving its handle back to the system. Every socket a function…
* `Error!Socket udp_bind(Endpoint at)` — A UDP socket bound to `at`; port 0 binds a port the system picks, which…
* `Error!i64 send_to(Socket s, []u8 data, Endpoint to)` — Send `data` to `to` as one datagram from the UDP socket `s`, and answer how…
* `Error!Datagram recv_from(Socket s, []u8 into)` — Receive one datagram on the UDP socket `s` into `into`, and answer its size…
* `Error!Endpoint local_endpoint(Socket s)` — The address and port `s` is bound to: how to find the port the system…
* `Error!Endpoint peer_endpoint(Socket s)` — The address and port of the peer `s` is connected to. Fails…
* `Error!void set_nonblocking(Socket s, bool on)` — Make `s` nonblocking (`true`) or blocking again (`false`). A call on a…
* `Error!void set_read_timeout(Socket s, i32 ms)` — Give every read on `s` (and `accept` and `recv_from`) at most `ms`
* `Error!void set_write_timeout(Socket s, i32 ms)` — Give every write on `s` at most `ms` milliseconds; one that runs out fails…
* `Error!void set_nodelay(Socket s, bool on)` — Send small writes on the TCP connection `s` at once (`true`), rather than…
* `Error!bool wait(Socket s, Ready what, i32 timeout_ms)` — Wait up to `timeout_ms` milliseconds for `s` to be ready to read or to…
* `Error!ref []Ip resolve(string8 host)` — The addresses `host` names, in the order the system's resolver gives them…
