using System;
using System.Text;
using Valve.VR;

namespace leap_control
{
    class VRManager
    {
        bool m_initialized = false;

        readonly Core m_core = null;

        CVRSystem m_vrSystem;
        VREvent_t m_vrEvent;
        readonly uint m_eventSize = 0;
        bool m_active = false;

        ulong m_notificationOverlay = OpenVR.k_ulOverlayHandleInvalid;
        uint m_leapDevice = OpenVR.k_unTrackedDeviceIndexInvalid;
        uint m_leftHandController = OpenVR.k_unTrackedDeviceIndexInvalid;
        uint m_rightHandController = OpenVR.k_unTrackedDeviceIndexInvalid;

        readonly TrackedDevicePose_t[] m_trackedPoses = null;

        public VRManager(Core p_core)
        {
            m_core = p_core;

            m_trackedPoses = new TrackedDevicePose_t[OpenVR.k_unMaxTrackedDeviceCount];
            m_eventSize = (uint)System.Runtime.InteropServices.Marshal.SizeOf(typeof(VREvent_t));
        }

        public bool Initialize()
        {
            if(!m_initialized)
            {
                EVRInitError l_initError = EVRInitError.None;
                m_vrSystem = OpenVR.Init(ref l_initError, EVRApplicationType.VRApplication_Overlay);
                if(l_initError == EVRInitError.None)
                {
                    OpenVR.Overlay.CreateOverlay("leap.control.notification", "Ultraleap Control", ref m_notificationOverlay);

                    // Find fake Leap Motion station device
                    for(uint i = 0; i < OpenVR.k_unMaxTrackedDeviceCount; i++)
                    {
                        ETrackedPropertyError l_propertyError = ETrackedPropertyError.TrackedProp_Success;
                        ulong l_property = m_vrSystem.GetUint64TrackedDeviceProperty(i, ETrackedDeviceProperty.Prop_VendorSpecific_Reserved_Start, ref l_propertyError);
                        if((l_propertyError == ETrackedPropertyError.TrackedProp_Success) && (l_property == 0x4C4D6F74696F6E))
                        {
                            m_leapDevice = i;
                            break;
                        }
                    }

                    m_initialized = true;
                    m_active = true;
                }
                else
                    System.Windows.Forms.MessageBox.Show("Unable to initialize OpenVR: " + Valve.VR.OpenVR.GetStringForHmdError(l_initError), "Driver Leap Control");

            }

            return m_initialized;
        }

        public void Terminate()
        {
            if(m_initialized)
            {

                m_initialized = false;
            }
        }

        public bool DoPulse()
        {
            m_vrSystem.GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin.TrackingUniverseRawAndUncalibrated, 0f, m_trackedPoses);

            while(m_vrSystem.PollNextEvent(ref m_vrEvent, m_eventSize))
            {
                switch(m_vrEvent.eventType)
                {
                    case (uint)EVREventType.VREvent_Quit:
                    case (uint)EVREventType.VREvent_RestartRequested:
                    case (uint)EVREventType.VREvent_ProcessQuit:
                        m_active = false;
                        break;
                    case (uint)EVREventType.VREvent_TrackedDeviceDeactivated:
                    {
                        if(m_leftHandController == m_vrEvent.trackedDeviceIndex)
                            m_leftHandController = OpenVR.k_unTrackedDeviceIndexInvalid;
                        if(m_rightHandController == m_vrEvent.trackedDeviceIndex)
                            m_rightHandController = OpenVR.k_unTrackedDeviceIndexInvalid;
                    }
                    break;
                }
            }

            if(m_trackedPoses[OpenVR.k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
            {
                GlmSharp.mat4 l_matrix = GlmSharp.mat4.Identity;
                m_trackedPoses[OpenVR.k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking.Convert(ref l_matrix);
                m_core.GetControlManager().SetHeadTransform(l_matrix);
            }

            if(m_leftHandController != OpenVR.k_unTrackedDeviceIndexInvalid)
            {
                GlmSharp.mat4 l_matrix = GlmSharp.mat4.Identity;
                m_trackedPoses[m_leftHandController].mDeviceToAbsoluteTracking.Convert(ref l_matrix);
                m_core.GetControlManager().SetHandTransform(ControlManager.Hand.Left, l_matrix);
            }
            else
                m_leftHandController = m_vrSystem.GetTrackedDeviceIndexForControllerRole(ETrackedControllerRole.LeftHand);

            if(m_rightHandController != OpenVR.k_unTrackedDeviceIndexInvalid)
            {
                GlmSharp.mat4 l_matrix = GlmSharp.mat4.Identity;
                m_trackedPoses[m_rightHandController].mDeviceToAbsoluteTracking.Convert(ref l_matrix);
                m_core.GetControlManager().SetHandTransform(ControlManager.Hand.Right, l_matrix);
            }
            else
                m_rightHandController = m_vrSystem.GetTrackedDeviceIndexForControllerRole(ETrackedControllerRole.RightHand);

            return m_active;
        }

        public void SendMessage(string p_message)
        {
            if(m_leapDevice != OpenVR.k_unTrackedDeviceIndexInvalid)
            {
                StringBuilder l_stringBuilder = new StringBuilder(32);
                OpenVR.Debug.DriverDebugRequest(m_leapDevice, p_message, l_stringBuilder, 32);
            }
        }

        public void ShowNotification(string p_message)
        {
            uint l_notification = 0;
            NotificationBitmap_t l_bitmap = new NotificationBitmap_t();
            l_bitmap.m_pImageData = (IntPtr)0;
            l_bitmap.m_nHeight = 0;
            l_bitmap.m_nWidth = 0;
            l_bitmap.m_nBytesPerPixel = 0;
            OpenVR.Notifications.CreateNotification(m_notificationOverlay, 500, EVRNotificationType.Transient, p_message, EVRNotificationStyle.None, ref l_bitmap, ref l_notification);
        }
    }
}
