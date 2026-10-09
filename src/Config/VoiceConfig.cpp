#include "Config/VoiceConfig.hpp"
#include "logging.hpp"

#include <algorithm>
#include <exception>

#include "beatsaber-hook/shared/config/config-utils.hpp"

extern modloader::ModInfo modInfo;

namespace VoiceChat::Config {

namespace {

Settings settings;

Configuration& GetConfiguration() {
    static Configuration configuration(modInfo);
    return configuration;
}

void ReadBool(ConfigDocument const& doc, char const* key, bool& out) {
    auto it = doc.FindMember(key);
    if (it != doc.MemberEnd() && it->value.IsBool()) out = it->value.GetBool();
}

}  // namespace

Settings& Get() {
    return settings;
}

void Load() {
    try {
        auto& configuration = GetConfiguration();
        configuration.Load();
        auto const& doc = configuration.config;
        if (!doc.IsObject()) return;

        ReadBool(doc, "enabled", settings.enabled);
        ReadBool(doc, "startMuted", settings.startMuted);
        ReadBool(doc, "pushToTalk", settings.pushToTalk);

        auto it = doc.FindMember("pttButton");
        if (it != doc.MemberEnd() && it->value.IsInt()) {
            int maxIndex = static_cast<int>(PttButtonNames().size()) - 1;
            settings.pttButton = std::clamp(it->value.GetInt(), 0, maxIndex);
        }
    } catch (std::exception const& e) {
        ERROR("Failed to load voice chat settings, using defaults: {}", e.what());
    }
}

void Save() {
    try {
        auto& configuration = GetConfiguration();
        auto& doc = configuration.config;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();
        doc.AddMember("enabled", settings.enabled, allocator);
        doc.AddMember("startMuted", settings.startMuted, allocator);
        doc.AddMember("pushToTalk", settings.pushToTalk, allocator);
        doc.AddMember("pttButton", settings.pttButton, allocator);
        configuration.Write();
    } catch (std::exception const& e) {
        ERROR("Failed to save voice chat settings: {}", e.what());
    }
}

std::vector<std::string_view>& PttButtonNames() {
    static std::vector<std::string_view> names{
        "Left grip", "Right grip", "X", "Y", "A", "B", "Left stick click", "Right stick click"};
    return names;
}

std::string_view PttButtonName(int index) {
    auto& names = PttButtonNames();
    if (index < 0 || index >= static_cast<int>(names.size())) return names.front();
    return names[index];
}

int PttButtonIndex(std::string_view name) {
    auto& names = PttButtonNames();
    auto it = std::find(names.begin(), names.end(), name);
    return it == names.end() ? 0 : static_cast<int>(std::distance(names.begin(), it));
}

GlobalNamespace::OVRInput_RawButton PttRawButton() {
    using Button = GlobalNamespace::OVRInput_RawButton;
    switch (settings.pttButton) {
        case 1: return Button::RHandTrigger;
        case 2: return Button::X;
        case 3: return Button::Y;
        case 4: return Button::A;
        case 5: return Button::B;
        case 6: return Button::LThumbstick;
        case 7: return Button::RThumbstick;
        default: return Button::LHandTrigger;
    }
}

}  // namespace VoiceChat::Config
