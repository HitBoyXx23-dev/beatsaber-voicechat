using System.Collections.Generic;
using UnityEngine;
using VoiceChat.Networking;

namespace VoiceChat.Voice
{
    internal class VoicePlayer
    {
        private const int MaxBufferedSamples = VoicePacket.SampleRate / 2;
        private const int StartThresholdSamples = VoicePacket.ChunkSamples * 3;

        private readonly Queue<float> _samples = new Queue<float>();
        private readonly object _lock = new object();
        private readonly GameObject _gameObject;
        private bool _buffering = true;

        public VoicePlayer(string playerId)
        {
            _gameObject = new GameObject($"VoiceChatPlayer_{playerId}");
            Object.DontDestroyOnLoad(_gameObject);

            var source = _gameObject.AddComponent<AudioSource>();
            source.spatialBlend = 0f;
            source.loop = true;
            source.clip = AudioClip.Create($"VoiceChat_{playerId}", VoicePacket.SampleRate, 1, VoicePacket.SampleRate, true, OnAudioRead);
            source.Play();
        }

        public void Enqueue(byte[] pcm)
        {
            if (pcm == null || pcm.Length < 2)
                return;

            lock (_lock)
            {
                for (int i = 0; i + 1 < pcm.Length; i += 2)
                {
                    short value = (short)(pcm[i] | (pcm[i + 1] << 8));
                    _samples.Enqueue(value / 32768f);
                }

                while (_samples.Count > MaxBufferedSamples)
                    _samples.Dequeue();
            }
        }

        public void Destroy()
        {
            if (_gameObject != null)
                Object.Destroy(_gameObject);
        }

        private void OnAudioRead(float[] data)
        {
            lock (_lock)
            {
                if (_buffering && _samples.Count >= StartThresholdSamples)
                    _buffering = false;

                for (int i = 0; i < data.Length; i++)
                {
                    if (_buffering || _samples.Count == 0)
                    {
                        data[i] = 0f;
                        continue;
                    }
                    data[i] = _samples.Dequeue();
                }

                if (_samples.Count == 0)
                    _buffering = true;
            }
        }
    }
}
