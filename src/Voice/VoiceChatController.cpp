#include "Voice/VoiceChatController.hpp"
#include "Voice/VoicePacket.hpp"
#include "logging.hpp"
#include "UI/VoiceMuteButton.hpp"

#include <algorithm>
#include <string_view>

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"

namespace VoiceChat {

namespace {

template <class TOut, class... TArgs>
auto RunStatic(std::string_view namespaze, std::string_view klassName, std::string_view methodName, TArgs&&... args) {
    auto* klass = il2cpp_utils::GetClassFromName(namespaze, klassName);
    if (!klass) return il2cpp_utils::MethodResult<TOut>(il2cpp_utils::RunMethodException("Class not found", nullptr));
    return il2cpp_utils::RunMethod<TOut>(klass, methodName, std::forward<TArgs>(args)...);
}

}  // namespace

VoiceChatController* VoiceChatController::instance_ = nullptr;

VoiceChatController* VoiceChatController::get_instance() {
    if (!instance_) instance_ = new VoiceChatController();
    return instance_;
}

void VoiceChatController::Arm(MultiplayerCore::Networking::MpPacketSerializer* serializer) {
    if (!serializer || armed_) return;

    serializer_ = serializer;
    armed_ = true;
    serializer_->RegisterCallback<Packets::VoicePacket*>(
        std::bind(&VoiceChatController::OnVoicePacket, this, std::placeholders::_1, std::placeholders::_2));
    INFO("Voice chat armed (serializer ready)");
}

void VoiceChatController::Disarm() {
    StopRuntime();
    if (!armed_) return;

    if (serializer_) serializer_->UnregisterCallback<Packets::VoicePacket*>();
    serializer_ = nullptr;
    armed_ = false;

    std::lock_guard lock(mutex_);
    buffers_.clear();
    pending_.clear();
    sendQueue_.clear();
    playbackQueue_.clear();
    outIndex_ = 0;
    INFO("Voice chat disarmed");
}

void VoiceChatController::StartRuntime() {
    if (!armed_ || runtimeActive_) return;

    runtimeActive_ = true;
    uiDelayFrames_ = 90;
    uiShown_ = false;
    micStarted_ = false;
    {
        std::lock_guard lock(mutex_);
        mic_.SetMuted(true);
    }
    EnsureSpeaker();
    INFO("Voice chat runtime started (lobby)");
}

void VoiceChatController::StopRuntime() {
    if (!runtimeActive_) return;

    runtimeActive_ = false;
    uiDelayFrames_ = 0;
    uiShown_ = false;
    UI::VoiceMuteButton::Hide();
    StopMic();
    micStarted_ = false;

    if (speaker_) {
        UnityEngine::Object::Destroy(speaker_->get_gameObject());
        speaker_ = nullptr;
    }

    std::lock_guard lock(mutex_);
    pending_.clear();
    sendQueue_.clear();
    playbackQueue_.clear();
    INFO("Voice chat runtime stopped");
}

void VoiceChatController::MainThreadTick() {
    if (!runtimeActive_) return;
    TryShowUi();
    PollMicCapture();
    FlushSendQueue();
    FlushPlaybackQueue();
}

void VoiceChatController::TryShowUi() {
    if (uiShown_ || uiDelayFrames_ <= 0) return;
    uiDelayFrames_--;
    if (uiDelayFrames_ > 0) return;
    UI::VoiceMuteButton::Show();
    uiShown_ = true;
}

void VoiceChatController::ToggleMute() {
    std::lock_guard lock(mutex_);
    mic_.Toggle();
    if (!mic_.IsMuted() && !micStarted_) {
        StartMic();
        micStarted_ = true;
    }
    INFO("Mic muted: {}", mic_.IsMuted());
}

bool VoiceChatController::IsMuted() {
    std::lock_guard lock(mutex_);
    return mic_.IsMuted();
}

void VoiceChatController::EnsureSpeaker() {
    if (speaker_) return;
    auto go = UnityEngine::GameObject::New_ctor(StringW("VoiceChatSpeaker"));
    if (!go) return;
    speaker_ = go->AddComponent<UnityEngine::AudioSource*>();
}

void VoiceChatController::StartMic() {
    if (micClip_) return;
    auto micStart = RunStatic<UnityEngine::AudioClip*>(
        "UnityEngine", "Microphone", "Start", StringW(), true, 1, kSampleRate);
    micClip_ = micStart.has_result() ? micStart.get_result() : nullptr;
    if (!micClip_) {
        WARNING("No microphone available; voice capture disabled");
        return;
    }
    micReadPos_ = 0;
}

void VoiceChatController::StopMic() {
    if (!micClip_) return;
    RunStatic<void>("UnityEngine", "Microphone", "End", StringW());
    micClip_ = nullptr;
}

void VoiceChatController::PollMicCapture() {
    if (!micClip_ || !runtimeActive_) return;

    auto posResult = RunStatic<int>("UnityEngine", "Microphone", "GetPosition", StringW());
    auto clipLenResult = il2cpp_utils::RunMethod<int>(micClip_, "get_samples");
    if (!posResult.has_result() || !clipLenResult.has_result()) return;

    int pos = posResult.get_result();
    int clipLen = clipLenResult.get_result();
    if (clipLen <= 0) return;

    int available = (pos - micReadPos_ + clipLen) % clipLen;
    if (available < kChunkSamples) return;

    ArrayW<float> floats(available);
    il2cpp_utils::RunMethod(micClip_, "GetData", floats, micReadPos_);
    micReadPos_ = (micReadPos_ + available) % clipLen;

    {
        std::lock_guard lock(mutex_);
        if (!mic_.ShouldTransmit()) {
            pending_.clear();
            return;
        }
        for (int i = 0; i < available; i++) {
            float s = std::clamp(floats[i], -1.0f, 1.0f);
            auto v = static_cast<int16_t>(s * 32767.0f);
            pending_.push_back(static_cast<uint8_t>(v & 0xFF));
            pending_.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
        }
        while (pending_.size() >= kMaxPcmBytesPerPacket) {
            sendQueue_.emplace_back(pending_.begin(), pending_.begin() + kMaxPcmBytesPerPacket);
            pending_.erase(pending_.begin(), pending_.begin() + kMaxPcmBytesPerPacket);
        }
    }
}

void VoiceChatController::FlushSendQueue() {
    if (!serializer_ || !runtimeActive_) return;

    std::deque<std::vector<uint8_t>> local;
    {
        std::lock_guard lock(mutex_);
        local.swap(sendQueue_);
    }

    for (auto& pcm : local) SendChunk(pcm);
}

void VoiceChatController::SendChunk(const std::vector<uint8_t>& pcm) {
    if (!serializer_ || !runtimeActive_ || pcm.empty()) return;

    auto packet = Packets::VoicePacket::Create();
    if (!packet) return;

    ArrayW<uint8_t> data(pcm.size());
    for (size_t i = 0; i < pcm.size(); i++) data[i] = pcm[i];

    {
        std::lock_guard lock(mutex_);
        packet->index = outIndex_++;
    }
    packet->data = data;
    serializer_->SendUnreliable(packet);
}

void VoiceChatController::OnVoicePacket(Packets::VoicePacket* packet, GlobalNamespace::IConnectedPlayer* player) {
    if (!packet || !player || !armed_) return;
    if (!packet->data) return;

    size_t len = static_cast<size_t>(packet->data.size());
    if (len == 0 || len > kMaxPcmBytesPerPacket) return;

    std::vector<std::vector<uint8_t>> ready;
    {
        std::lock_guard lock(mutex_);
        auto& jb = buffers_[player];
        jb.Push(packet->index, std::vector<uint8_t>(packet->data.begin(), packet->data.end()));
        while (auto chunk = jb.Pop()) ready.push_back(std::move(*chunk));
    }
    if (!runtimeActive_) return;
    {
        std::lock_guard lock(mutex_);
        for (auto& chunk : ready) playbackQueue_.push_back(std::move(chunk));
    }
}

void VoiceChatController::FlushPlaybackQueue() {
    if (!speaker_ || !runtimeActive_) return;

    std::deque<std::vector<uint8_t>> local;
    {
        std::lock_guard lock(mutex_);
        local.swap(playbackQueue_);
    }
    for (auto& chunk : local) PlayPcm(chunk);
}

void VoiceChatController::PlayPcm(const std::vector<uint8_t>& pcm) {
    if (!speaker_ || pcm.size() < 2) return;
    int samples = static_cast<int>(pcm.size() / 2);
    ArrayW<float> floats(samples);
    for (int i = 0; i < samples; i++) {
        auto v = static_cast<int16_t>(pcm[i * 2] | (pcm[i * 2 + 1] << 8));
        floats[i] = v / 32768.0f;
    }
    auto clipResult = RunStatic<UnityEngine::AudioClip*>(
        "UnityEngine", "AudioClip", "Create", StringW("voice"), samples, 1, kSampleRate, false);
    auto clip = clipResult.has_result() ? clipResult.get_result() : nullptr;
    if (!clip) return;
    il2cpp_utils::RunMethod(clip, "SetData", floats, 0);
    il2cpp_utils::RunMethod(speaker_, "PlayOneShot", clip);
}

}  // namespace VoiceChat
