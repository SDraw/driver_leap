using System.Collections.Generic;

namespace leap_control
{
    class ControlManager
    {
        public enum Hand
        {
            Left = 0,
            Right
        };

        bool m_initialized = false;

        readonly Core m_core = null;

        HandOverlay m_leftHandOverlay = null;
        HandOverlay m_rightHandOverlay = null;

        ulong m_leftFingerOverlay = 0;
        ulong m_rightFingerOverlay = 0;

        GlmSharp.mat4 m_headTransform;

        public ControlManager(Core p_core)
        {
            m_core = p_core;
        }

        public bool Initialize()
        {
            if(!m_initialized)
            {
                HandOverlay.LoadResources();

                m_leftHandOverlay = new HandOverlay(HandOverlay.Hand.Left);
                m_rightHandOverlay = new HandOverlay(HandOverlay.Hand.Right);

                Valve.VR.OpenVR.Overlay.CreateOverlay("leap.cursor.left", "Left Hand Finger Cursor", ref m_leftFingerOverlay);
                Valve.VR.OpenVR.Overlay.SetOverlayFromFile(m_leftFingerOverlay, System.AppDomain.CurrentDomain.BaseDirectory + "..\\..\\resources\\textures\\tx_cursor.png");
                Valve.VR.OpenVR.Overlay.SetOverlayColor(m_leftFingerOverlay, 0f, 1f, 0f);
                Valve.VR.OpenVR.Overlay.SetOverlayWidthInMeters(m_leftFingerOverlay, 0.006625f);
                Valve.VR.OpenVR.Overlay.SetOverlaySortOrder(m_leftFingerOverlay, 1);
                Valve.VR.OpenVR.Overlay.ShowOverlay(m_leftFingerOverlay);

                Valve.VR.OpenVR.Overlay.CreateOverlay("leap.cursor.right", "Left Hand Finger Cursor", ref m_rightFingerOverlay);
                Valve.VR.OpenVR.Overlay.SetOverlayFromFile(m_rightFingerOverlay, System.AppDomain.CurrentDomain.BaseDirectory + "..\\..\\resources\\textures\\tx_cursor.png");
                Valve.VR.OpenVR.Overlay.SetOverlayColor(m_rightFingerOverlay, 0f, 1f, 0f);
                Valve.VR.OpenVR.Overlay.SetOverlaySortOrder(m_rightFingerOverlay, 1);
                Valve.VR.OpenVR.Overlay.SetOverlayWidthInMeters(m_rightFingerOverlay, 0.00625f);
                Valve.VR.OpenVR.Overlay.ShowOverlay(m_rightFingerOverlay);

                m_initialized = true;
            }

            return m_initialized;
        }

        public void Terminate()
        {
            if(m_initialized)
            {
                m_leftHandOverlay = null;
                m_rightHandOverlay = null;
                m_initialized = false;
            }
        }

        public void SetHandTransform(Hand p_hand, GlmSharp.mat4 p_mat)
        {
            switch(p_hand)
            {
                case Hand.Left:
                    m_leftHandOverlay.SetWorldTransform(p_mat);
                    break;
                case Hand.Right:
                    m_rightHandOverlay.SetWorldTransform(p_mat);
                    break;
            }
        }

        public void SetHandPresence(Hand p_hand, bool p_state, GlmSharp.vec3 p_tipPos)
        {
            switch(p_hand)
            {
                case Hand.Left:
                {
                    m_leftHandOverlay.SetHandPresence(p_state, p_tipPos);

                    Valve.VR.HmdMatrix34_t l_matrix = new Valve.VR.HmdMatrix34_t();
                    (m_headTransform * GlmSharp.mat4.Translate(p_tipPos)).Convert(ref l_matrix);
                    Valve.VR.OpenVR.Overlay.SetOverlayTransformAbsolute(m_leftFingerOverlay, Valve.VR.ETrackingUniverseOrigin.TrackingUniverseRawAndUncalibrated, ref l_matrix);
                }
                break;
                case Hand.Right:
                {
                    m_rightHandOverlay.SetHandPresence(p_state, p_tipPos);

                    Valve.VR.HmdMatrix34_t l_matrix = new Valve.VR.HmdMatrix34_t();
                    (m_headTransform * GlmSharp.mat4.Translate(p_tipPos)).Convert(ref l_matrix);
                    Valve.VR.OpenVR.Overlay.SetOverlayTransformAbsolute(m_rightFingerOverlay, Valve.VR.ETrackingUniverseOrigin.TrackingUniverseRawAndUncalibrated, ref l_matrix);
                }
                break;
            }
        }

        public void SetHeadTransform(GlmSharp.mat4 p_transform)
        {
            m_headTransform = p_transform * m_core.GetConfigManager().GetRootTransform();
        }

        public void DoPulse()
        {
            m_leftHandOverlay.SetLocked(m_rightHandOverlay.IsInputActive());
            m_leftHandOverlay.Update(m_headTransform);

            m_rightHandOverlay.SetLocked(m_leftHandOverlay.IsInputActive());
            m_rightHandOverlay.Update(m_headTransform);

            ProcessControls(m_leftHandOverlay.GetControlButtons(), Hand.Left);
            ProcessControls(m_rightHandOverlay.GetControlButtons(), Hand.Right);
        }

        void ProcessControls(List<ControlButton> p_buttons, Hand p_hand)
        {
            foreach(var l_controlButton in p_buttons)
            {
                if(l_controlButton.IsUpdated())
                {
                    string l_message = "input ";
                    l_message += p_hand.ToString().ToLower();
                    l_message += ' ';
                    switch(l_controlButton.GetButtonType())
                    {
                        case ControlButton.ButtonType.Button:
                            l_message += "button ";
                            break;
                        case ControlButton.ButtonType.Axis:
                            l_message += "axis ";
                            break;
                    }
                    l_message += l_controlButton.GetButtonName();
                    l_message += ' ';

                    l_message += l_controlButton.GetButtonState().ToString().ToLower();

                    if(l_controlButton.GetButtonType() == ControlButton.ButtonType.Axis)
                    {
                        var l_axisValues = l_controlButton.GetAxisValues();

                        l_message += ' ';
                        l_message += l_axisValues.x.ToString("0.0", System.Globalization.CultureInfo.InvariantCulture);
                        l_message += ' ';
                        l_message += l_axisValues.y.ToString("0.0", System.Globalization.CultureInfo.InvariantCulture);
                    }

                    m_core.GetVRManager().SendMessage(l_message);
                }
            }
        }
    }
}
