using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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

        public ControlButton(ButtonType f_type, string f_name)
        {
            m_name = f_name;
            m_type = f_type;
        }

        public void SetState(ButtonState f_state)
        {
            if(m_state != f_state)
            {
                m_state = f_state;
                m_updated = true;
            }
        }

        public void SetAxes(GlmSharp.vec2 f_vec)
        {
            if(m_type == ButtonType.Axis && (m_axisValues != f_vec))
            {
                m_axisValues = f_vec;
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
