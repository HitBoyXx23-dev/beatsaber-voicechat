// Sends the local mic audio to other lobby players and plays theirs back.
// Ties together MicState (mute), voice_core (encoding) and JitterBuffer (ordering).
#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "multiplayer-core/shared/Networking/MpPacketSerializer.hpp"
#include "GlobalNamespace/IConnectedPlayer.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "voice_core.hpp"
#include "jitter_buffer.hpp"

namespace VoiceChat {

class VoiceChatController {
public:
    static constexpr int kSampleRate = 16000;
    static constexpr int kChunkSamples = 320;  // 20 ms at 16 kHz

    static VoiceChatController* get_instance();

    // Called from the lobby hook (main thread) with the injected serializer.
    void Attach(MultiplayerCore::Networking::MpPacketSerializer* serializer);
    void Detach();

    void ToggleMute();
    bool IsMuted();

private:
    void StartMic();
    void StopMic();
    void CaptureLoop();
    void SendChunk(const std::vector<uint8_t>& pcm);
    void OnVoicePacket(Packets::VoicePacket* packet, GlobalNamespace::IConnectedPlayer* player);
    void PlayPcm(const std::vector<uint8_t>& pcm);

    static VoiceChatController* instance_;

    MultiplayerCore::Networking::MpPacketSerializer* serializer_ = nullptr;
    voicechat::MicState mic_;
    std::mutex mutex_;  // guards mic_, buffers_, outIndex_, pending_
    int outIndex_ = 0;
    std::vector<uint8_t> pending_;  // PCM waiting to fill a whole chunk
    std::map<GlobalNamespace::IConnectedPlayer*, voicechat::JitterBuffer> buffers_;

    std::atomic<bool> running_{false};
    std::thread captureThread_;
    UnityEngine::AudioClip* micClip_ = nullptr;
    int micReadPos_ = 0;
    UnityEngine::AudioSource* speaker_ = nullptr;
};

}  // namespace VoiceChat
