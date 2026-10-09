using VoiceChat.UI;
using VoiceChat.Voice;
using Zenject;

namespace VoiceChat.Installers
{
    internal class VoiceMenuInstaller : Installer
    {
        public override void InstallBindings()
        {
            Container.BindInterfacesAndSelfTo<VoiceChatManager>().AsSingle();
            Container.BindInterfacesAndSelfTo<VoicePanel>().AsSingle();
            Container.BindInterfacesAndSelfTo<VoiceSettings>().AsSingle();
        }
    }
}
