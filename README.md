# Driver Leap [![Release](http://img.shields.io/github/release/SDraw/driver_leap.svg)](../../releases/latest)
Self-sustainable fork of SteamVR driver for Leap Motion controller with updated vendor libraries

[![](./.github/img_01.png)](https://www.youtube.com/watch?v=RdGnCV2g_oE)
  
## Installation (for users)
* Install [Ultraleap Gemini v5.3.1](https://developer.leapmotion.com/tracking-software-download)
* Extract [latest release archive](../../releases/latest) to `<SteamVR_folder>/drivers`
* Add line in section `steamvr` of `<Steam_folder>/config/steamvr.vrsettings` file:
```JSON
"activateMultipleDrivers": true,
```

## Usage
### Settings
Driver settings are configurated by editing `resources/settings.xml`. Available settings:
* `trackingLevel`: skeleton tracking style for OpenVR. Can be `partial` or `full`. `partial` by default.
* `handsReset`: marks controllers as out of range if hand for controller isn't detected by Leap Motion. `false` by default.
* `useVelocity`: enables velocity data from Leap Motion for hands. `false` by default.

### Gestures
List of hands gestures that correspond to controller original input:
* **Grip:** bending of middle, ring and pinky fingers
* **Trigger:** bending of index finger

## Notes
* Testing commits are currently pushed.
* If you see only green dots that represent tip of your index fingers, force `leap_control` to launch on dGPU through control panel of your GPU vendor.
  * Overlays aren't rendered on AMD GPUs due to [SteamVR internal bug](https://github.com/ValveSoftware/openvr/issues/1246).
* Tracking will be lost upon service shutdown/restart
