//========= Copyright Valve Corporation ============//
//
// leap_monitor.cpp : Interacts with driver_leap to provide overlay instructions at startup
//

#include "stdafx.h"
#include "leap_monitor.h"
#include <openvr.h>

#include <set>
#include <chrono>
#include <thread>
#include <sstream>

const std::chrono::milliseconds k_MonitorInterval( 10 );

class CLeapMonitor
{
public:
    CLeapMonitor( const std::string & path )
        : m_strOverlayImagePath( path )
        , m_OverlayHandle( vr::k_ulOverlayHandleInvalid )
        , m_eCurrentOverlay( k_eNone )
    {}

    ~CLeapMonitor() {}

    void Run()
    {
        if ( Init() )
        {
            MainLoop();
        }

        Shutdown();
    }

protected:

    enum EOverlayToDisplay { k_eNone, k_ePointAtBaseForHemisphereTracking, k_eHoldAtShouldersForCoordinateAlignment };

    bool Init()
    {
        // Start as "background" application.  This prevents vrserver from being started
        // on our behalf, and prevents us from keeping vrserver alive when everything else
        // exits.  This is very important because we're spawning from a driver, and any
        // class besides "background" would keep vrserver running forever
        vr::EVRInitError eVRInitError;
        vr::VR_Init( &eVRInitError, vr::VRApplication_Background );
        if ( !vr::VRSystem() || eVRInitError != vr::VRInitError_None )
            return false;
        
        // Keep track of which devices use driver_leap
        for ( int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i )
        {
            UpdateTrackedDevice( i );
        }

        return true;
    }

