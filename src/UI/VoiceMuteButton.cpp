#include "UI/VoiceMuteButton.hpp"
#include "Voice/VoiceChatController.hpp"
#include "logging.hpp"

#include "bsml/shared/BSML.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/UI/Button.hpp"

namespace {
BSML::FloatingScreen* screen = nullptr;
UnityEngine::UI::Button* button = nullptr;
}

namespace VoiceChat::UI {

void VoiceMuteButton::Show() {
    if (screen) return;
    screen = BSML::FloatingScreen::CreateFloatingScreen({20, 10}, false, {-1.5f, 1.2f, 3.0f}, UnityEngine::Quaternion::get_identity());

    // BSML::Lite::CreateUIButton(parent, text, anchor, size, onClick). Check this signature
    // against your bsml version; the call is the only place that depends on it.
    auto text = [] { return VoiceChat::VoiceChatController::get_instance()->IsMuted() ? "Unmute mic" : "Mute mic"; };
    button = BSML::Lite::CreateUIButton(screen->get_transform(), text(), {0, 0}, {18, 8}, [] {
        VoiceChat::VoiceChatController::get_instance()->ToggleMute();
        INFO("Mute toggled");
    });
}

void VoiceMuteButton::Hide() {
    if (screen) {
        UnityEngine::Object::Destroy(screen->get_gameObject());
        screen = nullptr;
        button = nullptr;
    }
}

}  // namespace VoiceChat::UI
