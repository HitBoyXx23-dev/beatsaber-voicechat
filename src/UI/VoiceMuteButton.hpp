#pragma once

#include "bsml/shared/BSML/FloatingScreen/FloatingScreen.hpp"

namespace VoiceChat::UI {

class VoiceMuteButton {
public:
    static void Show();
    static void Hide();
    static void Refresh();
};

}  // namespace VoiceChat::UI
