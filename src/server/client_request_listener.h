#pragma once

#include <atomic>
#include <cstdint>
#include <queue>

#include "market_data.pb.h"
#include "orderbook.h"
#include "spin_lock.h"
#include "tcp_socket.h"

// server thread2, listing to client reqeust of snapshot reqeust
namespace ClientRequestListener {

namespace wire = MarketDataDisseminationProtoBuff; // generated protobuf types

void run(const networkFrame::TcpSocket &connection_socket, const OrderBookManager &manager,
         std::queue<wire::MarketInfo> &info_queue, SpinLock &lock, const uint64_t &seq,
         std::atomic_int32_t &info_queue_size);

} // namespace ClientRequestListener
