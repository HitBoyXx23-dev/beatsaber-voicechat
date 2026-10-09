#include "UI/VoiceMuteButton.hpp"
#include "Config/VoiceConfig.hpp"
#include "Voice/VoiceChatController.hpp"
#include "logging.hpp"

#include <string>

#include "bsml/shared/BSML.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"
#include "TMPro/TextAlignmentOptions.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/TextAnchor.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"

namespace {

BSML::FloatingScreen* screen = nullptr;
HMUI::CurvedTextMeshPro* statusText = nullptr;
HMUI::CurvedTextMeshPro* hintText = nullptr;
UnityEngine::UI::Button* muteButton = nullptr;

const UnityEngine::Vector2 kPanelSize{44.0f, 26.0f};
const UnityEngine::Vector3 kPanelPosition{-1.0f, 0.85f, 1.9f};
const UnityEngine::Vector3 kPanelRotation{30.0f, -25.0f, 0.0f};

const UnityEngine::Color kLiveColor{0.35f, 1.0f, 0.45f, 1.0f};
const UnityEngine::Color kMutedColor{1.0f, 0.35f, 0.35f, 1.0f};
const UnityEngine::Color kHintColor{0.8f, 0.8f, 0.8f, 1.0f};

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

    screen = BSML::Lite::CreateFloatingScreen(kPanelSize, kPanelPosition, kPanelRotation, 0.0f, true, false);
    if (!screen) {
        ERROR("Failed to create the voice chat panel");
        return;
    }

    auto layout = BSML::Lite::CreateVerticalLayoutGroup(screen->get_transform());
    layout->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);
    layout->set_spacing(1.0f);
    layout->set_padding(UnityEngine::RectOffset::New_ctor(2, 2, 2, 2));
    auto parent = layout->get_transform();

    BSML::Lite::CreateText(parent, "Voice Chat", TMPro::FontStyles::Bold, 4.0f)->set_alignment(TMPro::TextAlignmentOptions::Center);

    statusText = BSML::Lite::CreateText(parent, "", TMPro::FontStyles::Bold, 7.0f);
    statusText->set_alignment(TMPro::TextAlignmentOptions::Center);

    hintText = BSML::Lite::CreateText(parent, "", TMPro::FontStyles::Italic, 3.0f);
    hintText->set_alignment(TMPro::TextAlignmentOptions::Center);
    hintText->set_color(kHintColor);

    muteButton = BSML::Lite::CreateUIButton(parent, "Mute", "PlayButton", {0, 0}, {34, 9}, OnMuteClicked);

    Refresh();
}

void VoiceMuteButton::Hide() {
    if (!screen) return;
    UnityEngine::Object::Destroy(screen->get_gameObject());
    screen = nullptr;
    statusText = nullptr;
    hintText = nullptr;
    muteButton = nullptr;
}

void VoiceMuteButton::Refresh() {
    if (!screen || !statusText || !hintText || !muteButton) return;

    auto& settings = Config::Get();
    bool muted = VoiceChatController::get_instance()->IsMuted();

    statusText->set_text(muted ? "MIC MUTED" : "MIC LIVE");
    statusText->set_color(muted ? kMutedColor : kLiveColor);

    if (settings.pushToTalk) {
        hintText->set_text("Hold " + std::string(Config::PttButtonName(settings.pttButton)) + " to talk");
        muteButton->get_gameObject()->SetActive(false);
    } else {
        hintText->set_text(muted ? "Others cannot hear you" : "Others can hear you");
        muteButton->get_gameObject()->SetActive(true);
        BSML::Lite::SetButtonText(muteButton, muted ? "Unmute" : "Mute");
    }
}

}  // namespace VoiceChat::UI
