#include "feed_reader.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace feeder {

namespace {

bool parse_action(const std::string& s, FeedAction& out) {
    if (s == "ADD") { out = FeedAction::ADD; return true; }
    if (s == "REMOVE") { out = FeedAction::REMOVE; return true; }
    if (s == "REPLACE") { out = FeedAction::REPLACE; return true; }
    return false;  // also rejects the "action" header cell
}

bool parse_side(const std::string& s, Side& out) {
    if (s == "BUY") { out = Side::BUY; return true; }
    if (s == "SELL") { out = Side::SELL; return true; }
    return false;
}

}  // namespace

std::optional<FeedEvent> parse_feed_line(const std::string& line) {
    if (line.empty()) return std::nullopt;

    std::stringstream ss(line);
    std::string action_s, order_id, instrument, side_s, price_s, qty_s;
    if (!std::getline(ss, action_s, ',')) return std::nullopt;
    if (!std::getline(ss, order_id, ',')) return std::nullopt;
    if (!std::getline(ss, instrument, ',')) return std::nullopt;
    if (!std::getline(ss, side_s, ',')) return std::nullopt;
    if (!std::getline(ss, price_s, ',')) return std::nullopt;
    if (!std::getline(ss, qty_s, ',')) return std::nullopt;

    FeedAction action;
    Side side;
    if (!parse_action(action_s, action)) return std::nullopt;
    if (!parse_side(side_s, side)) return std::nullopt;

    try {
        uint64_t price = std::stoull(price_s);
        uint64_t quantity = std::stoull(qty_s);
        return FeedEvent{action, Order(order_id, price, quantity, side, instrument)};
    } catch (const std::exception&) {
        return std::nullopt;  // non-numeric price/qty
    }
}

std::vector<FeedEvent> load_feed(const std::string& path) {
    std::vector<FeedEvent> events;
    std::ifstream file(path);
    if (!file) {
        std::cerr << "[feed] cannot open " << path << "\n";
        return events;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (auto ev = parse_feed_line(line)) events.push_back(*ev);
    }
    return events;
}

}  // namespace feeder