    void MainLoop()
    {
        while ( true )
        {
            std::this_thread::sleep_for( k_MonitorInterval );

#if defined( WIN32 )
            MSG msg = { 0 };
            while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }

            if ( msg.message == WM_QUIT )
                break;
#endif

            // Display instructions for user if we find any devices that need them
            ShowOverlay( BestOverlayForLeapDevices() );

            vr::VREvent_t Event;
            while ( vr::VRSystem()->PollNextEvent( &Event, sizeof( Event ) ) )
            {
                switch ( Event.eventType )
                {
                case vr::VREvent_Quit:
                    exit( 0 );
                    // NOTREAHED

                case vr::VREvent_TrackedDeviceActivated:
                case vr::VREvent_TrackedDeviceUpdated:
                    UpdateTrackedDevice( Event.trackedDeviceIndex );
                    break;

                case vr::VREvent_VendorSpecific_Reserved_Start + 0:
                    // User has made the "align" gesture.  The driver can't see the HMD
                    // coordinates, so we forward those from our client view.
                    if ( IsLeapDevice( Event.trackedDeviceIndex ) )
                    {
//                        printf("received event vr::VREvent_VendorSpecific_Reserved_Start + 0 for device %d\n", Event.trackedDeviceIndex);
                        TriggerRealignCoordinates( Event );
                    }
                    break;
                }
            }
        }
    }

    /** Create and show an overlay nailed to the user's face */
    bool ShowOverlay( EOverlayToDisplay eOverlay )
    {
        if ( m_eCurrentOverlay == eOverlay && m_OverlayHandle != vr::k_ulOverlayHandleInvalid )
            return true;

        // Hiding or changing, so destroy old overlay
        HideOverlay();

        if ( eOverlay == k_eNone )
        {
            m_eCurrentOverlay = eOverlay;
            return true;
        }

        // Compositor must be initialized to create overlays
        if ( !vr::VRCompositor() )
            return false;

        vr::EVROverlayError eOverlayError = vr::VROverlay()->CreateOverlay( "leap_monitor", "Leap Monitor", &m_OverlayHandle );
        if ( eOverlayError != vr::VROverlayError_None )
            return false;

        vr::HmdMatrix34_t matInFrontOfHead;
        memset( &matInFrontOfHead, 0, sizeof( matInFrontOfHead ) );
        float scale = 1.4f;
        matInFrontOfHead.m[0][0] = matInFrontOfHead.m[1][1] = matInFrontOfHead.m[2][2] = scale;
        matInFrontOfHead.m[2][3] = -2.0f;
        eOverlayError = vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative( m_OverlayHandle,    vr::k_unTrackedDeviceIndex_Hmd,    &matInFrontOfHead );
        if ( eOverlayError != vr::VROverlayError_None )
            return false;

        std::string image;
        switch ( eOverlay )
        {
        case k_ePointAtBaseForHemisphereTracking:
            image = m_strOverlayImagePath + "need_hemisphere_tracking.png";
            break;

        case k_eHoldAtShouldersForCoordinateAlignment:
            image = m_strOverlayImagePath + "need_alignment_gesture.png";
            break;

        default:
            HideOverlay();
            return false;
        }

        eOverlayError = vr::VROverlay()->SetOverlayFromFile( m_OverlayHandle, image.c_str() );
        if ( eOverlayError != vr::VROverlayError_None )
            return false; 
        
        eOverlayError = vr::VROverlay()->ShowOverlay( m_OverlayHandle );
        if ( eOverlayError != vr::VROverlayError_None )
            return false;

        m_eCurrentOverlay = eOverlay;

        return true;
    }

    void HideOverlay()
    {
        if ( m_OverlayHandle == vr::k_ulOverlayHandleInvalid )
            return;

        vr::VRCompositor();  // Required to call overlays...
        vr::VROverlay()->HideOverlay( m_OverlayHandle );
        vr::VROverlay()->DestroyOverlay( m_OverlayHandle );
        m_OverlayHandle = vr::k_ulOverlayHandleInvalid;
    }

    /** Send a message to the driver with the HMD coordinates (which are not available to the server side) */
    bool TriggerRealignCoordinates( const vr::VREvent_t & Event )
    {
        vr::TrackedDevicePose_t hmdPose;
        vr::VRSystem()->GetDeviceToAbsoluteTrackingPose( vr::TrackingUniverseRawAndUncalibrated, -Event.eventAgeSeconds, &hmdPose, 1 );
        if ( !hmdPose.bPoseIsValid )
            return false;

        std::ostringstream ss;
        char rgchReplyBuf[256];

        ss << "leap:realign_coordinates";
        for ( int i = 0; i < 3; ++i )
        {
            for ( int j = 0; j < 4; ++j )
            {
                ss << " " << hmdPose.mDeviceToAbsoluteTracking.m[i][j];
            }
        }
//        printf("%s\n", ss.str().c_str());
        vr::VRSystem()->DriverDebugRequest( Event.trackedDeviceIndex, ss.str().c_str(), rgchReplyBuf, sizeof( rgchReplyBuf ) );
        return true;
    }

    void Shutdown()
    {
        vr::VR_Shutdown();
    }

    /** Keep track of which devices are using driver_leap */
    void UpdateTrackedDevice( uint32_t unTrackedDeviceIndex )
    {
        char rgchTrackingSystemName[vr::k_unTrackingStringSize];
        vr::ETrackedPropertyError eError;

        uint32_t size = vr::VRSystem()->GetStringTrackedDeviceProperty( unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String, rgchTrackingSystemName, sizeof( rgchTrackingSystemName ), &eError );
        if ( eError == vr::TrackedProp_Success )
        {
            if ( strcmp( rgchTrackingSystemName, "leap" ) == 0 )
            {
                m_setLeapDevices.insert( unTrackedDeviceIndex );
            }
        }
    }

    /** If any Leap devices need automatic hemisphere tracking enabled, prompt the user to do that.
     * Otherwise, if any devices do not know how to transform into the global coordinate system, show
     * instructions for that. */
    EOverlayToDisplay BestOverlayForLeapDevices()
    {
        bool bNeedCoordinateAlignment = false;
        vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];

        // The "raw and uncalibrated" universe gives us coordinates in the HMD's native tracking space.
        // Adjustments like room setup and seated zero position will be applied equally to the HMD and
        // the coordinates we return, so the "raw" space is what we want our driver to match.
        vr::VRSystem()->GetDeviceToAbsoluteTrackingPose( vr::TrackingUniverseRawAndUncalibrated, 0, poses, vr::k_unMaxTrackedDeviceCount );
        for ( auto it = m_setLeapDevices.begin(); it != m_setLeapDevices.end(); ++it )
        {
            if ( poses[*it].bDeviceIsConnected )
            {
                switch ( poses[*it].eTrackingResult )
                {
                case vr::TrackingResult_Uninitialized:
                    // Getting all devices to have hemisphere tracking is high priority
                    return k_ePointAtBaseForHemisphereTracking;

                case vr::TrackingResult_Calibrating_InProgress:
                    bNeedCoordinateAlignment = true;
                    break;
                }
            }
        }

        if ( bNeedCoordinateAlignment )
            return k_eHoldAtShouldersForCoordinateAlignment;

        return k_eNone;
    }

    bool IsLeapDevice( uint32_t unTrackedDeviceIndex )
    {
        return ( m_setLeapDevices.count( unTrackedDeviceIndex ) != 0 );
    }

private:
    std::string m_strOverlayImagePath;
    vr::VROverlayHandle_t m_OverlayHandle;
    EOverlayToDisplay m_eCurrentOverlay;
    std::set<uint32_t> m_setLeapDevices;
};

#if 0
int main(int argc, char **argv)
{
    // Find resource path
    HMODULE hModule = GetModuleHandleA(NULL);
    char path[MAX_PATH];
    GetModuleFileNameA(hModule, path, MAX_PATH);

    char *snip = strstr(path, "\\bin\\");
    if (snip)
    {
        *snip = '\0';
    }

    std::string resources = std::string(path) + "\\resources\\overlays\\";

    CLeapMonitor LeapMonitor(resources);

    LeapMonitor.Run();
}
#else
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Find resource path
    HMODULE hModule = GetModuleHandleA( NULL );
    char path[MAX_PATH];
    GetModuleFileNameA( hModule, path, MAX_PATH );

    char *snip = strstr( path, "\\bin\\" );
    if ( snip )
    {
        *snip = '\0';
    }

    std::string resources = std::string( path ) + "\\resources\\overlays\\";

    CLeapMonitor LeapMonitor( resources );

    LeapMonitor.Run();
}
#endif
