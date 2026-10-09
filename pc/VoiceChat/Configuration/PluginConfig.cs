using System.Collections.Generic;
using System.Runtime.CompilerServices;
using IPA.Config.Stores;

[assembly: InternalsVisibleTo(GeneratedStore.AssemblyVisibilityTarget)]

namespace VoiceChat.Configuration
{
    public class PluginConfig
    {
        public static PluginConfig Instance { get; set; }

        public static readonly List<object> PushToTalkButtons = new List<object>
        {
            "Left grip", "Right grip", "X", "Y", "A", "B", "Left stick click", "Right stick click"
        };

        public virtual bool Enabled { get; set; } = true;
        public virtual bool StartMuted { get; set; } = true;
        public virtual bool PushToTalk { get; set; } = false;
        public virtual string PushToTalkButton { get; set; } = "Left grip";

        public virtual void Changed()
        {
        }
    }
}
