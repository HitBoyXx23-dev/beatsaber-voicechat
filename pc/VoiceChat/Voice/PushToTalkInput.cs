using UnityEngine.XR;

namespace VoiceChat.Voice
{
    internal static class PushToTalkInput
    {
        public static bool IsHeld(string buttonName)
        {
            switch (buttonName)
            {
                case "Right grip": return Read(XRNode.RightHand, CommonUsages.gripButton);
                case "X": return Read(XRNode.LeftHand, CommonUsages.primaryButton);
                case "Y": return Read(XRNode.LeftHand, CommonUsages.secondaryButton);
                case "A": return Read(XRNode.RightHand, CommonUsages.primaryButton);
                case "B": return Read(XRNode.RightHand, CommonUsages.secondaryButton);
                case "Left stick click": return Read(XRNode.LeftHand, CommonUsages.primary2DAxisClick);
                case "Right stick click": return Read(XRNode.RightHand, CommonUsages.primary2DAxisClick);
                default: return Read(XRNode.LeftHand, CommonUsages.gripButton);
            }
        }

        private static bool Read(XRNode node, InputFeatureUsage<bool> usage)
        {
            var device = InputDevices.GetDeviceAtXRNode(node);
            return device.isValid && device.TryGetFeatureValue(usage, out bool pressed) && pressed;
        }
    }
}
