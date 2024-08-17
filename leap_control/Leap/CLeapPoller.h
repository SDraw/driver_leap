#pragma once

class CLeapPoller
{
    std::atomic<bool> m_isRunning;
    std::mutex m_frameLock;
    std::thread *m_thread;

    LEAP_CONNECTION m_connection;
    std::atomic<int> m_devicesCount;
    LEAP_TRACKING_EVENT *m_frame;

    // Async methods
    void PollThread();
    void OnConnectionEvent();
    void OnConnectionLostEvent();
    void OnDeviceEvent(const LEAP_DEVICE_EVENT* p_event);
    void OnDeviceLostEvent(const LEAP_DEVICE_EVENT *p_event);
    void OnTrackingEvent(const LEAP_TRACKING_EVENT* p_event);
public:
    CLeapPoller();
    ~CLeapPoller();

    void Start();
    void Stop();

    bool IsConnected() const;
    bool GetFrame(LEAP_TRACKING_EVENT *p_target);

    void SetPolicy(uint64_t p_set, uint64_t p_clear = 0U);
#ifndef LEAP_ORION
    void SetTrackingMode(eLeapTrackingMode p_mode);
#endif
};


