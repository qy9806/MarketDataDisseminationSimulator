// Phase 2 subscriber: receive a Snapshot, then apply a stream of Incrementals
// onto a mirror book and print top-of-book after each update.
//
// The mirror is keyed by order_id, so the three actions collapse to:
//   ADD / REPLACE -> insert-or-overwrite   (REPLACE reuses the same order_id)
//   REMOVE        -> erase

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

#include "framing.h"
#include "market_data.pb.h"
#include "tcp_socket.h"

namespace {
namespace MDD = feeder;    // our transport/framing lib
namespace wire = MarketDataDisseminationProtoBuff;    // generated protobuf types
constexpr uint16_t kPort = 9001;

// order_id -> the order's current state. This IS the mirror book.
using Mirror = std::unordered_map<std::string, wire::Order>;

// Aggregate the L3 mirror to an L2 view and print the best few levels.
void print_top(const Mirror& mirror) {
    std::map<uint64_t, uint64_t, std::greater<uint64_t>> bids;  // price -> total qty, desc
    std::map<uint64_t, uint64_t> asks;                          // price -> total qty, asc
    for (const auto& [id, o] : mirror) {
        if (o.side() == wire::BUY) bids[o.price()] += o.quantity();
        else                     asks[o.price()] += o.quantity();
    }

    std::cout << "  ---- book ----\n";
    int shown = 0;
    for (const auto& [price, qty] : asks) {  // best (lowest) ask first
        std::cout << "   ASK " << price << " x" << qty << "\n";
        if (++shown == 3) break;
    }
    shown = 0;
    for (const auto& [price, qty] : bids) {  // best (highest) bid first
        std::cout << "   BID " << price << " x" << qty << "\n";
        if (++shown == 3) break;
    }
}

}  // namespace

int main(int argc, char** argv) {
    const std::string host = (argc > 1) ? argv[1] : "127.0.0.1";

    MDD::TcpSocket conn = MDD::TcpSocket::connect_to(host, kPort);
    std::cout << "[client] connected to " << host << ":" << kPort << "\n";

    Mirror mirror;

    // 1. Snapshot -- seed the mirror from full book state.
    wire::Snapshot snap;
    if (!MDD::recv_message(conn, snap)) {
        std::cerr << "[client] failed to receive snapshot\n";
        return EXIT_FAILURE;
    }
    for (const auto& book : snap.books()) {
        for (const auto& o : book.bids()) mirror[o.order_id()] = o;
        for (const auto& o : book.asks()) mirror[o.order_id()] = o;
    }
    std::cout << "[client] snapshot seq=" << snap.seq() << " (" << mirror.size() << " orders)\n";
    print_top(mirror);

    // 2. Apply incrementals until the server closes the stream.
    wire::Incremental inc;
    while (MDD::recv_message(conn, inc)) {
        const wire::Order& o = inc.order();
        switch (inc.action()) {
        case wire::ADD:
        case wire::REPLACE: mirror[o.order_id()] = o; break;
        case wire::REMOVE:  mirror.erase(o.order_id()); break;
        default: break;
        }
        std::cout << "[client] incremental seq=" << inc.seq() << " action=" << inc.action() << "\n";
        print_top(mirror);
    }

    std::cout << "[client] stream closed\n";
    return EXIT_SUCCESS;
}
