#include "stdafx.h"
#include "CGestureListener.h"
#include "CGestureMatcher.h"

void cls(HANDLE hConsole)
{
    COORD coordScreen = { 0, 0 };    // home for the cursor 
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    if(!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        return;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    if(!FillConsoleOutputCharacter(hConsole, (TCHAR) ' ', dwConSize, coordScreen, &cCharsWritten)) return;
    if(!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        return;
    }
    if(!FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten)) return;
    SetConsoleCursorPosition(hConsole, coordScreen);
}

void CGestureListener::onInit(const Leap::Controller& controller)
{
    std::cout << "Initialized" << std::endl;
}

void CGestureListener::onConnect(const Leap::Controller& controller)
{
    std::cout << "Connected" << std::endl;
}

void CGestureListener::onDisconnect(const Leap::Controller& controller)
{
    // Note: not dispatched when running in a debugger.
    std::cout << "Disconnected" << std::endl;
}

void CGestureListener::onExit(const Leap::Controller& controller)
{
    std::cout << "Exited" << std::endl;
}

void CGestureListener::onFrame(const Leap::Controller& controller)
{
    // Get the most recent frame and report some basic information
    const Leap::Frame frame = controller.frame();

    cls(GetStdHandle(STD_OUTPUT_HANDLE));

    for(int i = 0; i < 2; i++)
    {
        float scores[CGestureMatcher::NUM_GESTURES];
        bool handFound = CGestureMatcher::MatchGestures(frame, (CGestureMatcher::WhichHand)(i + 1), scores);
        if(handFound)
        {
            for(int j = 0; j < CGestureMatcher::NUM_GESTURES; j++)
            {
                std::string tmp = CGestureMatcher::GestureNameFromType((CGestureMatcher::GestureType)j);
                if(!tmp.empty())
                    fprintf(stderr, "%-10s %-30s - %4.2f\n", i ? "Right Hand" : "Left Hand", tmp.c_str(), scores[j]);
            }

            // Go through the hands in the dataset
            /*HandList &hands = frame.hands();
            for (int h = 0; h < hands.count(); h++)
            {
            Hand &hand = hands[h];
            }*/
        }
    }

    Sleep(50);
}

void CGestureListener::onFocusGained(const Leap::Controller& controller)
{
    std::cout << "Focus Gained" << std::endl;
}

void CGestureListener::onFocusLost(const Leap::Controller& controller)
{
    std::cout << "Focus Lost" << std::endl;
}

void CGestureListener::onDeviceChange(const Leap::Controller& controller)
{
    /*std::cout << "Device Changed" << std::endl;
    const DeviceList devices = controller.devices();

    for (int i = 0; i < devices.count(); ++i) {
    std::cout << "id: " << devices[i].toString() << std::endl;
    std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
    std::cout << "  isSmudged:" << (devices[i].isSmudged() ? "true" : "false") << std::endl;
    std::cout << "  isLightingBad:" << (devices[i].isLightingBad() ? "true" : "false") << std::endl;
    }*/
}

void CGestureListener::onServiceConnect(const Leap::Controller& controller)
{
    std::cout << "Service Connected" << std::endl;
}

void CGestureListener::onServiceDisconnect(const Leap::Controller& controller)
{
    std::cout << "Service Disconnected" << std::endl;
}

void CGestureListener::onServiceChange(const Leap::Controller& controller)
{
    std::cout << "Service Changed" << std::endl;
}

void CGestureListener::onDeviceFailure(const Leap::Controller& controller)
{
    std::cout << "Device Error" << std::endl;
    const Leap::FailedDeviceList devices = controller.failedDevices();

    for(Leap::FailedDeviceList::const_iterator dl = devices.begin(); dl != devices.end(); ++dl)
    {
        const Leap::FailedDevice device = *dl;
        std::cout << "  PNP ID:" << device.pnpId();
        std::cout << "    Failure type:" << device.failure();
    }
}

void CGestureListener::onLogMessage(const Leap::Controller&, Leap::MessageSeverity s, int64_t t, const char* msg)
{
    switch(s)
    {
        case Leap::MESSAGE_CRITICAL:
            std::cout << "[Critical]";
            break;
        case Leap::MESSAGE_WARNING:
            std::cout << "[Warning]";
            break;
        case Leap::MESSAGE_INFORMATION:
            std::cout << "[Info]";
            break;
        case Leap::MESSAGE_UNKNOWN:
            std::cout << "[Unknown]";
    }
    std::cout << "[" << t << "] ";
    std::cout << msg << std::endl;
}