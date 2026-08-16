#include "sender.h"

#include "framing.h"
#include "market_data.pb.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

namespace Sender {

void run(const networkFrame::TcpSocket &connection_socket, std::queue<wire::MarketInfo> &info_queue,
         SpinLock &lock, const std::atomic<bool> &running, std::atomic_int32_t &info_queue_size) {
    while (true) {
        if (info_queue_size == 0) {
            const bool done = !running.load();
            if (done) {
                break; // producers finished and nothing left
            }

            continue;
        }

        int by_batch = false;

        lock.lock(); // lock the queue, start to drain

        if (!by_batch) {
            wire::MarketInfo info = std::move(info_queue.front());
            info_queue.pop();
            info_queue_size--;
            lock.unlock(); // never hold the lock across the (blocking) send

            // The MarketInfo envelope goes on the wire as-is; the client
            // discriminates via body_case().
            if (!networkFrame::send_message(connection_socket, info)) {
                std::cerr << "[sender] client disconnected\n";
                break;
            }
        } else {
            // by batch
            std::vector<wire::MarketInfo> infos;
            while (info_queue.size()) {
                infos.push_back(std::move(info_queue.front()));
                info_queue.pop();
                info_queue_size--;
            }
            lock.unlock();
            if (!networkFrame::send_message(connection_socket, infos)) {
                std::cerr << "[sender] client disconnected\n";
                break;
            }
        }
    }
    std::cout << "[sender] exiting\n";
}

} // namespace Sender
