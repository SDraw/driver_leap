#include "stdafx.h"

#include "Core/CLeapPoller.h"

CLeapPoller::CLeapPoller()
{
    m_active = false;
    m_thread = nullptr;
    m_connection = nullptr;
    m_lastFrame = new LEAP_TRACKING_EVENT();
    m_newFrame = new LEAP_TRACKING_EVENT();
    m_deviceValidFrames = false;
}

CLeapPoller::~CLeapPoller()
{
    delete m_lastFrame;
    delete m_newFrame;
}

bool CLeapPoller::Initialize()
{
    if(!m_active)
    {
        if(LeapCreateConnection(nullptr, &m_connection) == eLeapRS_Success)
        {
            if(LeapOpenConnection(m_connection) == eLeapRS_Success)
            {
                m_active = true;
                m_thread = new std::thread(&CLeapPoller::ThreadUpdate, this);
            }
            else
            {
                LeapDestroyConnection(m_connection);
                m_connection = nullptr;
            }
        }
        else m_connection = nullptr;
    }

    return m_active;
}

void CLeapPoller::Terminate()
{
    if(m_active)
    {
        m_active = false;
        m_thread->join();
        m_thread = nullptr;

        LeapCloseConnection(m_connection);
        LeapDestroyConnection(m_connection);

        m_connection = nullptr;
    }
}

bool CLeapPoller::IsConnected() const
{
    return m_deviceValidFrames;
}

const LEAP_TRACKING_EVENT* CLeapPoller::GetFrame()
{
    LEAP_TRACKING_EVENT *l_result = nullptr;
    if(m_active && m_deviceValidFrames) l_result = m_lastFrame;
    return l_result;
}

void CLeapPoller::SetTrackingMode(eLeapTrackingMode p_mode)
{
    if(m_active) LeapSetTrackingMode(m_connection, p_mode);
}

void CLeapPoller::SetPolicy(uint64_t p_set, uint64_t p_clear)
{
    if(m_active) LeapSetPolicyFlags(m_connection, p_set, p_clear);
}

void CLeapPoller::Update()
{
    if(m_active)
    {
        if(m_frameLock.try_lock())
        {
            std::memcpy(m_lastFrame, m_newFrame, sizeof(LEAP_TRACKING_EVENT));
            m_frameLock.unlock();
        }
    }
}

void CLeapPoller::ThreadUpdate()
{
    const std::chrono::milliseconds l_threadDelay(1U);

    while(m_active)
    {
        // Poll events
        LEAP_CONNECTION_MESSAGE l_message{ sizeof(LEAP_CONNECTION_MESSAGE) };
        while(LeapPollConnection(m_connection, 0U, &l_message) == eLeapRS_Success)
        {
            if(l_message.type == eLeapEventType_None) break;
            switch(l_message.type)
            {
                case eLeapEventType_ConnectionLost:
                {
                    m_deviceValidFrames = false;
                } break;
                case eLeapEventType_Device:
                {
                    LEAP_DEVICE l_device;
                    LeapOpenDevice(l_message.device_event->device, &l_device);
                } break;
                case eLeapEventType_DeviceLost:
                {
                    m_deviceValidFrames = false;
                } break;
                case eLeapEventType_Tracking:
                {
                    m_frameLock.lock();
                    std::memcpy(m_newFrame, l_message.tracking_event, sizeof(LEAP_TRACKING_EVENT));
                    m_frameLock.unlock();
                    m_deviceValidFrames = true;
                } break;
            }
        }

        std::this_thread::sleep_for(l_threadDelay);
    }
}
