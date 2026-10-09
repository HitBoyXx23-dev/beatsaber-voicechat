using LiteNetLib.Utils;
using MultiplayerCore.Networking.Abstractions;

namespace VoiceChat.Networking
{
    public class VoicePacket : MpPacket
    {
        public const int SampleRate = 16000;
        public const int ChunkSamples = 320;
        public const int MaxDataBytes = ChunkSamples * 2;

        public int Index;
        public byte[] Data;

        public override void Serialize(NetDataWriter writer)
        {
            int length = Data?.Length ?? 0;
            writer.Put(Index);
            writer.Put(length);
            for (int i = 0; i < length; i++)
                writer.Put(Data[i]);
        }

        public override void Deserialize(NetDataReader reader)
        {
            Index = reader.GetInt();
            int length = reader.GetInt();
            if (length <= 0 || length > MaxDataBytes)
            {
                Data = null;
                return;
            }

            Data = new byte[length];
            for (int i = 0; i < length; i++)
                Data[i] = reader.GetByte();
        }
    }
}
