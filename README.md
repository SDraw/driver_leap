Fork with updated LeapSDK to 3.2.1 and OpenVR to 1.0.5, disabled touchpad movement gestures and removed installer.
Refer to [parent repository](https://github.com/cbuchner1/driver_leap) for base project.

### Building
* Open project in Visual Studio 2017
* Build your platform:
  * x64 - build output is in "<solution_folder>/x64"
  * x86 - build output is in "<solution_folder>/Win32"
* Copy build files to "SteamVR Leap Motion driver/leap/bin/<your_platform>":
  * driver_leap.dll
  * gesture_checker.exe
  * leap_monitor.exe
* Copy additional shared libraries to "SteamVR Leap Motion driver/leap/bin/<your_platform>":
  * vendor/LeapSDK/lib/<your_platform>/Leap.dll
  * vendor/openvr/lib/<your_platform>/openvr_api.dll
