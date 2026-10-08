// Sends the local mic audio to other lobby players and plays theirs back.
// Ties together MicState (mute), voice_core (encoding) and JitterBuffer (ordering).
#pragma once

#include <map>
#include <vector>

#include "multiplayer-core/shared/Networking/MpPacketSerializer.hpp"
#include "GlobalNamespace/IConnectedPlayer.hpp"
#include "voice_core.hpp"
#include "jitter_buffer.hpp"

namespace VoiceChat {

class VoiceChatController {
public:
    // Called once the lobby's packet serializer exists.
    void Attach(MultiplayerCore::Networking::MpPacketSerializer* serializer);
    void Detach();

    // Called every frame (or on a timer) while in a lobby.
    void Tick();

    void ToggleMute();
    bool IsMuted() const { return mic_.IsMuted(); }

private:
    void OnVoicePacket(Packets::VoicePacket* packet, GlobalNamespace::IConnectedPlayer* player);
    void SendChunk(const std::vector<uint8_t>& pcm);

    MultiplayerCore::Networking::MpPacketSerializer* serializer_ = nullptr;
    voicechat::MicState mic_;
    int outIndex_ = 0;
    std::map<int, voicechat::JitterBuffer> buffers_;  // one per remote player id
};

}  // namespace VoiceChat
