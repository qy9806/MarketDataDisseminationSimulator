#pragma once

#include <cstdint>

#include "feed_reader.h"
#include "market_data.pb.h"
#include "orderbook.h"

// Translate: the seam between the internal book model and the wire format.
// The ONLY module that includes both orderbook.h and the generated protobuf.
namespace Translate {

namespace wire = MarketDataDisseminationProtoBuff;  // generated protobuf types

// book-side types -> wire types
wire::Side to_wire_side(Side s);
wire::Action to_wire_action(feeder::FeedAction a);
wire::Order to_wire(const Order& o);

// Full L3 snapshot: one InstrumentBook per instrument the manager holds.
wire::Snapshot build_snapshot(uint64_t seq, const OrderBookManager& mgr);

}  // namespace Translate
