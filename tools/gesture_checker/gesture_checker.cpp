/******************************************************************************\
* Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include <iostream>
#include <cstring>
#include <Leap.h>
#include "GestureMatcher.h"

using namespace Leap;

class SampleListener : public Listener {
public:
    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
    virtual void onDeviceChange(const Controller&);
    virtual void onServiceConnect(const Controller&);
    virtual void onServiceDisconnect(const Controller&);
    virtual void onServiceChange(const Controller&);
    virtual void onDeviceFailure(const Controller&);
    virtual void onLogMessage(const Controller&, MessageSeverity severity, int64_t timestamp, const char* msg);
};

const std::string fingerNames[] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };
const std::string boneNames[] = { "Metacarpal", "Proximal", "Middle", "Distal" };

void SampleListener::onInit(const Controller& controller) {
    std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
}

void SampleListener::onDisconnect(const Controller& controller) {
    // Note: not dispatched when running in a debugger.
    std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
    std::cout << "Exited" << std::endl;
}

static float maprange(float input, float minimum, float maximum)
{
    float mapped = (input - minimum) / (maximum - minimum);
    return std::max(std::min(mapped, 1.0f), 0.0f);
}

#include <windows.h>

void cls(HANDLE hConsole)
{
    COORD coordScreen = { 0, 0 };    // home for the cursor 
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    // Get the number of character cells in the current buffer. 

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        return;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill the entire screen with blanks.

    if (!FillConsoleOutputCharacter(hConsole,        // Handle to console screen buffer 
        (TCHAR) ' ',     // Character to write to the buffer
        dwConSize,       // Number of cells to write 
        coordScreen,     // Coordinates of first cell 
        &cCharsWritten))// Receive number of characters written
    {
        return;
    }

    // Get the current text attribute.

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        return;
    }

    // Set the buffer's attributes accordingly.

    if (!FillConsoleOutputAttribute(hConsole,         // Handle to console screen buffer 
        csbi.wAttributes, // Character attributes to use
        dwConSize,        // Number of cells to set attribute 
        coordScreen,      // Coordinates of first cell 
        &cCharsWritten)) // Receive number of characters written
    {
        return;
    }

    // Put the cursor at its home coordinates.

    SetConsoleCursorPosition(hConsole, coordScreen);
}
void SampleListener::onFrame(const Controller& controller) {
    // Get the most recent frame and report some basic information
    const Frame frame = controller.frame();

    cls(GetStdHandle(STD_OUTPUT_HANDLE));

    static GestureMatcher matcher;

    for (int i = 0; i < 2; i++)
    {
        float scores[GestureMatcher::NUM_GESTURES];
        bool handFound = matcher.MatchGestures(frame, (GestureMatcher::WhichHand)(i + 1), scores);
        if (handFound)
        {
            for (int j = 0; j < GestureMatcher::NUM_GESTURES; j++)
            {
                std::string tmp = GestureMatcher::GestureNameFromType((GestureMatcher::GestureType)j);
                if (!tmp.empty())
                    fprintf(stderr, "%-10s %-30s - %4.2f\n", i ? "Right Hand" : "Left Hand", tmp.c_str(), scores[j]);
            }

            // Go through the hands in the dataset
            HandList &hands = frame.hands();
            for (int h = 0; h < hands.count(); h++)
            {
                Hand &hand = hands[h];
            }
        }
    }

    Sleep(50);
}

void SampleListener::onFocusGained(const Controller& controller) {
    std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
    std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
#if 0
    std::cout << "Device Changed" << std::endl;
    const DeviceList devices = controller.devices();

    for (int i = 0; i < devices.count(); ++i) {
        std::cout << "id: " << devices[i].toString() << std::endl;
        std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
        std::cout << "  isSmudged:" << (devices[i].isSmudged() ? "true" : "false") << std::endl;
        std::cout << "  isLightingBad:" << (devices[i].isLightingBad() ? "true" : "false") << std::endl;
    }
#endif
}

void SampleListener::onServiceConnect(const Controller& controller) {
    std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
    std::cout << "Service Disconnected" << std::endl;
}

void SampleListener::onServiceChange(const Controller& controller) {
    std::cout << "Service Changed" << std::endl;
}

void SampleListener::onDeviceFailure(const Controller& controller) {
    std::cout << "Device Error" << std::endl;
    const Leap::FailedDeviceList devices = controller.failedDevices();

    for (FailedDeviceList::const_iterator dl = devices.begin(); dl != devices.end(); ++dl) {
        const FailedDevice device = *dl;
        std::cout << "  PNP ID:" << device.pnpId();
        std::cout << "    Failure type:" << device.failure();
    }
}

void SampleListener::onLogMessage(const Controller&, MessageSeverity s, int64_t t, const char* msg) {
    switch (s) {
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

int main(int argc, char** argv) {
    // Create a sample listener and controller
    SampleListener listener;
    Controller controller;

    // Have the sample listener receive events from the controller
    controller.addListener(listener);

    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

    controller.setPolicy(Leap::Controller::POLICY_ALLOW_PAUSE_RESUME);

    // Keep this process running until Enter is pressed
    std::cout << "Press Enter to quit, or enter 'p' to pause or unpause the service..." << std::endl;

    bool paused = false;
    while (true) {
        char c = std::cin.get();
        if (c == 'p') {
            paused = !paused;
            controller.setPaused(paused);
            std::cin.get(); //skip the newline
        }
        else
            break;
    }

    // Remove the sample listener when done
    controller.removeListener(listener);

    return 0;
}
