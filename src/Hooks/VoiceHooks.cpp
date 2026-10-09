#include "hooking.hpp"
#include "logging.hpp"
#include "Voice/VoiceChatController.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "multiplayer-core/shared/Networking/MpPacketSerializer.hpp"
#include "GlobalNamespace/MultiplayerSessionManager.hpp"
#include "GlobalNamespace/GameServerLobbyFlowCoordinator.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "custom-types/shared/register.hpp"

#define VOICECHAT_QUEUE_HOOK(name_)                                                                           \
    struct Auto_Hook_##name_ {                                                                                \
        static void Install() {                                                                               \
            static constexpr auto logger = Paper::ConstLoggerContext(MOD_ID "_Install_" #name_);               \
            ::Hooking::InstallHook<Hook_##name_>(logger);                                                     \
        }                                                                                                     \
        Auto_Hook_##name_() { ::VoiceChat::Hooking::AddInstallFunc(Install); }                                \
    };                                                                                                        \
    static Auto_Hook_##name_ Auto_Hook_Instance_##name_

static bool gameplayReady = false;
static MultiplayerCore::Networking::MpPacketSerializer* pendingSerializer = nullptr;

MAKE_HOOK_MATCH(VoiceChat_MainMenu_DidActivate, &GlobalNamespace::MainMenuViewController::DidActivate, void, GlobalNamespace::MainMenuViewController* self, bool firstActivation, bool addedToHierarchy,
    bool screenSystemEnabling) {
    VoiceChat_MainMenu_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    VoiceChat::Hooking::EnsureGameplayReady();
}

using SubSerializer = GlobalNamespace::INetworkPacketSubSerializer_1<GlobalNamespace::IConnectedPlayer*>;

static MultiplayerCore::Networking::MpPacketSerializer* AsMpPacketSerializer(SubSerializer* subSerializer) {
    if (!subSerializer) return nullptr;
    auto cast = il2cpp_utils::try_cast<MultiplayerCore::Networking::MpPacketSerializer>(subSerializer);
    return cast.has_value() ? cast.value() : nullptr;
}

MAKE_HOOK_MATCH(VoiceChat_SessionManager_RegisterSerializer,
    &GlobalNamespace::MultiplayerSessionManager::RegisterSerializer, void,
    GlobalNamespace::MultiplayerSessionManager* self, GlobalNamespace::MultiplayerSessionManager_MessageType serializerType,
    SubSerializer* subSerializer) {
    VoiceChat_SessionManager_RegisterSerializer(self, serializerType, subSerializer);
    if (auto* serializer = AsMpPacketSerializer(subSerializer)) {
        pendingSerializer = serializer;
        if (gameplayReady) VoiceChat::VoiceChatController::get_instance()->Arm(serializer);
    }
}

MAKE_HOOK_MATCH(VoiceChat_SessionManager_UnregisterSerializer,
    &GlobalNamespace::MultiplayerSessionManager::UnregisterSerializer, void,
    GlobalNamespace::MultiplayerSessionManager* self, GlobalNamespace::MultiplayerSessionManager_MessageType serializerType,
    SubSerializer* subSerializer) {
    if (AsMpPacketSerializer(subSerializer)) {
        pendingSerializer = nullptr;
        VoiceChat::VoiceChatController::get_instance()->Disarm();
    }
    VoiceChat_SessionManager_UnregisterSerializer(self, serializerType, subSerializer);
}

MAKE_HOOK_MATCH(VoiceChat_SessionManager_LateUpdate, &GlobalNamespace::MultiplayerSessionManager::LateUpdate, void, GlobalNamespace::MultiplayerSessionManager* self) {
    VoiceChat_SessionManager_LateUpdate(self);
    if (VoiceChat::VoiceChatController::get_instance()->IsRuntimeActive()) {
        VoiceChat::VoiceChatController::get_instance()->MainThreadTick();
    }
}

MAKE_HOOK_MATCH(VoiceChat_Lobby_DidActivate, &GlobalNamespace::GameServerLobbyFlowCoordinator::DidActivate, void, GlobalNamespace::GameServerLobbyFlowCoordinator* self, bool firstActivation,
    bool addedToHierarchy, bool screenSystemEnabling) {
    VoiceChat_Lobby_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    if (VoiceChat::VoiceChatController::get_instance()->IsArmed()) {
        VoiceChat::VoiceChatController::get_instance()->StartRuntime();
    }
}

MAKE_HOOK_MATCH(VoiceChat_Lobby_DidDeactivate, &GlobalNamespace::GameServerLobbyFlowCoordinator::DidDeactivate, void, GlobalNamespace::GameServerLobbyFlowCoordinator* self, bool removedFromHierarchy,
    bool screenSystemDisabling) {
    VoiceChat::VoiceChatController::get_instance()->StopRuntime();
    VoiceChat_Lobby_DidDeactivate(self, removedFromHierarchy, screenSystemDisabling);
}

VOICECHAT_QUEUE_HOOK(VoiceChat_SessionManager_RegisterSerializer);
VOICECHAT_QUEUE_HOOK(VoiceChat_SessionManager_UnregisterSerializer);
VOICECHAT_QUEUE_HOOK(VoiceChat_SessionManager_LateUpdate);
VOICECHAT_QUEUE_HOOK(VoiceChat_Lobby_DidActivate);
VOICECHAT_QUEUE_HOOK(VoiceChat_Lobby_DidDeactivate);

void VoiceChat::Hooking::InstallBootstrapHook() {
    static bool installed = false;
    if (installed) return;
    installed = true;
    static constexpr auto logger = Paper::ConstLoggerContext(MOD_ID "_Install_Bootstrap");
    ::Hooking::InstallHook<Hook_VoiceChat_MainMenu_DidActivate>(logger);
    InstallHooks();
    INFO("Voice chat hooks installed");
}

void VoiceChat::Hooking::EnsureGameplayReady() {
    if (gameplayReady) return;
    custom_types::Register::AutoRegister();
    gameplayReady = true;
    if (pendingSerializer) VoiceChat::VoiceChatController::get_instance()->Arm(pendingSerializer);
    INFO("Voice chat gameplay ready");
}
