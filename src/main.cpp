// Quest entry point. Lobby hooks and the mute button are set up here.
#include "_config.h"
#include "logging.hpp"
#include "UI/VoiceMuteButton.hpp"
#include "scotland2/shared/loader.hpp"

modloader::ModInfo modInfo{MOD_ID, VERSION, VERSION_LONG};

BEATTOGETHER_EXPORT_FUNC void setup(CModInfo* info) {
    info->id = MOD_ID;
    info->version = VERSION;
    info->version_long = VERSION_LONG;
}

BEATTOGETHER_EXPORT_FUNC void late_load() {
    il2cpp_functions::Init();
    INFO("VoiceChat loaded");
    // Hooks in src/Hooks install themselves. The mute button is shown when the lobby
    // controller attaches, so it is not created here.
}
