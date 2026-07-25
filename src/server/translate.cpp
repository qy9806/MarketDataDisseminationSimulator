#include "translate.h"

namespace Translate {

wire::Side to_wire_side(Side s) { return s == Side::BUY ? wire::BUY : wire::SELL; }

wire::Action to_wire_action(feeder::FeedAction a) {
    switch (a) {
    case feeder::FeedAction::ADD:     return wire::ADD;
    case feeder::FeedAction::REMOVE:  return wire::REMOVE;
    case feeder::FeedAction::REPLACE: return wire::REPLACE;
    }
    return wire::ADD;
}

wire::Order to_wire(const Order& o) {
    wire::Order w;
    w.set_order_id(o.order_id);
    w.set_price(o.price);
    w.set_quantity(o.quantity);
    w.set_side(to_wire_side(o.side_));
    w.set_instrument(o.intrumentid_);
    return w;
}

wire::Snapshot build_snapshot(uint64_t seq, const OrderBookManager& mgr) {
    wire::Snapshot snap;
    snap.set_seq(seq);
    for (const auto& [id, book] : mgr.book_per_isntrument_map) {
        wire::InstrumentBook* ib = snap.add_books();
        ib->set_instrument(id);
        for (const auto& [price, orders] : book->bids_map)
            for (const auto& o : orders) *ib->add_bids() = to_wire(*o);
        for (const auto& [price, orders] : book->asks_map)
            for (const auto& o : orders) *ib->add_asks() = to_wire(*o);
    }
    return snap;
}

}  // namespace Translate
