#include "sender.h"

#include "framing.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

namespace Sender {

void run(const networkFrame::TcpSocket &connection_socket, std::queue<wire::MarketInfo> &info_queue,
         SpinLock &lock, const std::atomic<bool> &running) {
    while (true) {
        lock.lock();
        if (info_queue.empty()) {
            const bool done = !running.load();
            lock.unlock();
            if (done)
                break; // producers finished and nothing left
            std::this_thread::sleep_for(std::chrono::microseconds(10)); // spin-wait
            continue;
        }

        wire::MarketInfo info = std::move(info_queue.front());
        info_queue.pop();
        lock.unlock(); // never hold the lock across the (blocking) send

        // The MarketInfo envelope goes on the wire as-is; the client
        // discriminates via body_case().
        if (!networkFrame::send_message(connection_socket, info)) {
            std::cerr << "[sender] client disconnected\n";
            break;
        }
    }
    std::cout << "[sender] exiting\n";
}

} // namespace Sender
