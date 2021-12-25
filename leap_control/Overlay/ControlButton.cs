namespace leap_control
{
    class ControlButton
    {
        public enum ButtonType
        {
            Button = 0,
            Axis
        };

        public enum ButtonState
        {
            None = 0,
            Touched,
            Clicked
        }

        string m_name;
        ButtonType m_type;
        ButtonState m_state;
        GlmSharp.vec2 m_axisValues;
        bool m_updated = false;

        public ControlButton(ButtonType p_type, string p_name)
        {
            m_name = p_name;
            m_type = p_type;
        }

        public void SetState(ButtonState p_state)
        {
            if(m_state != p_state)
            {
                m_state = p_state;
                m_updated = true;
            }
        }

        public void SetAxes(GlmSharp.vec2 p_vec)
        {
            if(m_type == ButtonType.Axis && (m_axisValues != p_vec))
            {
                m_axisValues = p_vec;
                m_updated = true;
            }
        }

        public string GetButtonName() => m_name;
        public ButtonType GetButtonType() => m_type;
        public ButtonState GetButtonState() => m_state;
        public GlmSharp.vec2 GetAxisValues() => m_axisValues;
        public bool IsUpdated() => m_updated;
        public void ResetUpdate() => m_updated = false;
    }
}
