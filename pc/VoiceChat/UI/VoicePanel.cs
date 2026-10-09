using System;
using System.ComponentModel;
using System.Reflection;
using System.Runtime.CompilerServices;
using BeatSaberMarkupLanguage;
using BeatSaberMarkupLanguage.Attributes;
using BeatSaberMarkupLanguage.FloatingScreen;
using UnityEngine;
using VoiceChat.Configuration;
using VoiceChat.Voice;
using Zenject;

namespace VoiceChat.UI
{
    internal class VoicePanel : IInitializable, IDisposable, INotifyPropertyChanged
    {
        private const string ResourcePath = "VoiceChat.UI.VoicePanel.bsml";
        private static readonly Vector2 PanelSize = new Vector2(44f, 26f);
        private static readonly Vector3 PanelPosition = new Vector3(-1.0f, 0.85f, 1.9f);
        private static readonly Vector3 PanelRotation = new Vector3(30f, -25f, 0f);

        private readonly VoiceChatManager _voiceChatManager;
        private readonly BSMLParser _bsmlParser;
        private FloatingScreen _screen;

        public event PropertyChangedEventHandler PropertyChanged;

        public VoicePanel(VoiceChatManager voiceChatManager, BSMLParser bsmlParser)
        {
            _voiceChatManager = voiceChatManager;
            _bsmlParser = bsmlParser;
        }

        [UIValue("StatusText")]
        public string StatusText => _voiceChatManager.IsMuted ? "MIC MUTED" : "MIC LIVE";

        [UIValue("StatusColor")]
        public string StatusColor => _voiceChatManager.IsMuted ? "#FF5959" : "#59FF73";

        [UIValue("HintText")]
        public string HintText
        {
            get
            {
                var config = PluginConfig.Instance;
                if (config.PushToTalk)
                    return $"Hold {config.PushToTalkButton} to talk";
                return _voiceChatManager.IsMuted ? "Others cannot hear you" : "Others can hear you";
            }
        }

        [UIValue("ButtonText")]
        public string ButtonText => _voiceChatManager.IsMuted ? "Unmute" : "Mute";

        [UIValue("ShowButton")]
        public bool ShowButton => !PluginConfig.Instance.PushToTalk;

        public void Initialize()
        {
            _voiceChatManager.StateChanged += Refresh;
        }

        public void Dispose()
        {
            _voiceChatManager.StateChanged -= Refresh;
            DestroyScreen();
        }

        [UIAction("ToggleMute")]
        private void ToggleMute()
        {
            _voiceChatManager.ToggleMute();
        }

        private void Refresh()
        {
            if (!_voiceChatManager.IsActive)
            {
                DestroyScreen();
                return;
            }

            if (_screen == null)
                CreateScreen();

            NotifyPropertyChanged(nameof(StatusText));
            NotifyPropertyChanged(nameof(StatusColor));
            NotifyPropertyChanged(nameof(HintText));
            NotifyPropertyChanged(nameof(ButtonText));
            NotifyPropertyChanged(nameof(ShowButton));
        }

        private void CreateScreen()
        {
            _screen = FloatingScreen.CreateFloatingScreen(PanelSize, false, PanelPosition, Quaternion.Euler(PanelRotation));
            var content = Utilities.GetResourceContent(Assembly.GetExecutingAssembly(), ResourcePath);
            _bsmlParser.Parse(content, _screen.gameObject, this);
        }

        private void DestroyScreen()
        {
            if (_screen == null)
                return;

            UnityEngine.Object.Destroy(_screen.gameObject);
            _screen = null;
        }

        private void NotifyPropertyChanged([CallerMemberName] string propertyName = "")
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
