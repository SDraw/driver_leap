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
* Copy control_config.xml and profile.json from solution root to "SteamVR Leap Motion driver/leap/bin/<your_platform>"

You can use auto-build of latest commit from [latest release page](../../releases/latest) if you don't wish to build it yourself.
  
### Control configuration and inputs
You can set current game profile by editing control_config.xml.
Available game profiles:
* **default** - profile for all games. Control inputs can be restricted by setting parameters to "false".
Gestures:
  * Trigger - bending of the index finger
  * Grip - grab gesture
  * System menu - formed T-shape with two hands
  * Application menu - hand with palm directed towards face
  * Touchpad - Thumb press
  * Touchpad circle - index finger of hand directed to palm of another hand
* **vrchat** - profile for VRChat. Control restrictions are ignored.
Game gestures:
* Gun - corresponding hand gesture
* V-shape - corresponding hand gesture
* Point - corresponding hand gesture
* Rock out - corresponding hand gesture
* Thumbs up - corresponding hand gesture
* Spread hand - corresponding hand gesture
* Trigger - grab gesture
* Grip - slightly closed hand
* Application menu - formed T-shape with two hands
