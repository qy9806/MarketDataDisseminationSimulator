#include "parse_event.h"

#include <iostream>

namespace ParseEvent {

namespace {

const char* side_name(wire::Side s) { return s == wire::BUY ? "BUY" : "SELL"; }

const char* action_name(wire::Action a) {
    switch (a) {
    case wire::ADD:     return "ADD";
    case wire::REMOVE:  return "REMOVE";
    case wire::REPLACE: return "REPLACE";
    default:            return "?";
    }
}

void print_order(const wire::Order& o) {
    std::cout << "    " << side_name(o.side()) << " " << o.order_id()
              << " price=" << o.price() << " qty=" << o.quantity() << "\n";
}

}  // namespace

void print_snapshot(const wire::Snapshot& snap) {
    std::cout << "[client] SNAPSHOT seq=" << snap.seq() << "\n";
    for (const auto& book : snap.books()) {
        std::cout << "  " << book.instrument() << "\n";
        std::cout << "   bids:\n";
        for (const auto& o : book.bids()) print_order(o);
        std::cout << "   asks:\n";
        for (const auto& o : book.asks()) print_order(o);
    }
}

void print_action(const wire::MarketAction& action) {
    const wire::Order& o = action.order();
    std::cout << "[client] " << action_name(action.action())
              << " seq=" << action.seq()
              << " " << o.instrument()
              << " " << side_name(o.side())
              << " " << o.order_id()
              << " price=" << o.price()
              << " qty=" << o.quantity() << "\n";
}

}  // namespace ParseEvent
