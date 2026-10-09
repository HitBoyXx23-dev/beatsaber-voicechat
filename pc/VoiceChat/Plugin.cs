using IPA;
using IPA.Config.Stores;
using SiraUtil.Zenject;
using VoiceChat.Configuration;
using VoiceChat.Installers;
using IPAConfig = IPA.Config.Config;
using IPALogger = IPA.Logging.Logger;

namespace VoiceChat
{
    [Plugin(RuntimeOptions.SingleStartInit)]
    public class Plugin
    {
        internal static IPALogger Log { get; private set; }

        [Init]
        public Plugin(IPALogger logger, IPAConfig config, Zenjector zenjector)
        {
            Log = logger;
            PluginConfig.Instance = config.Generated<PluginConfig>();
            zenjector.UseLogger(logger);
            zenjector.Install<VoiceMenuInstaller>(Location.Menu);
            Log.Info("Voice chat initialized");
        }
    }
}
