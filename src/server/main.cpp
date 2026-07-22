// Phase 0 smoke test: accept one client and send a single raw C-string
// to prove the raw-TCP pipe works. No protobuf / framing / book yet.

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "tcp_socket.h"
#include "test_message.pb.h"
#include "framing.h"

namespace {
constexpr uint16_t kPort = 9001;
}  // namespace

int main() {

    MarketDataDisseminator::TcpSocket listening_socket =
        MarketDataDisseminator::TcpSocket::listen_on(kPort);
    // set up lisitng socket an dbinds to port 9001;
    std::cout << "[server] listening on port " << kPort << ", waiting for client...\n";

    MarketDataDisseminator::TcpSocket connection_socket = listening_socket.accept();
    // return a connection socket

    std::cout << "[server] client connected\n";

    const std::string msg = "this is a test";
    MarketDataDisseminator::TestMessage test_msg{};
    test_msg.set_id(100);
    test_msg.set_text(msg);

    auto res = MarketDataDisseminator::send_message(connection_socket, test_msg);
    if(!res){
        std::cerr<<"unble to send mesage";
        return 0;
    }
    std::cout << "[server] sent: " << msg << "\n";



    return EXIT_SUCCESS;
}
