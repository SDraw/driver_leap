#pragma once

class CLeapPoller
{
    std::atomic<bool> m_active;
    std::mutex m_frameLock;
    std::thread *m_thread;

    LEAP_CONNECTION m_connection;
    LEAP_TRACKING_EVENT *m_lastFrame;
    LEAP_TRACKING_EVENT *m_newFrame;

    // Shenanigans for weird behaviour of new SDK
    std::atomic<bool> m_deviceValidFrames;

    void ThreadUpdate();
public:
    CLeapPoller();
    ~CLeapPoller();

    bool Initialize();
    void Terminate();

    bool IsConnected() const;
    const LEAP_TRACKING_EVENT* GetFrame();

    void SetPolicy(uint64_t p_set, uint64_t p_clear = 0U);
    void SetTrackingMode(eLeapTrackingMode p_mode);

    void Update();
};


