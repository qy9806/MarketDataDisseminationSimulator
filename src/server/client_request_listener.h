#pragma once

#include <cstdint>
#include <queue>

#include "market_data.pb.h"
#include "orderbook.h"
#include "spin_lock.h"
#include "tcp_socket.h"

// Producer thread #2: client requests. Blocks on the (shared) connection socket
// reading SnapshotRequests; for each, builds a full-book Snapshot wrapped in a
// MarketInfo and pushes it onto the shared queue for the sender to drain.
//
// build_snapshot reads the whole book, which the feed thread mutates, so `lock`
// here is the SAME lock the feed thread uses -- it guards book + queue + seq.
// Returns when the socket's read half is shut down / the peer closes.
namespace ClientRequestListener {

namespace wire = MarketDataDisseminationProtoBuff;  // generated protobuf types

void run(const networkFrame::TcpSocket &connection_socket,
         const OrderBookManager &manager,
         std::queue<wire::MarketInfo> &info_queue,
         SpinLock &lock,
         const uint64_t &seq);

}  // namespace ClientRequestListener
