# Driver Leap [![Release](http://img.shields.io/github/release/SDraw/driver_leap.svg)](../../releases/latest)
Self-sustainable fork of SteamVR driver for Leap Motion controller with updated vendor libraries

[![](./.github/img_01.png)](https://www.youtube.com/watch?v=RdGnCV2g_oE)
  
## Installation (for users)
* Install [latest Ultraleap Gemini](https://developer.leapmotion.com/tracking-software-download)
* Extract [latest release archive](../../releases/latest) to `<SteamVR_folder>/drivers`
* Add line in section `steamvr` of `<Steam_folder>/config/steamvr.vrsettings` file:
```JSON
"activateMultipleDrivers": true,
```

## Usage
### Settings
Driver settings are configurated by editing `resources/settings.xml`. Available settings:
* `trackingLevel`: skeleton tracking style for OpenVR. Can be `partial` or `full`; `partial` by default.
* `handsReset`: marks controller as out of range if hand isn't detected by Leap Motion; `false` by default.
* `useVelocity`: enables velocity data from Leap Motion for hands; `false` by default.
* `rootOffset`: local position offset from HMD view point; values are in meters.
* `rootAngle`: local rotation offset for axis X, can be used with `rootOffset` to configure tracking for neck mounts; value is in radians.
* `handsOffset`: local position offset for controllers from theirs transformation point, X axis will be inverted for right hand; values are in meters.
* `handsRotationOffset`: local rotation offset for controllers from theirs transformation point, Y and Z axes will be inverted for right hand; values are in radians.

Settings can be reloaded at runtime from menu item by clicking on "Driver Leap Control" tray icon. All settings (except `trackingLevel`) are reloaded and applied.

### Gestures
List of hands gestures that correspond to controller original input:
* **Grip:** bending of middle, ring and pinky fingers
* **Trigger:** bending of index finger

## Notes
* CSFML graphics module is built from [fork](https://github.com/SDraw/SFML/tree/2.5.x) to address [SteamVR OpenGL textures problems on AMD GPUs](https://github.com/ValveSoftware/openvr/issues/1246). However, it's highly possible to have no effect due to lack of testing hardware.
* If you see only green dots that represent tip of your index fingers, force `leap_control` to launch on dGPU through control panel of your GPU vendor.
* Tracking will be lost upon service shutdown/restart.
* Coordinate system used for offsets is right handed.  
![](https://learnopengl.com/img/getting-started/coordinate_systems_right_handed.png)

# Announcement
The biggest problem of this project is user usability, and it's awful, very awful.  
Attempts of making new utility application that will allow average user to configure all aspects of driver have failed.
* Unity: unable to force application to always run on dedicated GPUs
* Qt: unable to compile projects at all, even OpenVR's `helloworldoverlay`
* SFML: problem of OpenGL's internal textures type mismatch between SFML and SteamVR on AMD GPUs

If you know any framework or library that:
* Has own implemented UI elements
* Uses Direct3D/OpenGL for UI rendering
* Has offscreen rendering support
* C++ or C# support

Write it down in [this opened issue](https://github.com/SDraw/driver_leap/issues/2).
