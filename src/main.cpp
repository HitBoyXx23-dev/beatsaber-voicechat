#include "_config.h"
#include "hooking.hpp"
#include "logging.hpp"
#include "scotland2/shared/loader.hpp"

modloader::ModInfo modInfo{MOD_ID, VERSION, VERSION_LONG};

VOICECHAT_EXPORT_FUNC void setup(CModInfo* info) {
    info->id = MOD_ID;
    info->version = VERSION;
    info->version_long = VERSION_LONG;
}

VOICECHAT_EXPORT_FUNC void late_load() {
    il2cpp_functions::Init();
    VoiceChat::Hooking::InstallBootstrapHook();
    INFO("Voice chat late_load complete");
}
