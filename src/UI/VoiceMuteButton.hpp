#pragma once

#include "bsml/shared/BSML/FloatingScreen/FloatingScreen.hpp"

namespace VoiceChat::UI {
// A small floating button in the lobby that toggles the local mic mute.
class VoiceMuteButton {
public:
    static void Show();
    static void Hide();
};
}  // namespace VoiceChat::UI
