// Phase 3: consume the CSV feed into the authoritative book AND disseminate it
// over three threads that share one queue (guarded by one spin lock):
//   MarketDataFeedListener : CSV -> book -> MarketInfo(action)  [producer]
//   ClientRequestListener  : SnapshotRequest -> MarketInfo(snapshot) [producer]
//   Sender                 : drain queue -> wire  [sole socket writer / consumer]
// main() only seeds the book, accepts the client, and spawns/joins the threads.

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

#include "client_request_listener.h"
#include "event_hanlder.h"
#include "market_data.pb.h"
#include "market_data_feed_listener.h"
#include "orderbook.h"
#include "sender.h"
#include "spin_lock.h"
#include "tcp_socket.h"
#include "translate.h"

namespace {
namespace wire = MarketDataDisseminationProtoBuff; // generated protobuf types
constexpr uint16_t kPort = 9001;

std::unordered_map<InstrumentId, Instrument> make_instruments() {
    return {
        {"AAPL", Instrument{"AAPL", "AAPL", 10}},
        {"MSFT", Instrument{"MSFT", "MSFT", 10}},
        {"NVDA", Instrument{"NVDA", "NVDA", 10}},
    };
}

} // namespace

int main() {
    // Relative to the binary's working dir: run from build/ so ../ is the repo root.
    const std::string feed_path = "../data/feed_big.csv";

    auto instruments = make_instruments();
    OrderBookManager manager;
    std::queue<wire::MarketInfo> info_queue;
    SpinLock spin_lock{};
    uint64_t seq = 0;

    std::ifstream feed(feed_path);
    if (!feed) {
        std::cerr << "[server] cannot open feed " << feed_path << "\n";
        return EXIT_FAILURE;
    }

    // 1. Seed: replay the first few events into the book before anyone connects
    //    (the "market already running"). Same open file -- the feed thread
    //    resumes streaming from here.
    constexpr int kSeedLines = 5;
    for (int i = 0; i < kSeedLines; ++i) {
        auto feed_event = EventHandler::next_event(feed);
        if (!feed_event)
            break;
        EventHandler::apply_event(manager, instruments, *feed_event);
    }
    std::cout << "[server] seeded order book" << std::endl;

    // 2. Wait for one subscriber.
    networkFrame::TcpSocket listening_socket = networkFrame::TcpSocket::listen_on(kPort);
    std::cout << "[server] listening on " << kPort << ", waiting for client...\n";
    networkFrame::TcpSocket connection_socket = listening_socket.accept();
    std::cout << "[server] client connected\n";

    // 3. Queue the initial full Snapshot (single-threaded here -- no lock yet).
    {
        wire::MarketInfo info;
        *info.mutable_snapshot() = Translate::build_snapshot(seq, manager);
        info_queue.push(std::move(info));
    }

    // 4. Spawn the three threads. `running` lets main tell the sender to stop
    //    once the producers are done and the queue has drained.
    std::atomic<bool> running{true};

    std::thread sender_thread(
        [&] { Sender::run(connection_socket, info_queue, spin_lock, running); });

    std::thread feed_thread([&] {
        MarketDataFeedListener::run(feed, manager, instruments, info_queue, spin_lock, seq);
    });

    std::thread request_thread([&] {
        ClientRequestListener::run(connection_socket, manager, info_queue, spin_lock, seq);
    });

    // 5. The feed drives the lifetime: when it exhausts, wind everything down.
    feed_thread.join(); // all MarketActions produced

    connection_socket.shutdown_read(); // unblock the request listener's recv
    request_thread.join();             // no more snapshots will be produced

    running.store(false); // let the sender drain the tail, then exit
    sender_thread.join();

    std::cout << "[server] done\n";
    return EXIT_SUCCESS;
}
