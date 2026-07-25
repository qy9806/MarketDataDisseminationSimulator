#pragma once

#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>

#include "feed_reader.h"
#include "orderbook.h"

// EventHandler: drives the authoritative book from the feed. Protobuf-free.
namespace EventHandler {

// Dispatch one feed event onto the book: ADD/REMOVE/REPLACE -> the matching apply_*.
void apply_event(OrderBookManager& mgr,
                 std::unordered_map<InstrumentId, Instrument>& instruments,
                 const feeder::FeedEvent& ev);

// Pull the next parseable event off the feed, skipping header/blank/bad rows.
// Returns nullopt at end of file.
std::optional<feeder::FeedEvent> next_event(std::ifstream& feed);

}  // namespace EventHandler
