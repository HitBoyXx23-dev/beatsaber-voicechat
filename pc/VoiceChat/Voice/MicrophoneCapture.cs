using System;
using System.Collections.Generic;
using UnityEngine;
using VoiceChat.Networking;

namespace VoiceChat.Voice
{
    internal class MicrophoneCapture
    {
        private const int ClipLengthSeconds = 1;

        private readonly List<float> _resampled = new List<float>();
        private AudioClip _clip;
        private string _device;
        private int _deviceRate;
        private int _readPosition;
        private float[] _readBuffer = new float[0];
        private double _resamplePosition;

        public bool IsRunning => _clip != null;

        public bool Start()
        {
            if (_clip != null)
                return true;

            if (Microphone.devices.Length == 0)
            {
                Plugin.Log.Warn("No microphone found; voice capture disabled");
                return false;
            }

            _device = Microphone.devices[0];
            _deviceRate = PickDeviceRate(_device);
            _clip = Microphone.Start(_device, true, ClipLengthSeconds, _deviceRate);
            if (_clip == null)
            {
                Plugin.Log.Warn($"Could not start microphone '{_device}'");
                return false;
            }

            _readPosition = 0;
            _resamplePosition = 0;
            _resampled.Clear();
            Plugin.Log.Info($"Microphone '{_device}' started at {_deviceRate} Hz");
            return true;
        }

        public void Stop()
        {
            if (_clip == null)
                return;

            Microphone.End(_device);
            _clip = null;
            _resampled.Clear();
        }

        public void ReadChunks(bool transmit, Action<byte[]> onChunk)
        {
            if (_clip == null)
                return;

            int position = Microphone.GetPosition(_device);
            int clipSamples = _clip.samples;
            int available = (position - _readPosition + clipSamples) % clipSamples;
            if (available <= 0)
                return;

            if (_readBuffer.Length != available)
                _readBuffer = new float[available];

            _clip.GetData(_readBuffer, _readPosition);
            _readPosition = (_readPosition + available) % clipSamples;

            if (!transmit)
            {
                _resampled.Clear();
                return;
            }

            Resample(_readBuffer, available);

            while (_resampled.Count >= VoicePacket.ChunkSamples)
            {
                onChunk(EncodePcm16(_resampled, VoicePacket.ChunkSamples));
                _resampled.RemoveRange(0, VoicePacket.ChunkSamples);
            }
        }

        private void Resample(float[] input, int count)
        {
            if (_deviceRate == VoicePacket.SampleRate)
            {
                for (int i = 0; i < count; i++)
                    _resampled.Add(input[i]);
                return;
            }

            double step = (double)_deviceRate / VoicePacket.SampleRate;
            while (_resamplePosition < count - 1)
            {
                int index = (int)_resamplePosition;
                float fraction = (float)(_resamplePosition - index);
                _resampled.Add(Mathf.Lerp(input[index], input[index + 1], fraction));
                _resamplePosition += step;
            }
            _resamplePosition -= count;
            if (_resamplePosition < 0)
                _resamplePosition = 0;
        }

        private static byte[] EncodePcm16(List<float> samples, int count)
        {
            var bytes = new byte[count * 2];
            for (int i = 0; i < count; i++)
            {
                short value = (short)(Mathf.Clamp(samples[i], -1f, 1f) * short.MaxValue);
                bytes[i * 2] = (byte)(value & 0xFF);
                bytes[i * 2 + 1] = (byte)((value >> 8) & 0xFF);
            }
            return bytes;
        }

        private static int PickDeviceRate(string device)
        {
            Microphone.GetDeviceCaps(device, out int minRate, out int maxRate);
            if (minRate == 0 && maxRate == 0)
                return VoicePacket.SampleRate;
            if (VoicePacket.SampleRate >= minRate && VoicePacket.SampleRate <= maxRate)
                return VoicePacket.SampleRate;
            return maxRate;
        }
    }
}
