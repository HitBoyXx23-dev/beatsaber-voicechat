#include "UI/VoiceMuteButton.hpp"
#include "Config/VoiceConfig.hpp"
#include "Voice/VoiceChatController.hpp"
#include "logging.hpp"

#include <string>

#include "bsml/shared/BSML.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/TextAnchor.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "UnityEngine/UI/HorizontalLayoutGroup.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"

namespace {

BSML::FloatingScreen* screen = nullptr;
UnityEngine::UI::Button* muteButton = nullptr;

const UnityEngine::Vector2 kPanelSize{32.0f, 11.0f};
const UnityEngine::Vector2 kButtonSize{30.0f, 9.0f};
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

    auto layout = BSML::Lite::CreateHorizontalLayoutGroup(screen->get_transform());
    layout->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);
    layout->set_childControlWidth(true);
    layout->set_childControlHeight(true);
    layout->set_childForceExpandWidth(false);
    layout->set_childForceExpandHeight(false);
    if (auto fitter = layout->GetComponent<UnityEngine::UI::ContentSizeFitter*>()) {
        UnityEngine::Object::Destroy(fitter);
    }
    auto layoutRect = layout->get_transform().cast<UnityEngine::RectTransform>();
    layoutRect->set_anchorMin({0.0f, 0.0f});
    layoutRect->set_anchorMax({1.0f, 1.0f});
    layoutRect->set_anchoredPosition({0.0f, 0.0f});
    layoutRect->set_sizeDelta({0.0f, 0.0f});

    muteButton = BSML::Lite::CreateUIButton(layout->get_transform(), "", DEFAULT_BUTTONTEMPLATE, {0, 0}, kButtonSize, OnMuteClicked);
    BSML::Lite::SetButtonTextSize(muteButton, 3.5f);
    BSML::Lite::ToggleButtonWordWrapping(muteButton, false);
    if (auto buttonFitter = muteButton->GetComponent<UnityEngine::UI::ContentSizeFitter*>()) {
        UnityEngine::Object::Destroy(buttonFitter);
    }
    auto layoutElement = muteButton->GetComponent<UnityEngine::UI::LayoutElement*>();
    if (!layoutElement) layoutElement = muteButton->get_gameObject()->AddComponent<UnityEngine::UI::LayoutElement*>();
    layoutElement->set_preferredWidth(kButtonSize.x);
    layoutElement->set_preferredHeight(kButtonSize.y);
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
