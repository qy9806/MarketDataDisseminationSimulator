#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

enum class Side { BUY, SELL };
using InstrumentId = std::string;
using OrderId = std::string;

struct Order {

    OrderId order_id;
    uint64_t price;
    uint64_t quantity;
    Side side_;
    InstrumentId intrumentid_;

    Order(OrderId order_id, uint64_t price, uint64_t quantity, Side side,
          InstrumentId instrument_id)
        : order_id(std::move(order_id)), price(price), quantity(quantity), side_(side),
          intrumentid_(instrument_id) {}
};

using OrderPtr = std::shared_ptr<Order>;
using OrderPtrs = std::list<OrderPtr>;

struct Instrument {
    InstrumentId id;
    std::string symbol;
    int depth;
};
struct Orderbook {
    int total_depth;

    std::map<uint64_t, OrderPtrs, std::greater<uint64_t>> bids_map;
    std::map<uint64_t, OrderPtrs, std::less<uint64_t>> asks_map;
};

struct OrderBookManager {
    struct OrderEntry {
        OrderPtrs::iterator location_;
        OrderPtr order_ptr;
    };
    std::unordered_map<InstrumentId, std::shared_ptr<Orderbook>> book_per_isntrument_map;
    std::unordered_map<OrderId, OrderEntry> OrderEntry_map;

    bool apply_add_order(Order &order, Instrument &instrument);
    bool apply_remove_order(OrderId order_id);
    bool apply_replace_order(Order new_order, Instrument &instrument);
};
