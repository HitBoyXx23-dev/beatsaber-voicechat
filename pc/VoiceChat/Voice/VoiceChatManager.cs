using System;
using System.Collections.Generic;
using System.Linq;
using MultiplayerCore.Networking;
using UnityEngine;
using VoiceChat.Configuration;
using VoiceChat.Networking;
using Zenject;

namespace VoiceChat.Voice
{
    internal class VoiceChatManager : IInitializable, ITickable, IDisposable
    {
        private readonly MpPacketSerializer _packetSerializer;
        private readonly IMultiplayerSessionManager _sessionManager;
        private readonly MicrophoneCapture _microphone = new MicrophoneCapture();
        private readonly Dictionary<string, VoicePlayer> _players = new Dictionary<string, VoicePlayer>();

        private GameServerLobbyFlowCoordinator _lobbyFlowCoordinator;
        private int _outIndex;

        public event Action StateChanged;

        public bool IsActive { get; private set; }
        public bool IsMuted { get; private set; } = true;

        public VoiceChatManager(MpPacketSerializer packetSerializer, IMultiplayerSessionManager sessionManager)
        {
            _packetSerializer = packetSerializer;
            _sessionManager = sessionManager;
        }

        public void Initialize()
        {
            _packetSerializer.RegisterCallback<VoicePacket>(HandleVoicePacket);
            _sessionManager.playerDisconnectedEvent += HandlePlayerDisconnected;
        }

        public void Dispose()
        {
            StopRuntime();
            _packetSerializer.UnregisterCallback<VoicePacket>();
            _sessionManager.playerDisconnectedEvent -= HandlePlayerDisconnected;
        }

        public void Tick()
        {
            bool shouldBeActive = PluginConfig.Instance.Enabled && _sessionManager.isConnected && IsInLobby();
            if (shouldBeActive && !IsActive)
                StartRuntime();
            else if (!shouldBeActive && IsActive)
                StopRuntime();

            if (!IsActive)
                return;

            UpdatePushToTalk();
            _microphone.ReadChunks(!IsMuted, SendChunk);
        }

        public void ToggleMute()
        {
            if (!IsActive || PluginConfig.Instance.PushToTalk)
                return;

            SetMuted(!IsMuted);
        }

        public void ApplySettings()
        {
            if (!IsActive)
                return;

            if (PluginConfig.Instance.PushToTalk)
            {
                _microphone.Start();
                SetMuted(true);
            }
            StateChanged?.Invoke();
        }

        private void StartRuntime()
        {
            IsActive = true;
            var config = PluginConfig.Instance;
            IsMuted = config.PushToTalk || config.StartMuted;
            if (config.PushToTalk || !IsMuted)
                _microphone.Start();

            Plugin.Log.Info("Voice chat started (lobby)");
            StateChanged?.Invoke();
        }

        private void StopRuntime()
        {
            if (!IsActive)
                return;

            IsActive = false;
            _microphone.Stop();
            foreach (var player in _players.Values)
                player.Destroy();
            _players.Clear();

            Plugin.Log.Info("Voice chat stopped");
            StateChanged?.Invoke();
        }

        private void SetMuted(bool muted)
        {
            if (IsMuted == muted)
                return;

            IsMuted = muted;
            if (!muted)
                _microphone.Start();
            StateChanged?.Invoke();
        }

        private void UpdatePushToTalk()
        {
            var config = PluginConfig.Instance;
            if (!config.PushToTalk)
                return;

            SetMuted(!PushToTalkInput.IsHeld(config.PushToTalkButton));
        }

        private void SendChunk(byte[] pcm)
        {
            if (!_sessionManager.isConnected)
                return;

            var packet = new VoicePacket { Index = _outIndex++, Data = pcm };
            _sessionManager.SendUnreliable(packet);
        }

        private void HandleVoicePacket(VoicePacket packet, IConnectedPlayer player)
        {
            if (!IsActive || packet?.Data == null || player == null || player.isMe)
                return;

            if (!_players.TryGetValue(player.userId, out var voicePlayer))
            {
                voicePlayer = new VoicePlayer(player.userId);
                _players[player.userId] = voicePlayer;
            }
            voicePlayer.Enqueue(packet.Data);
        }

        private void HandlePlayerDisconnected(IConnectedPlayer player)
        {
            if (player == null || !_players.TryGetValue(player.userId, out var voicePlayer))
                return;

            voicePlayer.Destroy();
            _players.Remove(player.userId);
        }

        private bool IsInLobby()
        {
            if (_lobbyFlowCoordinator == null)
                _lobbyFlowCoordinator = Resources.FindObjectsOfTypeAll<GameServerLobbyFlowCoordinator>().FirstOrDefault();
            return _lobbyFlowCoordinator != null && _lobbyFlowCoordinator.isActivated;
        }
    }
}
