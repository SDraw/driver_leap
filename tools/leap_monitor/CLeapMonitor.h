#pragma once

class CLeapMonitor
{
    std::string m_strOverlayImagePath;
    vr::VROverlayHandle_t m_OverlayHandle;
    enum EOverlayToDisplay
    {
        k_eNone, k_ePointAtBaseForHemisphereTracking, k_eHoldAtShouldersForCoordinateAlignment
    } m_eCurrentOverlay;
    std::set<uint32_t> m_setLeapDevices;

    bool Init();
    void MainLoop();
    static void Shutdown();

    bool ShowOverlay(EOverlayToDisplay eOverlay);
    void HideOverlay();

    /** Send a message to the driver with the HMD coordinates (which are not available to the server side) */
    static bool TriggerRealignCoordinates(const vr::VREvent_t& Event);

    /** Keep track of which devices are using driver_leap */
    void UpdateTrackedDevice(uint32_t unTrackedDeviceIndex);

    /** If any Leap devices need automatic hemisphere tracking enabled, prompt the user to do that.
     * Otherwise, if any devices do not know how to transform into the global coordinate system, show
     * instructions for that. */
    EOverlayToDisplay BestOverlayForLeapDevices();

    bool IsLeapDevice(uint32_t unTrackedDeviceIndex);
public:
    explicit CLeapMonitor(const std::string& path);
    ~CLeapMonitor();

    void Run();
};