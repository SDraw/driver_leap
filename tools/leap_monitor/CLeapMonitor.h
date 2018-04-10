#pragma once

class CLeapMonitor
{
    std::set<uint32_t> m_setLeapDevices;

    bool Init();
    void MainLoop();
    static void Shutdown();

    /** Send a message to the driver with the HMD coordinates (which are not available to the server side) */
    static bool TriggerRealignCoordinates(const vr::VREvent_t& Event);

    /** Keep track of which devices are using driver_leap */
    void UpdateTrackedDevice(uint32_t unTrackedDeviceIndex);

    bool IsLeapDevice(uint32_t unTrackedDeviceIndex);
public:
    explicit CLeapMonitor();
    ~CLeapMonitor();

    void Run();
};