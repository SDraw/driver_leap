namespace leap_control
{
    class LeapManager
    {
        static readonly GlmSharp.vec3 ms_zeroPoint = new GlmSharp.vec3(0f);

        bool m_initialized = false;

        readonly Core m_core = null;

        Leap.Controller m_controller = null;

        GlmSharp.vec3 m_leftTipPosition;
        GlmSharp.vec3 m_rightTipPosition;

        public LeapManager(Core p_core)
        {
            m_core = p_core;
        }

        public bool Initialize()
        {
            if(!m_initialized)
            {
                m_controller = new Leap.Controller();
                m_controller.StartConnection();
                m_controller.PolicyChange += OnControllerPolicyChange;
                m_controller.Device += OnControllerConnected;
                m_controller.DeviceLost += OnControllerDisconnected;
                m_initialized = true;
            }

            return m_initialized;
        }

        public void Terminate()
        {
            if(m_initialized)
            {
                if(m_controller != null)
                {
                    m_controller.StopConnection();
                    m_controller.Dispose();
                    m_controller = null;
                }

                m_initialized = false;
            }
        }

        void OnControllerPolicyChange(object sender, Leap.PolicyEventArgs e)
        {
            m_core.GetVRManager().ShowNotification("Policies changed");
        }

        void OnControllerConnected(object sender, Leap.DeviceEventArgs e)
        {
            m_core.GetVRManager().ShowNotification("Controller connected");
        }

        void OnControllerDisconnected(object sender, Leap.DeviceEventArgs e)
        {
            m_core.GetVRManager().ShowNotification("Controller disconnected");
        }

        public void DoPulse()
        {
            if(m_controller.IsConnected)
            {
                bool l_leftFound = false;
                bool l_rightFound = false;

                Leap.Frame l_frame = m_controller.Frame();
                if(l_frame != null)
                {
                    foreach(Leap.Hand l_hand in l_frame.Hands)
                    {
                        if(l_hand.IsLeft && !l_leftFound)
                        {
                            l_leftFound = true;
                            foreach(Leap.Finger l_finger in l_hand.Fingers)
                            {
                                if(l_finger.Type == Leap.Finger.FingerType.TYPE_INDEX)
                                {
                                    l_finger.TipPosition.Convert(ref m_leftTipPosition);
                                    ConvertOrientation(ref m_leftTipPosition);
                                    break;
                                }
                            }
                            continue;
                        }
                        if(l_hand.IsRight && !l_rightFound)
                        {
                            l_rightFound = true;
                            foreach(Leap.Finger l_finger in l_hand.Fingers)
                            {
                                if(l_finger.Type == Leap.Finger.FingerType.TYPE_INDEX)
                                {
                                    l_finger.TipPosition.Convert(ref m_rightTipPosition);
                                    ConvertOrientation(ref m_rightTipPosition);
                                    break;
                                }
                            }
                        }
                    }
                }

                m_core.GetControlManager().SetHandPresence(ControlManager.Hand.Left, l_rightFound, l_rightFound ? m_rightTipPosition : ms_zeroPoint);
                m_core.GetControlManager().SetHandPresence(ControlManager.Hand.Right, l_leftFound, l_leftFound ? m_leftTipPosition : ms_zeroPoint);
            }
        }

        void ConvertOrientation(ref GlmSharp.vec3 p_vec) => p_vec = -0.001f * p_vec.swizzle.xzy;
    }
}
