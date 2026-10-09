#include "_config.h"
#include "hooking.hpp"
#include "logging.hpp"
#include "Config/VoiceConfig.hpp"
#include "UI/VoiceSettings.hpp"
#include "bsml/shared/BSML.hpp"
#include "scotland2/shared/loader.hpp"

modloader::ModInfo modInfo{MOD_ID, VERSION, VERSION_LONG};

VOICECHAT_EXPORT_FUNC void setup(CModInfo* info) {
    info->id = MOD_ID;
    info->version = VERSION;
    info->version_long = VERSION_LONG;
}

VOICECHAT_EXPORT_FUNC void late_load() {
    il2cpp_functions::Init();
    VoiceChat::Config::Load();
    BSML::Init();
    VoiceChat::UI::VoiceSettings::Register();
    VoiceChat::Hooking::InstallBootstrapHook();
    INFO("Voice chat late_load complete");
}
