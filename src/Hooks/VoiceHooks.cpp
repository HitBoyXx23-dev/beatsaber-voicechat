#include "hooking.hpp"
#include "logging.hpp"
#include "Voice/VoiceChatController.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "multiplayer-core/shared/Networking/MpPacketSerializer.hpp"
#include "GlobalNamespace/MultiplayerSessionManager.hpp"

#define VOICECHAT_INSTALL_HOOK(name_)                                                                         \
    struct Auto_Hook_##name_ {                                                                                \
        static void Install() {                                                                               \
            static constexpr auto logger = Paper::ConstLoggerContext(MOD_ID "_Install_" #name_);               \
            ::Hooking::InstallHook<Hook_##name_>(logger);                                                     \
        }                                                                                                     \
        Auto_Hook_##name_() { VoiceChat::Hooking::AddInstallFunc(Install); }                                  \
    };                                                                                                        \
    static Auto_Hook_##name_ Auto_Hook_Instance_##name_

MAKE_HOOK_CHECKED_FIND_CLASS(VoiceChat_PacketSerializer_Initialize,
    &MultiplayerCore::Networking::MpPacketSerializer::Initialize,
    "MultiplayerCore.Networking", "MpPacketSerializer", "Initialize", void,
    MultiplayerCore::Networking::MpPacketSerializer* self) {
    VoiceChat_PacketSerializer_Initialize(self);
    VoiceChat::VoiceChatController::get_instance()->Attach(self);
}

MAKE_HOOK_CHECKED_FIND_CLASS(VoiceChat_PacketSerializer_Dispose,
    &MultiplayerCore::Networking::MpPacketSerializer::Dispose,
    "MultiplayerCore.Networking", "MpPacketSerializer", "Dispose", void,
    MultiplayerCore::Networking::MpPacketSerializer* self) {
    VoiceChat::VoiceChatController::get_instance()->Detach();
    VoiceChat_PacketSerializer_Dispose(self);
}

MAKE_HOOK_CHECKED_FIND_CLASS(VoiceChat_SessionManager_LateUpdate,
    &GlobalNamespace::MultiplayerSessionManager::LateUpdate, "GlobalNamespace", "MultiplayerSessionManager", "LateUpdate",
    void, GlobalNamespace::MultiplayerSessionManager* self) {
    VoiceChat_SessionManager_LateUpdate(self);
    if (VoiceChat::VoiceChatController::get_instance()->IsActive()) {
        VoiceChat::VoiceChatController::get_instance()->MainThreadTick();
    }
}

VOICECHAT_INSTALL_HOOK(VoiceChat_PacketSerializer_Initialize);
VOICECHAT_INSTALL_HOOK(VoiceChat_PacketSerializer_Dispose);
VOICECHAT_INSTALL_HOOK(VoiceChat_SessionManager_LateUpdate);
