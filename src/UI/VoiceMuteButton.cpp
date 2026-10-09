#include "UI/VoiceMuteButton.hpp"
#include "Config/VoiceConfig.hpp"
#include "Voice/VoiceChatController.hpp"
#include "logging.hpp"

#include <string>

#include "bsml/shared/BSML.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/UI/Button.hpp"

namespace {

BSML::FloatingScreen* screen = nullptr;
UnityEngine::UI::Button* muteButton = nullptr;

const UnityEngine::Vector2 kPanelSize{24.0f, 8.0f};
const UnityEngine::Vector3 kPanelPosition{-1.7f, 1.1f, 1.6f};
const UnityEngine::Vector3 kPanelRotation{10.0f, -47.0f, 0.0f};

constexpr std::string_view kLiveLabel = "<color=#59FF73>MIC ON</color>";
constexpr std::string_view kMutedLabel = "<color=#FF5959>MIC OFF</color>";

void OnMuteClicked() {
    VoiceChat::VoiceChatController::get_instance()->ToggleMute();
    VoiceChat::UI::VoiceMuteButton::Refresh();
}

}  // namespace

namespace VoiceChat::UI {

void VoiceMuteButton::Show() {
    if (screen) {
        Refresh();
        return;
    }

    screen = BSML::Lite::CreateFloatingScreen(kPanelSize, kPanelPosition, kPanelRotation, 0.0f, false, false);
    if (!screen) {
        ERROR("Failed to create the voice chat button");
        return;
    }

    muteButton = BSML::Lite::CreateUIButton(screen->get_transform(), "", DEFAULT_BUTTONTEMPLATE, {0, 0}, kPanelSize, OnMuteClicked);
    BSML::Lite::SetButtonTextSize(muteButton, 3.5f);
    Refresh();
}

void VoiceMuteButton::Hide() {
    if (!screen) return;
    UnityEngine::Object::Destroy(screen->get_gameObject());
    screen = nullptr;
    muteButton = nullptr;
}

void VoiceMuteButton::Refresh() {
    if (!screen || !muteButton) return;

    auto& settings = Config::Get();
    bool muted = VoiceChatController::get_instance()->IsMuted();
    std::string label(muted ? kMutedLabel : kLiveLabel);

    if (settings.pushToTalk) {
        label += " (hold " + std::string(Config::PttButtonName(settings.pttButton)) + ")";
        muteButton->set_interactable(false);
    } else {
        muteButton->set_interactable(true);
    }
    BSML::Lite::SetButtonText(muteButton, label);
}

}  // namespace VoiceChat::UI
