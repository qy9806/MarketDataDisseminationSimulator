#include "client_request_listener.h"

#include <iostream>
#include <utility>

#include "framing.h"
#include "translate.h"

namespace ClientRequestListener {

void run(const networkFrame::TcpSocket &connection_socket,
         const OrderBookManager &manager,
         std::queue<wire::MarketInfo> &info_queue,
         SpinLock &lock,
         const uint64_t &seq) {
    wire::SnapshotRequest request;

    // Block on the socket's recv half; each decoded request => one snapshot.
    // recv_message returns false when the read half is shut down (main's clean-
    // exit signal) or the peer closes, which ends the loop.
    while (networkFrame::recv_message(connection_socket, request)) {
        wire::MarketInfo info;

        // Hold the lock across build_snapshot: it walks the whole book, which
        // the feed thread mutates. Reading `seq` under the same lock keeps
        // snapshot.seq in step with the last event applied to the book.
        lock.lock();
        *info.mutable_snapshot() = Translate::build_snapshot(seq, manager);
        info_queue.push(std::move(info));
        lock.unlock();

        std::cout << "[request] snapshot requested -> queued (seq=" << seq << ")\n";
    }

    std::cout << "[request] read half closed, exiting\n";
}

}  // namespace ClientRequestListener
