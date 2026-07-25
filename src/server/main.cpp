// Phase 2: consume the CSV feed into the authoritative book AND disseminate it.
// main() only orchestrates -- the real work lives in the modules it calls:
//   feeder       (feed_reader)  : CSV line  -> FeedEvent
//   EventHandler (event_hanlder): FeedEvent -> mutate the book / next_event
//   Translate    (translate)    : Order/book -> wire protobuf
//   feeder       (framing/tcp)  : wire msg   -> bytes on the socket

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include "event_hanlder.h"
#include "framing.h"
#include "market_data.pb.h"
#include "orderbook.h"
#include "tcp_socket.h"
#include "translate.h"

namespace {
namespace wire = MarketDataDisseminationProtoBuff; // generated protobuf types
constexpr uint16_t kPort = 9001;

std::unordered_map<InstrumentId, Instrument> make_instruments() {
    return {{"AAPL", Instrument{"AAPL", "AAPL", 10}}};
}

} // namespace

int main(int argc, char **argv) {
    const std::string feed_path = (argc > 1) ? argv[1] : "data/feed.csv";

    auto instruments = make_instruments();
    OrderBookManager manager;

    std::ifstream feed(feed_path);
    if (!feed) {
        std::cerr << "[server] cannot open feed " << feed_path << "\n";
        return EXIT_FAILURE;
    }

    // 1. Seed: replay the first few events into the book before anyone connects
    //    (the "market already running"). Same open file -- streaming continues below.
    constexpr int kSeedLines = 5;
    for (int i = 0; i < kSeedLines; ++i) {
        auto ev = EventHandler::next_event(feed);
        if (!ev)
            break;
        EventHandler::apply_event(manager, instruments, *ev);
    }
    std::cout << "[server] seeded order book" << std::endl;

    // 2. Wait for one subscriber.
    feeder::TcpSocket listening_socket = feeder::TcpSocket::listen_on(kPort);
    std::cout << "[server] listening on " << kPort << ", waiting for client...\n";
    feeder::TcpSocket conn = listening_socket.accept();
    std::cout << "[server] client connected\n";

    // 3. Send the whole book set as a Snapshot to seed the subscriber.
    uint64_t seq = 0;
    auto snap = Translate::build_snapshot(seq, manager);
    feeder::send_message(conn, snap);
    std::cout << "[server] sent snapshot (" << snap.books_size() << " instruments)\n";

    // 4. Stream the rest of the feed live: getline -> apply -> disseminate.
    while (auto ev = EventHandler::next_event(feed)) {
        EventHandler::apply_event(manager, instruments, *ev);
        wire::Incremental inc;
        inc.set_seq(++seq);
        inc.set_action(Translate::to_wire_action(ev->action));
        *inc.mutable_order() = Translate::to_wire(ev->order);
        if (!feeder::send_message(conn, inc)) {
            std::cerr << "[server] client disconnected\n";
            break;
        }
        std::cout << "[server] sent incremental seq=" << seq << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    std::cout << "[server] feed exhausted, done\n";
    return EXIT_SUCCESS;
}
