#include "tcp_socket.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <string>

namespace networkFrame {

namespace {
void fail(const char *what) {
    throw std::runtime_error(std::string(what) + ": " + std::strerror(errno));
}
} // namespace

TcpSocket::~TcpSocket() { close_fd(); }

void TcpSocket::close_fd() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

TcpSocket TcpSocket::listen_on(uint16_t port, int backlog) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0); // IPV4, TCP, >=0 ON SUCCESS

    // step-1 os returns back a file discriptor

    if (fd < 0)
        fail("socket failed to create");

    int yes = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes,
                 sizeof(yes)); // allowing port to be reused immediatly
    // step-2 set up soketoption on through on file descriptor

    sockaddr_in addr{};
    addr.sin_family = AF_INET;                // IPV4
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // listen on all the machines' network
    addr.sin_port = htons(port);

    // step-3 create a socketaddr object set up its config

    if (::bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        fail("bind");
    }
    // step-4: call bind to claim the addres we declared through the file descriptor
    //  bind returns 0 on  sccesss, if succeded, we now bind the socket to the port

    if (::listen(fd, backlog) < 0) {
        ::close(fd);
        fail("listen");
    }
    return TcpSocket(fd);
}

TcpSocket TcpSocket::accept() const {
    int connection_socket_fd =
        ::accept(fd_, nullptr, nullptr); // blocks until a client connects, >=0 on success

    // step-1 accept() runs on the listening socket (fd_). it pulls one completed
    //        connection off the queue and hands back a NEW fd for that connection socket
    //        (distinct from fd_, which keeps listening for more clients).
    //        nullptr, nullptr = we don't bother capturing the client's addr/port.

    if (connection_socket_fd < 0)
        fail("accept");

    int yes = 1;
    ::setsockopt(connection_socket_fd, IPPROTO_TCP, TCP_NODELAY, &yes,
                 sizeof(yes)); // disable Nagle on THIS connection
    // step-2 TCP_NODELAY is per-connection, so set it on conn (not the listener).
    //        flush small writes out immediately - we want latency over batching.
    /* what is nagle?, i notice th esocket setup option is different form listiening port,
    wha tis IPPROTO_TCP vs SOL_SOCKET? and whjat does tcp nodelay  configure?  it fluses all teh
    packet int eh socket immediately upon receive?

    */

    return TcpSocket(connection_socket_fd); // step-3 wrap the connection-socket fd and return it
}

TcpSocket TcpSocket::connect_to(const std::string &host, uint16_t port) {
    addrinfo hints{};
    hints.ai_family = AF_INET;       // IPV4
    hints.ai_socktype = SOCK_STREAM; // TCP

    // step-1 hints is a FILTER telling getaddrinfo what kind of address we want back
    //        (here: IPv4 + TCP). the {} zeroes the rest of the struct.

    addrinfo *res = nullptr;
    const std::string port_str = std::to_string(port);
    if (::getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0) {
        fail("getaddrinfo");
    }

    // step-2 getaddrinfo resolves host + port (e.g. "127.0.0.1" / "localhost" + "9001")
    //        into a linked-list of candidate address structs (res). this is the name-
    //        resolution step (handles DNS too). returns 0 on success. NOTE: no bind() -
    //        the client's local ephemeral port is auto-assigned by connect() below.

    /* so essentialy getaddrinfo is a syscall underthe hood to see if oskernal can resovle
    this address to a real conenction port or what? and it binds the actual resovled addrinto
    to the res? what type is addressinfo?
    */

    int fd = -1;
    for (addrinfo *p = res; p != nullptr; p = p->ai_next) {
        fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0)
            continue;
        if (::connect(fd, p->ai_addr, p->ai_addrlen) == 0)
            break; // 0 = connected, stop
        ::close(fd);
        fd = -1;
    }
    // step-3 a host can resolve to MULTIPLE addresses, so walk the list: make a socket,
    //        try to connect(). first one that succeeds -> break. if it fails, close this
    //        fd and try the next candidate.
    /* but for our case we wil be only rinng the look for one right, since we runnign our host
    only one instance on given address/port
    */

    ::freeaddrinfo(res); // step-4 free the list getaddrinfo malloc'd for us (cleanup)
    if (fd < 0)
        fail("connect"); //       if fd still < 0, every candidate failed

    int yes = 1;
    ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes,
                 sizeof(yes)); // disable Nagle on this connection (same latency reason as accept)
    // step-5 TCP_NODELAY per-connection again - send small writes out immediately

    /* so it reutrns the client' side connection socket? client side only have on socket rihgt , no
     * listening socket */
    return TcpSocket(fd); // step-6 wrap the connection-socket fd and return it
}

bool TcpSocket::send_all(const void *buf, size_t len) const {
    /* why buf is a void ptr? does it mean it it a raw address ? */
    const auto *p = static_cast<const uint8_t *>(buf);
    /* you are casting to a unsigned char ptr? */
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = ::send(fd_, p + sent, len - sent, 0);
        /*
        return the amoutn of bytes that kernal has accepted to teh sending buff
        */
        if (n <= 0) {
            if (n < 0 && errno == EINTR)
                continue;
            std::cerr << " failed to send buffer";
            /* failed to accept buffer */
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    return true;
}

void TcpSocket::shutdown_read() const {
    if (fd_ >= 0)
        ::shutdown(fd_, SHUT_RD);
}

bool TcpSocket::recv_all(void *buf, size_t len) const {
    /* as a client how am i suppose to know how mych bytes shoudld i expected? where
    does the len coming from
    */
    auto *p = static_cast<uint8_t *>(buf);
    size_t got = 0;
    while (got < len) {
        ssize_t n = ::recv(fd_, p + got, len - got, 0);
        if (n == 0)
            return false; // peer closed
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return false;
        }
        got += static_cast<size_t>(n);
    }

    return true;
}

} // namespace networkFrame
