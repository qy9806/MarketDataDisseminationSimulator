#pragma once

#include <atomic>
#include <queue>

#include "market_data.pb.h"
#include "spin_lock.h"
#include "tcp_socket.h"

// Consumer thread: the ONLY writer of the connection socket. Spin-drains the
// shared queue and puts each MarketInfo envelope on the wire as-is; the client
// discriminates Snapshot vs MarketAction via body_case(). Because it is the
// sole writer, no lock guards the socket -- `lock` only guards the queue ops.
//
// Exits when `running` is false AND the queue is empty (clean shutdown after
// the producers finish), or when a send fails (client disconnected).
namespace Sender {

namespace wire = MarketDataDisseminationProtoBuff; // generated protobuf types

void run(const networkFrame::TcpSocket &connection_socket, std::queue<wire::MarketInfo> &info_queue,
         SpinLock &lock, const std::atomic<bool> &running);

} // namespace Sender
