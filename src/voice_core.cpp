#include "voice_core.hpp"

namespace voicechat {

namespace {

void WriteInt32(std::vector<uint8_t>& out, int32_t value) {
    uint32_t u = static_cast<uint32_t>(value);
    for (int i = 0; i < 4; i++) out.push_back(static_cast<uint8_t>((u >> (8 * i)) & 0xFF));
}

int32_t ReadInt32(const uint8_t* p) {
    uint32_t u = 0;
    for (int i = 0; i < 4; i++) u |= static_cast<uint32_t>(p[i]) << (8 * i);
    return static_cast<int32_t>(u);
}

}  // namespace

int32_t ComputeChecksum(const std::vector<uint8_t>& data) {
    uint32_t sum = 0;
    for (uint8_t b : data) sum += b;
    return static_cast<int32_t>(sum);
}

std::vector<uint8_t> EncodeVoicePacket(const VoicePacket& packet) {
    std::vector<uint8_t> out;
    out.reserve(kHeaderSize + packet.data.size());
    out.push_back(kPacketVersion);
    WriteInt32(out, packet.index);
    WriteInt32(out, ComputeChecksum(packet.data));
    WriteInt32(out, static_cast<int32_t>(packet.data.size()));
    out.insert(out.end(), packet.data.begin(), packet.data.end());
    return out;
}

std::optional<VoicePacket> DecodeVoicePacket(const std::vector<uint8_t>& bytes) {
    if (bytes.size() < kHeaderSize) return std::nullopt;
    if (bytes[0] != kPacketVersion) return std::nullopt;

    int32_t index = ReadInt32(&bytes[1]);
    int32_t checksum = ReadInt32(&bytes[5]);
    int32_t length = ReadInt32(&bytes[9]);
    if (length < 0 || bytes.size() != kHeaderSize + static_cast<size_t>(length)) return std::nullopt;

    VoicePacket packet;
    packet.index = index;
    packet.data.assign(bytes.begin() + kHeaderSize, bytes.end());
    if (ComputeChecksum(packet.data) != checksum) return std::nullopt;
    return packet;
}

}  // namespace voicechat
