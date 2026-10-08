#include "Voice/VoicePacket.hpp"

#include "LiteNetLib/Utils/NetDataWriter.hpp"
#include "LiteNetLib/Utils/NetDataReader.hpp"

DEFINE_TYPE(VoiceChat::Packets, VoicePacket);

void VoiceChat::Packets::VoicePacket::Serialize(LiteNetLib::Utils::NetDataWriter* writer) {
    writer->Put(index);
    writer->Put(static_cast<int>(data.size()));
    for (uint8_t b : data) writer->Put(b);
}

void VoiceChat::Packets::VoicePacket::Deserialize(LiteNetLib::Utils::NetDataReader* reader) {
    index = reader->GetInt();
    int length = reader->GetInt();
    constexpr int kMaxVoiceBytes = 640;
    if (length <= 0 || length > kMaxVoiceBytes) {
        data = nullptr;
        return;
    }
    auto bytes = ArrayW<uint8_t>(length);
    for (int i = 0; i < length; i++) bytes[i] = reader->GetByte();
    data = bytes;
}

