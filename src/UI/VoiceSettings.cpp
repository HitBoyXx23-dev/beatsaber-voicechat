#include "UI/VoiceSettings.hpp"
#include "Config/VoiceConfig.hpp"
#include "Voice/VoiceChatController.hpp"

#include "bsml/shared/BSML.hpp"

namespace VoiceChat::UI {

namespace {

void SaveAndApply() {
    Config::Save();
    VoiceChatController::get_instance()->ApplySettings();
}

}  // namespace

void VoiceSettings::Register() {
    BSML::Register::RegisterSettingsMenu("Voice Chat", &VoiceSettings::DidActivate, false);
}

void VoiceSettings::DidActivate(HMUI::ViewController* self, bool firstActivation, bool, bool) {
    if (!firstActivation || !self) return;

    auto& settings = Config::Get();
    auto container = BSML::Lite::CreateScrollableSettingsContainer(self->get_transform());
    auto parent = container->get_transform();

    BSML::Lite::CreateToggle(parent, "Enable voice chat", settings.enabled, [](bool value) {
        Config::Get().enabled = value;
        SaveAndApply();
    });

    BSML::Lite::CreateToggle(parent, "Start muted in lobbies", settings.startMuted, [](bool value) {
        Config::Get().startMuted = value;
        SaveAndApply();
    });

    BSML::Lite::CreateToggle(parent, "Push-to-talk", settings.pushToTalk, [](bool value) {
        Config::Get().pushToTalk = value;
        SaveAndApply();
    });

    BSML::Lite::CreateDropdown(parent, "Push-to-talk button", Config::PttButtonName(settings.pttButton),
        Config::PttButtonNames(), [](StringW value) {
            Config::Get().pttButton = Config::PttButtonIndex(static_cast<std::string>(value));
            SaveAndApply();
        });

    BSML::Lite::CreateText(parent, "Push-to-talk: hold the button to talk. Otherwise use the panel in the lobby to mute or unmute.",
        TMPro::FontStyles::Italic, 3.0f);
}

}  // namespace VoiceChat::UI
