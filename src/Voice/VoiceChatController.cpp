#include "Voice/VoiceChatController.hpp"
#include "Voice/VoicePacket.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

// Mic capture and playback go through il2cpp_utils::RunMethod because the
// UnityEngine.Microphone type is not in the bs-cordl headers. See TODOs below.

namespace VoiceChat {

void VoiceChatController::Attach(MultiplayerCore::Networking::MpPacketSerializer* serializer) {
    serializer_ = serializer;
    if (!serializer_) return;
    serializer_->RegisterCallback<Packets::VoicePacket*>(
        std::bind(&VoiceChatController::OnVoicePacket, this, std::placeholders::_1, std::placeholders::_2));
}

void VoiceChatController::Detach() {
    if (serializer_) serializer_->UnregisterCallback<Packets::VoicePacket*>();
    serializer_ = nullptr;
    buffers_.clear();
}

void VoiceChatController::ToggleMute() {
    mic_.Toggle();
}

void VoiceChatController::Tick() {
    if (!serializer_ || !mic_.ShouldTransmit()) return;
    // TODO(capture): read new samples from the mic AudioClip via il2cpp_utils::RunMethod
    // ("UnityEngine", "Microphone", "GetPosition"), convert float -> 16-bit PCM,
    // and pass them to SendChunk. Nothing is captured yet.
}

void VoiceChatController::SendChunk(const std::vector<uint8_t>& pcm) {
    if (!serializer_ || pcm.empty()) return;
    auto packet = MultiplayerCore::Networking::MpPacketSerializer::ObtainPacket<Packets::VoicePacket*>();
    if (!packet) return;

    auto encoded = voicechat::EncodeVoicePacket({outIndex_++, pcm});
    // The inner payload is the raw PCM. The index travels in the packet field.
    packet->index = outIndex_ - 1;
    ArrayW<uint8_t> data(pcm.size());
    for (size_t i = 0; i < pcm.size(); i++) data[i] = pcm[i];
    packet->data = data;

    serializer_->SendUnreliable(packet);
}

void VoiceChatController::OnVoicePacket(Packets::VoicePacket* packet, GlobalNamespace::IConnectedPlayer* player) {
    if (!packet || !player) return;

    auto& jb = buffers_[player->get_userId().hash()];  // TODO: confirm the per-player key API
    std::vector<uint8_t> bytes(packet->data.begin(), packet->data.end());
    jb.Push(packet->index, std::move(bytes));

    while (auto chunk = jb.Pop()) {
        // TODO(playback): convert the 16-bit PCM in *chunk to floats, build an AudioClip with
        // AudioClip.Create + SetData, and play it on an AudioSource for this player.
        (void)chunk;
    }
}

}  // namespace VoiceChat
