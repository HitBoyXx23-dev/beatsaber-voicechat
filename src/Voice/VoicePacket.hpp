// Voice packet carried over MultiplayerCore's packet serializer.
// Wraps the platform-independent encoding in src/voice_core (index + payload).
#pragma once

#include "custom-types/shared/macros.hpp"
#include "multiplayer-core/shared/Networking/Abstractions/MpPacket.hpp"
#include "beatsaber-hook/shared/utils/typedefs-array.hpp"

DECLARE_CLASS_CUSTOM(VoiceChat::Packets, VoicePacket, MultiplayerCore::Networking::Abstractions::MpPacket) {
    DECLARE_INSTANCE_FIELD(int, index);
    DECLARE_INSTANCE_FIELD(ArrayW<uint8_t>, data);

    DECLARE_OVERRIDE_METHOD_MATCH(void, Serialize, &LiteNetLib::Utils::INetSerializable::Serialize, LiteNetLib::Utils::NetDataWriter* writer);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Deserialize, &LiteNetLib::Utils::INetSerializable::Deserialize, LiteNetLib::Utils::NetDataReader* reader);

    DECLARE_DEFAULT_CTOR();
};
