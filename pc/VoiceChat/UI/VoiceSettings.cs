using System;
using System.Collections.Generic;
using BeatSaberMarkupLanguage.Attributes;
using BeatSaberMarkupLanguage.Settings;
using VoiceChat.Configuration;
using VoiceChat.Voice;
using Zenject;

namespace VoiceChat.UI
{
    internal class VoiceSettings : IInitializable, IDisposable
    {
        private const string MenuName = "Voice Chat";
        private const string ResourcePath = "VoiceChat.UI.VoiceSettings.bsml";

        private readonly VoiceChatManager _voiceChatManager;
        private readonly BSMLSettings _bsmlSettings;

        public VoiceSettings(VoiceChatManager voiceChatManager, BSMLSettings bsmlSettings)
        {
            _voiceChatManager = voiceChatManager;
            _bsmlSettings = bsmlSettings;
        }

        [UIValue("Enabled")]
        public bool Enabled
        {
            get => PluginConfig.Instance.Enabled;
            set { PluginConfig.Instance.Enabled = value; Apply(); }
        }

        [UIValue("StartMuted")]
        public bool StartMuted
        {
            get => PluginConfig.Instance.StartMuted;
            set { PluginConfig.Instance.StartMuted = value; Apply(); }
        }

        [UIValue("PushToTalk")]
        public bool PushToTalk
        {
            get => PluginConfig.Instance.PushToTalk;
            set { PluginConfig.Instance.PushToTalk = value; Apply(); }
        }

        [UIValue("PushToTalkButton")]
        public string PushToTalkButton
        {
            get => PluginConfig.Instance.PushToTalkButton;
            set { PluginConfig.Instance.PushToTalkButton = value; Apply(); }
        }

        [UIValue("PushToTalkButtons")]
        public List<object> PushToTalkButtons => PluginConfig.PushToTalkButtons;

        public void Initialize()
        {
            _bsmlSettings.AddSettingsMenu(MenuName, ResourcePath, this);
        }

        public void Dispose()
        {
            _bsmlSettings.RemoveSettingsMenu(this);
        }

        private void Apply()
        {
            PluginConfig.Instance.Changed();
            _voiceChatManager.ApplySettings();
        }
    }
}
