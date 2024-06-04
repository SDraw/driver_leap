#pragma once

class CLeapIndexController;
class CControllerInput
{
    struct Device
    {
        bool connected{ false };
        int handle{ -1 };
    };

    int m_deviceCount;
    std::map<int, Device> m_devices;
public:
    explicit CControllerInput();
    ~CControllerInput();

    bool IsConnected();
    void Update(CLeapIndexController *p_left, CLeapIndexController *p_right);
};
