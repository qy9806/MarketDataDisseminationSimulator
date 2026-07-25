#include <memory>
#include <orderbook.h>
#include <vector>

bool OrderBookManager::apply_add_order(Order &order, Instrument &instrument) {

    if (!book_per_isntrument_map.contains(instrument.id)) {
        auto book_ptr = std::make_shared<Orderbook>();
        book_ptr->total_depth = instrument.depth;
        book_per_isntrument_map[instrument.id] = book_ptr;
    } // set up instrument order book

    auto cur_order_book = book_per_isntrument_map.at(instrument.id);

    if (order.side_ == Side::BUY) {
        auto order_ptr = std::make_shared<Order>(order.order_id, order.price, order.quantity,
                                                 order.side_, order.intrumentid_);

        auto &bids_list_at_cur_price = cur_order_book->bids_map[order.price];
        auto itr = bids_list_at_cur_price.insert(bids_list_at_cur_price.end(), order_ptr);
        OrderEntry_map[order.order_id] = OrderEntry(itr, order_ptr);
    } else {
        auto order_ptr = std::make_shared<Order>(order.order_id, order.price, order.quantity,
                                                 order.side_, order.intrumentid_);

        auto &asks_list_at_cur_price = cur_order_book->asks_map[order.price];
        auto itr = asks_list_at_cur_price.insert(asks_list_at_cur_price.end(), order_ptr);
        OrderEntry_map[order.order_id] = OrderEntry(itr, order_ptr);
    }

    return true;
}

bool OrderBookManager::apply_remove_order(OrderId order_id) {

    if (!OrderEntry_map.contains(order_id)) {
        std::cerr << "Order id" << order_id << " does not exsting in teh orderEntry map";
        return false;
    }
    auto order_entry = OrderEntry_map[order_id];
    auto location = order_entry.location_;
    auto order_price = order_entry.order_ptr->price;
    auto order_side = order_entry.order_ptr->side_;
    auto order_instrument_id = order_entry.order_ptr->intrumentid_;

    auto instrument_order_map = book_per_isntrument_map.at(order_instrument_id);
    if (order_side == Side::BUY) {
        auto &order_book_buy_map_bid_list = instrument_order_map->bids_map[order_price];
        order_book_buy_map_bid_list.erase(location);
        if (order_book_buy_map_bid_list.empty()) {
            instrument_order_map->bids_map.erase(order_price);
        }
        OrderEntry_map.erase(order_id);
    } else {
        auto &order_book_ask_list = instrument_order_map->asks_map[order_price];
        order_book_ask_list.erase(location);
        if (order_book_ask_list.empty()) {
            instrument_order_map->asks_map.erase(order_price);
        }
        OrderEntry_map.erase(order_id);
    }

    return true;
}

bool OrderBookManager::apply_replace_order(Order new_order, Instrument &instrument) {
    auto order_id = new_order.order_id;
    if (!apply_remove_order(order_id)) {
        return false;
    }
    return apply_add_order(new_order, instrument);
}
