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
        private static readonly Vector2 PanelSize = new Vector2(32f, 11f);
        private static readonly Vector3 PanelPosition = new Vector3(-1.6f, 2.4f, 2.4f);
        private static readonly Vector3 PanelRotation = new Vector3(-12f, -34f, 0f);

        private readonly VoiceChatManager _voiceChatManager;
        private readonly BSMLParser _bsmlParser;
        private FloatingScreen _screen;

        public event PropertyChangedEventHandler PropertyChanged;

        public VoicePanel(VoiceChatManager voiceChatManager, BSMLParser bsmlParser)
        {
            _voiceChatManager = voiceChatManager;
            _bsmlParser = bsmlParser;
        }

        [UIValue("ButtonText")]
        public string ButtonText
        {
            get
            {
                string label = _voiceChatManager.IsMuted ? "<color=#FF5959>MIC OFF</color>" : "<color=#59FF73>MIC ON</color>";
                var config = PluginConfig.Instance;
                return config.PushToTalk ? $"{label} (hold {config.PushToTalkButton})" : label;
            }
        }

        [UIValue("ButtonInteractable")]
        public bool ButtonInteractable => !PluginConfig.Instance.PushToTalk;

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

            NotifyPropertyChanged(nameof(ButtonText));
            NotifyPropertyChanged(nameof(ButtonInteractable));
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
