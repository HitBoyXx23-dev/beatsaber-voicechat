// Quest entry point. Lobby hooks and the mute button are set up here.
#include "_config.h"
#include "hooking.hpp"
#include "logging.hpp"
#include "custom-types/shared/register.hpp"
#include "scotland2/shared/loader.hpp"

modloader::ModInfo modInfo{MOD_ID, VERSION, VERSION_LONG};

VOICECHAT_EXPORT_FUNC void setup(CModInfo* info) {
    info->id = MOD_ID;
    info->version = VERSION;
    info->version_long = VERSION_LONG;
}

VOICECHAT_EXPORT_FUNC void late_load() {
    il2cpp_functions::Init();
    custom_types::Register::AutoRegister();  // registers VoicePacket
    VoiceChat::Hooking::InstallHooks();      // lobby hooks in src/Hooks
    INFO("VoiceChat loaded");
}
