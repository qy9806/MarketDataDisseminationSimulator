// Phase 0 client: connect to the server, receive one Snapshot, print the book.
// No reconstruction / gap logic yet -- just proves the wire works.

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <print>
#include "framing.h"
#include "market_data.pb.h"
#include "tcp_socket.h"
#include "test_message.pb.h"

namespace {
constexpr uint16_t kPort = 9001;
}

int main() {
    const std::string host = "127.0.0.1";

    MarketDataDisseminator::TcpSocket client_connection_sock = MarketDataDisseminator::TcpSocket::connect_to(host, kPort);
    std::cout << "[client] connected to " << host << ":" << kPort << "\n";

    MarketDataDisseminator::TestMessage t1{};


    if (!MarketDataDisseminator::recv_message(client_connection_sock, t1)) {
        std::cerr << "[client] failed to receive messgae\n";
        return EXIT_FAILURE;
    }
    std::print("message receiged: {}", t1.text() );
    return EXIT_SUCCESS; //just reutnr 1 dont use macro 
}

