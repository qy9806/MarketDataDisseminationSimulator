#pragma once

#include "market_data.pb.h"

// Client-side handling of decoded feed messages -> console output.
//   Snapshot                 -> print every instrument's full book
//   MarketAction (ADD/REMOVE/REPLACE) -> print the single event
namespace ParseEvent {

namespace wire = MarketDataDisseminationProtoBuff;  // generated protobuf types

void print_snapshot(const wire::Snapshot& snap);
void print_action(const wire::MarketAction& action);

}  // namespace ParseEvent
