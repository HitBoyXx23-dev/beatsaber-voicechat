// Connects the voice controller to the lobby. MpPacketSerializer is created by
// MultiplayerCore when a multiplayer session starts and disposed when it ends.
#include "hooking.hpp"
#include "logging.hpp"
#include "Voice/VoiceChatController.hpp"

MAKE_AUTO_HOOK_MATCH(VoiceChat_PacketSerializer_Initialize,
    &MultiplayerCore::Networking::MpPacketSerializer::Initialize, void,
    MultiplayerCore::Networking::MpPacketSerializer* self) {
    VoiceChat_PacketSerializer_Initialize(self);
    VoiceChat::VoiceChatController::get_instance()->Attach(self);
}

MAKE_AUTO_HOOK_MATCH(VoiceChat_PacketSerializer_Dispose,
    &MultiplayerCore::Networking::MpPacketSerializer::Dispose, void,
    MultiplayerCore::Networking::MpPacketSerializer* self) {
    VoiceChat::VoiceChatController::get_instance()->Detach();
    VoiceChat_PacketSerializer_Dispose(self);
}
