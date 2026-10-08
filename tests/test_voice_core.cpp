// Host-side tests for voice_core. Run with build.bat (no Quest toolchain needed).
#include <cstdio>
#include "../src/voice_core.hpp"

using namespace voicechat;

static int failures = 0;

#define CHECK(cond)                                                  \
    do {                                                             \
        if (!(cond)) {                                               \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            failures++;                                              \
        }                                                            \
    } while (0)

int main() {
    // Round trip.
    VoicePacket in{42, {1, 2, 3, 250, 255}};
    auto bytes = EncodeVoicePacket(in);
    CHECK(bytes.size() == kHeaderSize + 5);
    auto out = DecodeVoicePacket(bytes);
    CHECK(out.has_value());
    if (out) {
        CHECK(out->index == 42);
        CHECK(out->data == in.data);
    }

    // Checksum wraps on overflow: 255 * 300 fits in uint32 without wrapping,
    // so use enough 0xFF bytes to exceed 2^32 is impractical; check the value directly.
    std::vector<uint8_t> big(300, 0xFF);
    CHECK(ComputeChecksum(big) == 300 * 255);

    // Empty payload is valid.
    auto empty = DecodeVoicePacket(EncodeVoicePacket({0, {}}));
    CHECK(empty.has_value() && empty->data.empty());

    // Rejections.
    auto tampered = bytes;
    tampered.back() ^= 0x01;  // data changed, checksum no longer matches
    CHECK(!DecodeVoicePacket(tampered).has_value());

    auto wrongVersion = bytes;
    wrongVersion[0] = 99;
    CHECK(!DecodeVoicePacket(wrongVersion).has_value());

    auto truncated = bytes;
    truncated.pop_back();
    CHECK(!DecodeVoicePacket(truncated).has_value());

    CHECK(!DecodeVoicePacket({}).has_value());

    // Mute state.
    MicState mic;
    CHECK(!mic.IsMuted() && mic.ShouldTransmit());
    CHECK(mic.Toggle() == true);
    CHECK(!mic.ShouldTransmit());
    CHECK(mic.Toggle() == false);
    CHECK(mic.ShouldTransmit());

    if (failures == 0) std::printf("All voice_core tests passed.\n");
    return failures == 0 ? 0 : 1;
}
