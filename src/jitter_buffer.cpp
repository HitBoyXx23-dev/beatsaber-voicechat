#include "jitter_buffer.hpp"

namespace voicechat {

void JitterBuffer::Push(int32_t index, std::vector<uint8_t> data) {
    if (nextIndex_ && index < *nextIndex_) return;  // too late, already moved past it
    pending_[index] = std::move(data);
}

std::optional<std::vector<uint8_t>> JitterBuffer::Pop() {
    if (pending_.size() < depth_) return std::nullopt;

    // If packets are missing, the oldest one we hold is released anyway,
    // so a lost packet never stalls playback.
    auto it = pending_.begin();
    nextIndex_ = it->first + 1;
    std::vector<uint8_t> out = std::move(it->second);
    pending_.erase(it);
    return out;
}

void JitterBuffer::Reset() {
    pending_.clear();
    nextIndex_.reset();
}

}  // namespace voicechat
