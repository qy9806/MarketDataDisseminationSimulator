#include <memory>
#include <orderbook.h>
#include <vector>

bool OrderBookManager::apply_add_bid(LevelEntry entry, Instrument &instrument) {

    if (!book_per_isntrument_map.contains(instrument.id)) {
        auto book_ptr = std::make_shared<Orderbook>();
        book_ptr->total_depth = instrument.depth;
        book_ptr->bids_list.push_back(std::move(entry));
        book_per_isntrument_map[instrument.id] = book_ptr;

    } else {
        auto book_ptr = book_per_isntrument_map[instrument.id];
        book_ptr->bids_list.push_back(std::move(entry));
    }
    return true;
}

bool OrderBookManager::apply_add_ask(LevelEntry entry, Instrument &instrument) {

    if (!book_per_isntrument_map.contains(instrument.id)) {
        auto book_ptr = std::make_shared<Orderbook>();
        book_ptr->total_depth = instrument.depth;
        book_ptr->asks_list.push_back(std::move(entry));
        book_per_isntrument_map[instrument.id] = book_ptr;
    } else {
        auto book_ptr = book_per_isntrument_map[instrument.id];
        book_ptr->asks_list.push_back(std::move(entry));
    }
    return true;
}

bool OrderBookManager::apply_remove(LevelEntry entry, Side side, InstrumentId ins_id) {
    if (!book_per_isntrument_map.contains(ins_id)) {
        std::cerr << " unable to find instrument book";
        return false;
    }
    auto order_book_ptr = book_per_isntrument_map.at(ins_id);

    auto &side_list = (side == Side::BUY) ? order_book_ptr->bids_list : order_book_ptr->asks_list;

    auto removed = std::erase_if(
        side_list, [&](const LevelEntry &listentry) { return entry.price == listentry.price; });
    return removed > 0;
}

bool apply_remove(LevelEntry entry);
bool apply_replace(LevelEntry entry);