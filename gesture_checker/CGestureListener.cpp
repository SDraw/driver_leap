#include "stdafx.h"
#include "CGestureListener.h"
#include "CGestureMatcher.h"

void ClearConsole()
{
    const HANDLE l_console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO l_csbi;
    if(!GetConsoleScreenBufferInfo(l_console, &l_csbi))
    {
        return;
    }

    const COORD l_coordScreen = { 0, 0 };
    DWORD l_charsWritten;
    const DWORD l_conSize = l_csbi.dwSize.X * l_csbi.dwSize.Y;
    if(!FillConsoleOutputCharacter(l_console, (TCHAR) ' ', l_conSize, l_coordScreen, &l_charsWritten)) return;
    if(!GetConsoleScreenBufferInfo(l_console, &l_csbi)) return;
    if(!FillConsoleOutputAttribute(l_console, l_csbi.wAttributes, l_conSize, l_coordScreen, &l_charsWritten)) return;
    SetConsoleCursorPosition(l_console, l_coordScreen);
}

void CGestureListener::onInit(const Leap::Controller&)
{
    std::cout << "Initialized" << std::endl;
}

void CGestureListener::onConnect(const Leap::Controller&)
{
    std::cout << "Connected" << std::endl;
}

void CGestureListener::onDisconnect(const Leap::Controller&)
{
    // Note: not dispatched when running in a debugger.
    std::cout << "Disconnected" << std::endl;
}

void CGestureListener::onExit(const Leap::Controller&)
{
    std::cout << "Exited" << std::endl;
}

void CGestureListener::onFrame(const Leap::Controller& controller)
{
    ClearConsole();

    const Leap::Frame l_frame = controller.frame();
    std::vector<float> l_scoresLeft, l_scoresRight;
    CGestureMatcher::GetGestures(l_frame, CGestureMatcher::GH_LeftHand, l_scoresLeft);
    CGestureMatcher::GetGestures(l_frame, CGestureMatcher::GH_RightHand, l_scoresRight);

    const Leap::HeadPose l_headPose = controller.headPose(l_frame.timestamp());
    const Leap::Vector l_headPos = l_headPose.position();
    const Leap::Quaternion l_headRot = l_headPose.orientation();

    fprintf(stderr, "Head orientation: P(%.4f,%.4f,%.4f) R(%.4f,%.4f,%.4f,%.4f)\n", l_headPos.x, l_headPos.y, l_headPos.z, l_headRot.x, l_headRot.y, l_headRot.z, l_headRot.w);
    fprintf(stderr, "<-- CGestureMatcher data -->\n");
    for(size_t i = 0U; i < CGestureMatcher::GT_GesturesCount; i++)
    {
        std::string l_gestureName;
        CGestureMatcher::GetGestureName(static_cast<CGestureMatcher::GestureType>(i), l_gestureName);
        if(!l_gestureName.empty()) fprintf(stderr, "%-24s -> %.4f\t%.4f\n", l_gestureName.c_str(), l_scoresLeft[i], l_scoresRight[i]);
    }

    //for(auto l_hand : frame.hands())
    //{
    //    if(l_hand.isValid())
    //    {
    //        glm::vec3 l_handPosition = l_hand.wristPosition().toVector3<glm::vec3>();
    //        glm::quat l_handRotation = glm::quat_cast(l_hand.basis().toMatrix4x4<glm::mat4>());

    //        fprintf(stdout, "> %s hand: [%.4f,%.4f,%.4f] - [%.4f,%.4f,%.4f,%.4f]\n", l_hand.isLeft() ? "Left" : "Right",
    //            l_handPosition.x, l_handPosition.y, l_handPosition.z,
    //            l_handRotation.x, l_handRotation.y, l_handRotation.z, l_handRotation.w
    //            );

    //        for(auto l_finger : l_hand.fingers())
    //        {
    //            if(l_finger.isValid())
    //            {
    //                fprintf(stdout, ">> %s finger:\n", g_fingerNames[l_finger.type()].c_str());

    //                Leap::Bone l_bones[4U];
    //                for(size_t i = 0U; i < 4U; i++) l_bones[i] = l_finger.bone(static_cast<Leap::Bone::Type>(i));
    //                // basis() - global rotation
    //                // prevJoint() - global translation

    //                // Local position = Inv([ParentPos,ParentRot])*Pos
    //                glm::mat4 l_identity(1.f);
    //                glm::vec3 l_boneLocalPos[4U];
    //                l_boneLocalPos[0U] = glm::inverse(glm::translate(l_identity, l_handPosition)*glm::mat4_cast(l_handRotation))*l_bones[0U].prevJoint().toVector4<glm::vec4>(1.f);
    //                l_boneLocalPos[1U] = glm::inverse(glm::translate(l_identity, l_bones[0U].prevJoint().toVector3<glm::vec3>())*l_bones[0U].basis().toMatrix4x4<glm::mat4>())*l_bones[1U].prevJoint().toVector4<glm::vec4>(1.f);
    //                l_boneLocalPos[2U] = glm::inverse(glm::translate(l_identity, l_bones[1U].prevJoint().toVector3<glm::vec3>())*l_bones[1U].basis().toMatrix4x4<glm::mat4>())*l_bones[2U].prevJoint().toVector4<glm::vec4>(1.f);
    //                l_boneLocalPos[3U] = glm::inverse(glm::translate(l_identity, l_bones[2U].prevJoint().toVector3<glm::vec3>())*l_bones[2U].basis().toMatrix4x4<glm::mat4>())*l_bones[3U].prevJoint().toVector4<glm::vec4>(1.f);

    //                // Local rotation = Inv(ParentRot)*Rot
    //                glm::quat l_boneLocalRot[4U];
    //                l_boneLocalRot[0U] = glm::quat_cast((l_hand.basis().rigidInverse()*l_bones[0U].basis()).toMatrix4x4<glm::mat4>());
    //                l_boneLocalRot[1U] = glm::quat_cast((l_bones[0U].basis().rigidInverse()*l_bones[1U].basis()).toMatrix4x4<glm::mat4>());
    //                l_boneLocalRot[2U] = glm::quat_cast((l_bones[1U].basis().rigidInverse()*l_bones[2U].basis()).toMatrix4x4<glm::mat4>());
    //                l_boneLocalRot[3U] = glm::quat_cast((l_bones[2U].basis().rigidInverse()*l_bones[3U].basis()).toMatrix4x4<glm::mat4>());

    //                for(size_t i = 0U; i < 4U; i++)
    //                {
    //                    fprintf(stdout, ">>> Bone %u: [%.4f,%.4f,%.4f] - [%.4f,%.4f,%.4f,%.4f]\n", i,
    //                        l_boneLocalPos[i].x, l_boneLocalPos[i].y, l_boneLocalPos[i].z,
    //                        l_boneLocalRot[i].x, l_boneLocalRot[i].y, l_boneLocalRot[i].z, l_boneLocalRot[i].w
    //                        );
    //                }
    //            }
    //        }
    //    }
    //}
}

