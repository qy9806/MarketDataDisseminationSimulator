#pragma once

#include <cstdint>
#include <fstream>
#include <queue>
#include <unordered_map>

#include "market_data.pb.h"
#include "orderbook.h"
#include "spin_lock.h"

// Producer thread #1: the market-data feed. Streams the CSV, applies each event
// to the authoritative book, and pushes a MarketInfo(action) onto the shared
// queue for the sender to drain. Returns when the feed is exhausted.
//
// `lock` is the shared spin lock guarding book mutation + `seq` bump + queue
// push (same lock the request listener and sender use).
namespace MarketDataFeedListener {

namespace wire = MarketDataDisseminationProtoBuff; // generated protobuf types

void run(std::ifstream &feed, OrderBookManager &manager,
         std::unordered_map<InstrumentId, Instrument> &instruments,
         std::queue<wire::MarketInfo> &info_queue, SpinLock &lock, uint64_t &seq,
         std::atomic_int32_t &info_queue_size);

} // namespace MarketDataFeedListener
