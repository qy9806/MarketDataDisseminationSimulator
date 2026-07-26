#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

namespace networkFrame {

// Thin RAII wrapper around a POSIX TCP socket file descriptor.
// Non-copyable, movable. The whole "protocol" lives above this in framing.h.
class TcpSocket {
  public:
    TcpSocket() = default;
    explicit TcpSocket(int fd) : fd_(fd) {}
    ~TcpSocket();

    // Server side: create a listening socket bound to `port`.
    static TcpSocket listen_on(uint16_t port, int backlog = 16);

    // Block until a client connects; returns the accepted connection.
    TcpSocket accept() const;

    // Client side: connect to host:port.
    static TcpSocket connect_to(const std::string &host, uint16_t port);

    // Loop until exactly `len` bytes are sent / received.
    // Returns false on error or peer close before `len` bytes.
    bool send_all(const void *buf, size_t len) const;
    bool recv_all(void *buf, size_t len) const;

    // Half-close the READ direction. Unblocks another thread parked in recv_all
    // on this same fd (its recv returns 0/EOF) while leaving the write half open
    // -- lets a sender thread keep writing during a clean listener shutdown.
    void shutdown_read() const;

    bool valid() const { return fd_ >= 0; }
    int fd() const { return fd_; }

  private:
    void close_fd();
    int fd_ = -1;
};

} // namespace networkFrame
