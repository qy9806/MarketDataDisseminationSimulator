#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <list>
#include <memory>
struct LevelEntry{
    uint64_t price;
    uint64_t quantity;
};
enum class Side{
    BUY,
    SELL
};

using LevelList = std::list<LevelEntry>; // to store bids level and ask level
using InstrumentId = uint64_t;

struct Instrument{
    InstrumentId id;
    std::string  symbol;
    int          depth;
};
struct Orderbook{
int total_depth;
LevelList bids_list;
LevelList asks_list;
};


struct OrderBookManager{
std::unordered_map<InstrumentId , std::shared_ptr<Orderbook>> book_per_isntrument_map;

bool apply_add_bid( LevelEntry entry, Instrument& instrument);
bool apply_add_ask( LevelEntry entry, Instrument& instrument);
bool apply_remove(LevelEntry entry, Side side, InstrumentId ins_id);
bool apply_replace(LevelEntry entry);

};

