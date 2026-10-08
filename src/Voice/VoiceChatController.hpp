#pragma once

#include <atomic>
#include <deque>
#include <map>
#include <mutex>
#include <vector>

#include "Voice/VoicePacket.hpp"
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
    static constexpr int kChunkSamples = 320;
    static constexpr size_t kMaxPcmBytesPerPacket = kChunkSamples * 2;

    static VoiceChatController* get_instance();

    void Arm(MultiplayerCore::Networking::MpPacketSerializer* serializer);
    void Disarm();

    void StartRuntime();
    void StopRuntime();

    bool IsArmed() const { return armed_; }
    bool IsRuntimeActive() const { return runtimeActive_; }
    void MainThreadTick();

    void ToggleMute();
    bool IsMuted();

private:
    void EnsureSpeaker();
    void StartMic();
    void StopMic();
    void PollMicCapture();
    void SendChunk(const std::vector<uint8_t>& pcm);
    void OnVoicePacket(Packets::VoicePacket* packet, GlobalNamespace::IConnectedPlayer* player);
    void PlayPcm(const std::vector<uint8_t>& pcm);
    void FlushSendQueue();
    void FlushPlaybackQueue();
    void TryShowUi();

    static VoiceChatController* instance_;

    std::atomic<bool> armed_{false};
    std::atomic<bool> runtimeActive_{false};
    int uiDelayFrames_ = 0;
    bool uiShown_ = false;
    bool micStarted_ = false;

    MultiplayerCore::Networking::MpPacketSerializer* serializer_ = nullptr;
    voicechat::MicState mic_;
    std::mutex mutex_;
    int outIndex_ = 0;
    std::vector<uint8_t> pending_;
    std::map<GlobalNamespace::IConnectedPlayer*, voicechat::JitterBuffer> buffers_;
    std::deque<std::vector<uint8_t>> sendQueue_;
    std::deque<std::vector<uint8_t>> playbackQueue_;

    UnityEngine::AudioClip* micClip_ = nullptr;
    int micReadPos_ = 0;
    UnityEngine::AudioSource* speaker_ = nullptr;
};

}  // namespace VoiceChat
