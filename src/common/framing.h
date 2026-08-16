#pragma once

#include <google/protobuf/message_lite.h>

#include <vector>

#include "market_data.pb.h"
#include "tcp_socket.h"

namespace networkFrame {

namespace wire = MarketDataDisseminationProtoBuff; // generated protobuf types

// Serialize `msg`, prefix with its length, and write it to the socket.
bool send_message(const TcpSocket &sock, const google::protobuf::MessageLite &msg);

bool send_message(const TcpSocket &sock, const std::vector<wire::MarketInfo> &msgs);

// Read one length-prefixed frame from the socket and parse it into `msg`.
bool recv_message(const TcpSocket &sock, google::protobuf::MessageLite &msg);

} // namespace networkFrame
