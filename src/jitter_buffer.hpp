// Reorders incoming voice packets by index and drops ones that arrive too late.
// Pure logic, no game or audio APIs, so it is tested on PC.
#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace voicechat {

class JitterBuffer {
public:
    // depth: how many packets to hold before releasing the oldest one.
    explicit JitterBuffer(size_t depth = 3) : depth_(depth) {}

    // Adds a packet. Packets older than what was already released are dropped.
    void Push(int32_t index, std::vector<uint8_t> data);

    // Returns the next packet in order once the buffer holds `depth` packets,
    // or nullopt if it should wait for more. Skips over missing indices.
    std::optional<std::vector<uint8_t>> Pop();

    size_t Size() const { return pending_.size(); }
    void Reset();

private:
    size_t depth_;
    std::map<int32_t, std::vector<uint8_t>> pending_;
    std::optional<int32_t> nextIndex_;  // next index expected to be released
};

}  // namespace voicechat
