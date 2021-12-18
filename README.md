# Driver Leap [![Release](http://img.shields.io/github/release/SDraw/driver_leap.svg)](../../releases/latest)
Fork with updated vendor libraries.
  
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
* `interpolation`: enables internal Leap Motion data capture interpolation. `false` by default.
* `useVelocity`: enables velocity data from Leap Motion for hands. `false` by default.

### Gestures
List of hands gestures that correspond to controller original input:
* **Grip:** bending of middle, ring and pinky fingers
* **Trigger:** bending of index finger.

### Notes
* Testing commits are currently pushed.
