#include "Voice/VoiceChatController.hpp"
#include "Voice/VoicePacket.hpp"
#include "logging.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "UnityEngine/GameObject.hpp"

// il2cpp exports used to let a plain std::thread call into managed code.
extern "C" void* il2cpp_domain_get();
extern "C" void* il2cpp_thread_attach(void* domain);
extern "C" void il2cpp_thread_detach(void* thread);

namespace VoiceChat {

VoiceChatController* VoiceChatController::instance_ = nullptr;

VoiceChatController* VoiceChatController::get_instance() {
    if (!instance_) instance_ = new VoiceChatController();
    return instance_;
}

void VoiceChatController::Attach(MultiplayerCore::Networking::MpPacketSerializer* serializer) {
    if (!serializer) return;
    serializer_ = serializer;
    serializer_->RegisterCallback<Packets::VoicePacket*>(
        std::bind(&VoiceChatController::OnVoicePacket, this, std::placeholders::_1, std::placeholders::_2));

    // Speaker object, created on the main thread.
    auto go = UnityEngine::GameObject::New_ctor(StringW("VoiceChatSpeaker"));
    speaker_ = go->AddComponent<UnityEngine::AudioSource*>();

    StartMic();
    INFO("Voice chat attached");
}

void VoiceChatController::Detach() {
    StopMic();
    if (serializer_) serializer_->UnregisterCallback<Packets::VoicePacket*>();
    serializer_ = nullptr;
    std::lock_guard lock(mutex_);
    buffers_.clear();
    pending_.clear();
    INFO("Voice chat detached");
}

void VoiceChatController::ToggleMute() {
    std::lock_guard lock(mutex_);
    mic_.Toggle();
    INFO("Mic muted: {}", mic_.IsMuted());
}

bool VoiceChatController::IsMuted() {
    std::lock_guard lock(mutex_);
    return mic_.IsMuted();
}

void VoiceChatController::StartMic() {
    // Microphone.Start(deviceName=null, loop=true, lengthSec=1, frequency)
    micClip_ = il2cpp_utils::RunMethod<UnityEngine::AudioClip*>(
        "UnityEngine", "Microphone", "Start", StringW(), true, 1, kSampleRate).value_or(nullptr);
    if (!micClip_) {
        ERROR("No microphone available");
        return;
    }
    micReadPos_ = 0;
    running_ = true;
    captureThread_ = std::thread(&VoiceChatController::CaptureLoop, this);
}

void VoiceChatController::StopMic() {
    running_ = false;
    if (captureThread_.joinable()) captureThread_.join();
    if (micClip_) {
        il2cpp_utils::RunMethod("UnityEngine", "Microphone", "End", StringW());
        micClip_ = nullptr;
    }
}

// Runs on its own thread. Reads new samples from the looping mic clip, converts
// them to 16-bit PCM, and sends them in fixed-size chunks.
void VoiceChatController::CaptureLoop() {
    void* thread = il2cpp_thread_attach(il2cpp_domain_get());
    while (running_) {
        auto pos = il2cpp_utils::RunMethod<int>("UnityEngine", "Microphone", "GetPosition", StringW());
        int clipLen = il2cpp_utils::RunMethod<int>(micClip_, "get_samples").value_or(0);
        if (!pos || clipLen <= 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        int available = (*pos - micReadPos_ + clipLen) % clipLen;
        if (available >= kChunkSamples) {
            ArrayW<float> floats(available);
            il2cpp_utils::RunMethod(micClip_, "GetData", floats, micReadPos_);
            micReadPos_ = (micReadPos_ + available) % clipLen;

            std::lock_guard lock(mutex_);
            if (mic_.ShouldTransmit()) {
                for (int i = 0; i < available; i++) {
                    float s = std::clamp(floats[i], -1.0f, 1.0f);
                    auto v = static_cast<int16_t>(s * 32767.0f);
                    pending_.push_back(static_cast<uint8_t>(v & 0xFF));
                    pending_.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
                }
                while (pending_.size() >= static_cast<size_t>(kChunkSamples) * 2) {
                    std::vector<uint8_t> chunk(pending_.begin(), pending_.begin() + kChunkSamples * 2);
                    pending_.erase(pending_.begin(), pending_.begin() + kChunkSamples * 2);
                    SendChunk(chunk);
                }
            } else {
                pending_.clear();  // muted: drop audio, do not send
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    il2cpp_thread_detach(thread);
}

// Caller must hold mutex_.
void VoiceChatController::SendChunk(const std::vector<uint8_t>& pcm) {
    if (!serializer_) return;
    auto packet = MultiplayerCore::Networking::MpPacketSerializer::ObtainPacket<Packets::VoicePacket*>();
    if (!packet) return;

    ArrayW<uint8_t> data(pcm.size());
    for (size_t i = 0; i < pcm.size(); i++) data[i] = pcm[i];
    packet->index = outIndex_++;
    packet->data = data;
    serializer_->SendUnreliable(packet);
}

void VoiceChatController::OnVoicePacket(Packets::VoicePacket* packet, GlobalNamespace::IConnectedPlayer* player) {
    if (!packet || !player) return;

    std::vector<std::vector<uint8_t>> ready;
    {
        std::lock_guard lock(mutex_);
        auto& jb = buffers_[player];
        jb.Push(packet->index, std::vector<uint8_t>(packet->data.begin(), packet->data.end()));
        while (auto chunk = jb.Pop()) ready.push_back(std::move(*chunk));
    }
    for (auto& chunk : ready) PlayPcm(chunk);
}

// Plays one chunk of 16-bit PCM through the speaker on the main thread's AudioSource.
void VoiceChatController::PlayPcm(const std::vector<uint8_t>& pcm) {
    if (!speaker_ || pcm.size() < 2) return;
    int samples = static_cast<int>(pcm.size() / 2);
    ArrayW<float> floats(samples);
    for (int i = 0; i < samples; i++) {
        auto v = static_cast<int16_t>(pcm[i * 2] | (pcm[i * 2 + 1] << 8));
        floats[i] = v / 32768.0f;
    }
    auto clip = il2cpp_utils::RunMethod<UnityEngine::AudioClip*>(
        "UnityEngine", "AudioClip", "Create", StringW("voice"), samples, 1, kSampleRate, false).value_or(nullptr);
    if (!clip) return;
    il2cpp_utils::RunMethod(clip, "SetData", floats, 0);
    il2cpp_utils::RunMethod(speaker_, "PlayOneShot", clip);
}

}  // namespace VoiceChat
