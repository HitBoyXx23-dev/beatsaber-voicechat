// Quest entry point. The game-facing hooks (mic capture, lobby packets, mute button UI)
// are not implemented yet. This file only wires up the mod lifecycle and config.
#include "voice_core.hpp"

namespace {
voicechat::MicState gMic;
}

// Standard mod-loader exports. The names and signatures follow the scotland2 / qpm template.
extern "C" __attribute__((visibility("default"))) void setup(void* /*modInfo*/) {
    // Logging and config setup goes here once the modloader headers are available.
}

extern "C" __attribute__((visibility("default"))) void load() {
    // TODO: install Beat Saber hooks (lobby join, mic button) with beatsaber-hook.
    // gMic holds mute state until those hooks are added.
    (void)gMic;
}
