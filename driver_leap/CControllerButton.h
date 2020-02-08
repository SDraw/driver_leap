#pragma once

class CControllerButton final
{
    vr::VRInputComponentHandle_t m_handle;

    unsigned char m_inputType;
    bool m_state;
    float m_value;
    bool m_updated;

    CControllerButton(const CControllerButton &that) = delete;
    CControllerButton& operator=(const CControllerButton &that) = delete;
public:
    enum InputType : unsigned char
    {
        IT_None = 0U,
        IT_Boolean,
        IT_Float
    };

    CControllerButton();
    ~CControllerButton();

    inline vr::VRInputComponentHandle_t GetHandle() const { return m_handle; }
    inline vr::VRInputComponentHandle_t& GetHandleRef() { return m_handle; }

    inline void SetInputType(unsigned char f_type) { m_inputType = f_type; }
    inline unsigned char GetInputType() const { return m_inputType; }

    void SetState(bool f_state);
    inline bool GetState() const { return m_state; }

    void SetValue(float f_value);
    inline float GetValue() const { return m_value; }

    inline bool IsUpdated() const { return m_updated; }
    inline void ResetUpdate() { m_updated = false; }
};
