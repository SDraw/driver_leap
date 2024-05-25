#pragma once
class CButton
{
public: // I _love_ C++ for this ...
    enum ButtonType : size_t
    {
        BT_Button = 0,
        BT_Axis
    };
    enum ButtonState : size_t
    {
        BS_None = 0,
        BS_Touched,
        BS_Clicked
    };
private:
    std::string m_name;
    ButtonType m_type;
    ButtonState m_state;
    glm::vec2 m_axisValues;
    bool m_updated = false;
public:
    CButton(ButtonType p_type, std::string p_name);
    ~CButton() = default;

    void SetState(ButtonState p_state);
    ButtonState GetState() const;

    void SetAxis(const glm::vec2 &p_value);
    const glm::vec2& GetAxis() const;

    const std::string& GetName() const;
    ButtonType GetType() const;

    bool IsUpdated() const;
    void ResetUpdate();
};

