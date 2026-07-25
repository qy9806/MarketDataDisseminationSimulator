#pragma once

#include <google/protobuf/message_lite.h>

#include "tcp_socket.h"

namespace feeder {

// Our entire "application-layer protocol": a 4-byte big-endian length prefix
// followed by the protobuf payload. This is the framing gRPC/HTTP-2 would
// otherwise do for us -- we own it instead.

// Serialize `msg`, prefix with its length, and write it to the socket.
bool send_message(const TcpSocket& sock, const google::protobuf::MessageLite& msg);

// Read one length-prefixed frame from the socket and parse it into `msg`.
bool recv_message(const TcpSocket& sock, google::protobuf::MessageLite& msg);

}  // namespace feeder
