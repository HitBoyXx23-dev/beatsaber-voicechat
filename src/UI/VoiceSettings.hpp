#pragma once

#include "HMUI/ViewController.hpp"

namespace VoiceChat::UI {

class VoiceSettings {
public:
    static void Register();

private:
    static void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
};

}  // namespace VoiceChat::UI
