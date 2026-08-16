// Phase 3 subscriber: read a stream of MarketInfo envelopes and dispatch on
// body_case(). Every 3rd action received, send a SnapshotRequest back on the
// same socket (gap-recovery style); the server answers with a snapshot that
// arrives inline in the stream.
//   MarketInfo{snapshot} -> print the whole book
//   MarketInfo{action}   -> print the event

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "framing.h"
#include "market_data.pb.h"
#include "parse_event.h"
#include "tcp_socket.h"

namespace {
namespace wire = MarketDataDisseminationProtoBuff;  // generated protobuf types
constexpr uint16_t kPort = 9001;
constexpr int kRequestEvery = 3;  // send a SnapshotRequest per N actions
}  // namespace

int main(int argc, char** argv) {
    const std::string host = (argc > 1) ? argv[1] : "127.0.0.1";

    networkFrame::TcpSocket conn = networkFrame::TcpSocket::connect_to(host, kPort);
    std::cout << "[client] connected to " << host << ":" << kPort << "\n";

    int action_count = 0;
    wire::MarketInfo info;
    while (networkFrame::recv_message(conn, info)) {
        switch (info.body_case()) {
        case wire::MarketInfo::kSnapshot:
            ParseEvent::print_snapshot(info.snapshot());
            break;

        case wire::MarketInfo::kAction:
            ParseEvent::print_action(info.action());
            // Every N actions, ask the server for a fresh full-book snapshot.
            if (++action_count % kRequestEvery == 0) {
                wire::SnapshotRequest request;
                networkFrame::send_message(conn, request);
                std::cout << "[client] -> sent SnapshotRequest (after " << action_count
                          << " actions)\n";
            }
            break;

        default:
            break;
        }
    }

    std::cout << "[client] stream closed\n";
    return EXIT_SUCCESS;
}
