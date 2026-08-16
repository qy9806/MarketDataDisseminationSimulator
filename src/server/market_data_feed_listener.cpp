#include "market_data_feed_listener.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

#include "event_hanlder.h"
#include "translate.h"

namespace MarketDataFeedListener {

void run(std::ifstream &feed, OrderBookManager &manager,
         std::unordered_map<InstrumentId, Instrument> &instruments,
         std::queue<wire::MarketInfo> &info_queue, SpinLock &lock, uint64_t &seq,
         std::atomic_int32_t &info_queue_size) {
    while (auto ev = EventHandler::next_event(feed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // pace the feed

        // Book mutation + seq bump under the lock, so the request listener sees
        // a consistent (book, seq) pair when it snapshots.
        lock.lock();
        EventHandler::apply_event(manager, instruments, *ev);
        const uint64_t s = ++seq;
        lock.unlock();

        // Build the wire message off the shared state (no lock needed here).
        wire::MarketInfo info;
        wire::MarketAction *action = info.mutable_action();
        action->set_seq(s);
        action->set_action(Translate::to_wire_action(ev->action));
        *action->mutable_order() = Translate::to_wire(ev->order);

        // Hand off to the sender.
        lock.lock();
        info_queue.push(std::move(info));
        info_queue_size++;
        lock.unlock();

        std::cout << "[feed] produced seq=" << s << "\n";
    }
    std::cout << "[feed] exhausted, exiting\n";
}

} // namespace MarketDataFeedListener
