#include "framing.h"

#include <arpa/inet.h> // htonl / ntohl

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace networkFrame {

bool send_message(const TcpSocket &sock, const google::protobuf::MessageLite &msg) {
    const size_t payload_size = msg.ByteSizeLong();
    // total payload size

    std::vector<uint8_t> buf(sizeof(uint32_t) + payload_size);
    // buf about to be sent to wire, siz eif uint32_t ( 4 byts of header) + actual payload size

    const uint32_t len_header = htonl(static_cast<uint32_t>(payload_size));
    std::memcpy(buf.data(), &len_header, sizeof(len_header));
    // copy exactly 4 byte to the buff first 4 byte
    if (!msg.SerializeToArray(buf.data() + sizeof(uint32_t), static_cast<int>(payload_size))) {
        std::cerr << "Failed to serialize the message";
        return false;
    }

    // serize the msg to raw bytes?  why you pass buf.data() + 4?
    // buf.data() reutrn a ptr too origins start of buff , but does this operation also copy
    // the  rest of rntire of payuload? basicaly it put msg (serilzied) start form 5th ?
    return sock.send_all(buf.data(), buf.size());
}
bool send_message(const TcpSocket &sock, const std::vector<wire::MarketInfo> &msgs) {
    size_t current_total_size{};
    for (const auto &msg : msgs) {
        current_total_size += sizeof(uint32_t) + msg.ByteSizeLong();
    }
    std::vector<uint8_t> buff(current_total_size);

    auto cur_addr = buff.data();
    for (const auto &msg : msgs) {
        const auto current_mesage_size = msg.ByteSizeLong();

        // fill the buff
        uint32_t header = htonl(static_cast<uint32_t>(current_mesage_size));
        std::memcpy(cur_addr, &header, sizeof(header));
        if (!msg.SerializeToArray(cur_addr + sizeof(header), current_mesage_size)) {
            std::cerr << "Failed to serialize the message";
            return false;
        }
        cur_addr += sizeof(header) + current_mesage_size;
    }

    return sock.send_all(buff.data(), buff.size());
}
bool recv_message(const TcpSocket &sock, google::protobuf::MessageLite &msg) {
    uint32_t payload_size = 0;
    if (!sock.recv_all(&payload_size, sizeof(payload_size)))
        return false;
    const uint32_t payload_size_after_endian = ntohl(payload_size);

    std::vector<uint8_t> buf(payload_size_after_endian);
    if (payload_size_after_endian > 0 && !sock.recv_all(buf.data(), payload_size_after_endian))
        return false;

    return msg.ParseFromArray(buf.data(), static_cast<int>(payload_size_after_endian));
}

} // namespace networkFrame
