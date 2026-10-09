#pragma once

#include <string_view>
#include <vector>

#include "GlobalNamespace/OVRInput.hpp"

namespace VoiceChat::Config {

struct Settings {
    bool enabled = true;
    bool startMuted = true;
    bool pushToTalk = false;
    int pttButton = 0;
};

Settings& Get();
void Load();
void Save();

std::vector<std::string_view>& PttButtonNames();
std::string_view PttButtonName(int index);
int PttButtonIndex(std::string_view name);
GlobalNamespace::OVRInput_RawButton PttRawButton();

}  // namespace VoiceChat::Config
