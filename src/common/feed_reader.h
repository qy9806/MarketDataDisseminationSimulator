#pragma once

#include <optional>
#include <string>
#include <vector>

#include "orderbook.h"

namespace feeder {

// The book mutation a feed row asks for -- one per OrderBookManager::apply_* method.
enum class FeedAction { ADD, REMOVE, REPLACE };

// One replayed market-data event: an action + the order it acts on.
struct FeedEvent {
    FeedAction action;
    Order order;
};

// Parse one CSV row "action,order_id,instrument,side,price,quantity" into an event.
// Returns nullopt on a blank line, the header row, or any malformed field.
std::optional<FeedEvent> parse_feed_line(const std::string& line);

// Read a whole CSV feed file into an ordered list of events (bad/header rows skipped).
std::vector<FeedEvent> load_feed(const std::string& path);

}  // namespace feeder
