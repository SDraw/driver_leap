using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace leap_control
{
    class ControlManager
    {
        public enum Hand
        {
            Left = 0,
            Right
        };

        static readonly GlmSharp.vec4 ms_pointMultiplier = new GlmSharp.vec4(0f, 0f, 0f, 1f);

        bool m_initialized = false;

        Core m_core = null;

        HandOverlay m_leftHandOverlay = null;
        HandOverlay m_rightHandOverlay = null;

        ulong m_leftFingerOverlay = 0;
        ulong m_rightFingerOverlay = 0;

        GlmSharp.mat4 m_headTransform;

        public ControlManager(Core f_core)
        {
            m_core = f_core;
        }

        public bool Initialize()
        {
            if(!m_initialized)
            {
                HandOverlay.LoadResources();

                m_leftHandOverlay = new HandOverlay(HandOverlay.Hand.Left);
                m_rightHandOverlay = new HandOverlay(HandOverlay.Hand.Right);

                Valve.VR.OpenVR.Overlay.CreateOverlay("leap.cursor.left", "Left Hand Finger Cursor", ref m_leftFingerOverlay);
                Valve.VR.OpenVR.Overlay.SetOverlayFromFile(m_leftFingerOverlay, System.AppDomain.CurrentDomain.BaseDirectory + "resources\\tx_cursor.png");
                Valve.VR.OpenVR.Overlay.SetOverlayColor(m_leftFingerOverlay, 0f, 1f, 0f);
                Valve.VR.OpenVR.Overlay.SetOverlayWidthInMeters(m_leftFingerOverlay, 0.006625f);
                Valve.VR.OpenVR.Overlay.SetOverlaySortOrder(m_leftFingerOverlay, 1);
                Valve.VR.OpenVR.Overlay.ShowOverlay(m_leftFingerOverlay);

                Valve.VR.OpenVR.Overlay.CreateOverlay("leap.cursor.right", "Left Hand Finger Cursor", ref m_rightFingerOverlay);
                Valve.VR.OpenVR.Overlay.SetOverlayFromFile(m_rightFingerOverlay, System.AppDomain.CurrentDomain.BaseDirectory + "resources\\tx_cursor.png");
                Console.WriteLine(System.AppDomain.CurrentDomain.BaseDirectory + "resources\\tx_cursor.png");
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

        public void SetHandTransform(Hand f_hand, GlmSharp.mat4 f_mat)
        {
            switch(f_hand)
            {
                case Hand.Left:
                    m_leftHandOverlay.SetWorldTransform(f_mat);
                    break;
                case Hand.Right:
                    m_rightHandOverlay.SetWorldTransform(f_mat);
                    break;
            }
        }

        public void SetHandPresence(Hand f_hand, bool f_state, GlmSharp.vec3 f_tipPos)
        {
            switch(f_hand)
            {
                case Hand.Left:
                {
                    m_leftHandOverlay.SetHandPresence(f_state, f_tipPos);

                    Valve.VR.HmdMatrix34_t l_matrix = new Valve.VR.HmdMatrix34_t();
                    (m_headTransform * GlmSharp.mat4.Translate(f_tipPos)).Convert(ref l_matrix);
                    Valve.VR.OpenVR.Overlay.SetOverlayTransformAbsolute(m_leftFingerOverlay, Valve.VR.ETrackingUniverseOrigin.TrackingUniverseRawAndUncalibrated, ref l_matrix);
                }
                break;
                case Hand.Right:
                {

                    m_rightHandOverlay.SetHandPresence(f_state, f_tipPos);

                    Valve.VR.HmdMatrix34_t l_matrix = new Valve.VR.HmdMatrix34_t();
                    (m_headTransform * GlmSharp.mat4.Translate(f_tipPos)).Convert(ref l_matrix);
                    Valve.VR.OpenVR.Overlay.SetOverlayTransformAbsolute(m_rightFingerOverlay, Valve.VR.ETrackingUniverseOrigin.TrackingUniverseRawAndUncalibrated, ref l_matrix);
                }
                break;
            }
        }

        public void SetHeadTransform(GlmSharp.mat4 f_transform)
        {
            m_headTransform = f_transform;
        }

        public void DoPulse()
        {
            m_leftHandOverlay.Update(m_headTransform);
            m_rightHandOverlay.Update(m_headTransform);

            ProcessControls(m_leftHandOverlay.GetControlButtons(), Hand.Left);
            ProcessControls(m_rightHandOverlay.GetControlButtons(), Hand.Right);
        }

        void ProcessControls(List<ControlButton> l_buttons, Hand f_hand)
        {
            foreach(var l_controlButton in l_buttons)
            {
                if(l_controlButton.IsUpdated())
                {
                    string l_message = "input ";
                    l_message += f_hand.ToString().ToLower();
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

                    m_core.VRManager.SendMessage(l_message);
                }
            }
        }
    }
}