void CGestureListener::onFocusGained(const Leap::Controller&)
{
    std::cout << "Focus Gained" << std::endl;
}

void CGestureListener::onFocusLost(const Leap::Controller&)
{
    std::cout << "Focus Lost" << std::endl;
}

void CGestureListener::onDeviceChange(const Leap::Controller& controller)
{
    std::cout << "Device changed:" << std::endl;
    const Leap::DeviceList l_devices = controller.devices();
    for(const auto l_device : l_devices)
    {
        std::cout << "\tid: " << l_device.toString();
        std::cout << ", isStreaming " << (l_device.isStreaming() ? "true" : "false");
        std::cout << ", isSmudged " << (l_device.isSmudged() ? "true" : "false");
        std::cout << ", isLightingBad " << (l_device.isLightingBad() ? "true" : "false") << std::endl;
    }
}

void CGestureListener::onServiceConnect(const Leap::Controller&)
{
    std::cout << "Service Connected" << std::endl;
}

void CGestureListener::onServiceDisconnect(const Leap::Controller&)
{
    std::cout << "Service Disconnected" << std::endl;
}

void CGestureListener::onServiceChange(const Leap::Controller&)
{
    std::cout << "Service Changed" << std::endl;
}

void CGestureListener::onDeviceFailure(const Leap::Controller& controller)
{
    std::cout << "Device error:" << std::endl;
    const Leap::FailedDeviceList l_devices = controller.failedDevices();
    for(const auto l_device : l_devices)
    {
        std::cout << "\tPNP ID " << l_device.pnpId();
        std::cout << ", failure type " << l_device.failure() << std::endl;;
    }
}

void CGestureListener::onLogMessage(const Leap::Controller&, Leap::MessageSeverity severity, int64_t timestamp, const char* msg)
{
    switch(severity)
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
    std::cout << '[' << timestamp << "] " << msg << std::endl;
}
