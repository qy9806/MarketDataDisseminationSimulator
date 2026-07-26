#include "event_hanlder.h"

namespace EventHandler {

void apply_event(OrderBookManager &mgr, std::unordered_map<InstrumentId, Instrument> &instruments,
                 const feeder::FeedEvent &ev) {
    auto it = instruments.find(ev.order.intrumentid_);
    if (it == instruments.end())
        return;
    Instrument &instrument = it->second;
    Order order = ev.order; // apply_add_order takes Order& -> needs an lvalue
    switch (ev.action) {
    case feeder::FeedAction::ADD:
        mgr.apply_add_order(order, instrument);
        break;
    case feeder::FeedAction::REMOVE:
        mgr.apply_remove_order(order.order_id);
        break;
    case feeder::FeedAction::REPLACE:
        mgr.apply_replace_order(order, instrument);
        break;
    }
}

std::optional<feeder::FeedEvent> next_event(std::ifstream &feed) {
    std::string line;
    while (std::getline(feed, line)) {
        if (auto ev = feeder::parse_feed_line(line)) {
            // if it doesn't parse continue
            return ev;
        }
    }
    return std::nullopt;
}

} // namespace EventHandler
