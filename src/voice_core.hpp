// Platform-independent voice logic: mute state and the VoiceData packet format.
// Kept free of game/Unity code so it can be unit tested on a PC (see build.bat).
#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace voicechat {

// Packet layout (version 1), little-endian, as documented in
// MultiplayerExtensions.VoiceChat:
//   byte  PacketVersion
//   int32 Index
//   int32 Checksum   (sum of the data bytes, wrapping)
//   int32 DataLength
//   byte[DataLength] Data
constexpr uint8_t kPacketVersion = 1;
constexpr size_t kHeaderSize = 1 + 4 + 4 + 4;

struct VoicePacket {
    int32_t index = 0;
    std::vector<uint8_t> data;
};

// Sum of bytes, wrapping on overflow, as the wire format specifies.
int32_t ComputeChecksum(const std::vector<uint8_t>& data);

std::vector<uint8_t> EncodeVoicePacket(const VoicePacket& packet);

// Returns nullopt for truncated, wrong-version, or checksum-mismatched input.
std::optional<VoicePacket> DecodeVoicePacket(const std::vector<uint8_t>& bytes);

// Tracks whether the local microphone is muted. Mute state persists across lobbies.
class MicState {
public:
    bool IsMuted() const { return muted_; }
    // Returns the new muted state.
    bool Toggle() { muted_ = !muted_; return muted_; }
    void SetMuted(bool muted) { muted_ = muted; }

    // Outgoing audio is dropped while muted.
    bool ShouldTransmit() const { return !muted_; }

private:
    bool muted_ = false;
};

}  // namespace voicechat
