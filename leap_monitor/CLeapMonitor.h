#pragma once

class CLeapMonitor
{
    std::set<uint32_t> m_setLeapDevices;

    bool Init();
    void MainLoop();
    static void Shutdown();

    /** Keep track of which devices are using driver_leap */
    void UpdateTrackedDevice(uint32_t unTrackedDeviceIndex);
    void UpdateApplicationKey(const char *f_appKey);
public:
    explicit CLeapMonitor();
    ~CLeapMonitor();

    void Run();
};