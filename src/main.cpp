// Quest entry point. Lobby wiring (creating a VoiceChatController and calling Attach
// with the injected MpPacketSerializer) is a TODO, since it needs a Zenject installer
// or MultiplayerCore hook.
#include "voice_core.hpp"

namespace {
voicechat::MicState gMic;
}

// Standard mod-loader exports. The names and signatures follow the scotland2 / qpm template.
extern "C" __attribute__((visibility("default"))) void setup(void* /*modInfo*/) {}

extern "C" __attribute__((visibility("default"))) void load() {
    // TODO: install the lobby hook that creates VoiceChatController and calls Attach().
    (void)gMic;
}
