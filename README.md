[![Build status](https://ci.appveyor.com/api/projects/status/2pc49d2hpt2hx944?svg=true)](https://ci.appveyor.com/project/SDraw/driver-leap) [![Release](http://img.shields.io/github/release/SDraw/driver_leap.svg)](../../releases/latest)

Fork with updated LeapSDK and OpenVR.
Refer to [parent repository](https://github.com/cbuchner1/driver_leap) for base project installation.

### Building
* Open solution in Visual Studio 2013
* Build your platform:
  * x64 - build output is in "<solution_folder>/x64"
  * x86 - build output is in "<solution_folder>/Win32"
  
### Installation
* Install [base project driver](https://github.com/cbuchner1/driver_leap)
* Copy build files to "SteamVR Leap Motion driver/leap/bin/<your_platform>":
  * driver_leap.dll
  * gesture_checker.exe
  * leap_monitor.exe
* Copy additional shared libraries to "SteamVR Leap Motion driver/leap/bin/<your_platform>":
  * vendor/LeapSDK/lib/<your_platform>/Leap.dll
  * vendor/openvr/bin/<your_platform>/openvr_api.dll
* Copy control_config.xml from solution root to "SteamVR Leap Motion driver/leap/bin/<your_platform>"
  
### Configuration
You can edit control_config.xml to disable specified controls.
Note: disabling touchpad also disables touchpad axes and several gestures.
