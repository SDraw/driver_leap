# Leap Motion Driver for SteamVR

## Note about WORK IN PROGRESS

You're seeing an early version of this software. I've got positional tracking established now as well as hand pose tracking. Some experimental mappings of hand gestures to triggers and buttons were added:

- Bending of the index finger maps to the trigger button, like you would fire a gun.
- clenching the middle, ring, pinky finger to a fist maps to the grabbing buttons
- the thumbpress gesture (just point the thumb in the direction of your palm) touches and clicks the trackpad
- Flat hand held in front of you, palm towards face is used for application menu button
- Flat hand held in front of you, palm away from you is used for system menu button

I am working on allowing to freely map gestures to buttons in the steamvr.vrsettings config file in your Steam\config folder. However note that the "leap_gestures" section is currently not parsed yet. It's merely a sign of things to come.

        // Finger gestures (these would not throw your hand's orientation off much)
        TriggerFinger,           // bend your index finger as if pulling a trigger
        LowerFist,               // grab with your middle, ring, pinky fingers
        Pinch,                   // pinch with your thumb and index fingers
        Thumbpress,              // point the thumb towards the direction of your pinky

        // Hand gestures (these would significantly change the orientation of your hand)
        FlippingTheBird,         // flip someone off with your middle finger
        ILY,                     // pinky and index finger extended, middle and ring bent
        Victory,                 // V shape with your index, middle fingers, other fingers curled
        FlatHandPalmUp,          // flat hand, palm points upwards (relative to alignment of Leap!)
        FlatHandPalmDown,        // flat hand, palm points downwards (relative to alignment of Leap!)
        FlatHandPalmAway,        // flat hand, palm points away from self (relative to alignment of Leap!)
        FlatHandPalmTowards,     // flat hand, palm points towards self (relative to alignment of Leap!)
        ThumbUp,                 // thumb points up, remaining fingers form a fist
        ThumbInward,             // thumb points towards the left for the right hand and vice versa


## Supported gestures

There are other gestures detected currently, but not mapped to buttons. If you want to try these out, click on the application "gesture_checker.exe" in the directory C:\Program Files (x86)\SteamVR Leap Motion driver\leap\bin\Win32

Then I also recommend that you simultaneously bring up your Leap Motion's settings and from there start the diagnostic visualizer (the windowed version, not the VR one). Press "v" once to switch it to headmount optimized mode.

Now pull both windows side by side and bring a hand into view. The command prompt running the gesture_checker program should output a series of numbers next to the names of the gestures. A "1.0" means confidential detection, a "0.0" means no detection.

You can practise some gestures this way and also cross-check your pose in the Leap Motion diagnostic visualizer against the detection confidence.


### Games/Experiences that work mostly

- the Blu (all three stages)
- Irrational Exuberance: Prologue
- the Rose and I
- The Lab (some experiences work, others are tricky)
- Final Approach
- Audioshield: somehow the controllers are swapped? Control is tricky and not very precise. Semi-playable though.
  
### Games/Experiences that are starting but not quite playable yet.

- Tilt Brush: starts and you can start doing things, but there is lack of complete trackpad support in my driver.
- Brookhaven Experiment: tracking only works while SteamVR window is in focus. Why? Gun in right hand needs a 60 degree uptilt angle (define this in steamvr.vrsettings config file in Steam config folder). Trigger gesture detection is way to imprecise, you won't even survive the first wave of Zombies.


### Demos that won't work at all
- n/a


## Known Issues

I am seeing SteamVR Server crash on shutdown a lot. This could be related to my driver, but I have not yet found the root cause for the crash.

The Brookhaven experiment seems to steal focus from StreamVR, so that Steam does not get any position tracking. Clicking on the SteamVR window restores tracking, but mutes the audio on Brookhaven. Meh.

Some games work better when no grip angle is added to the controller pose, other games actually require a steep angle to be playable (Brookhaven, Audioshield). We may have to add a feature to chose the preferred default pose at runtime.

Tracking is not quite reliable to always detect my trigger gestures. I think we will have to integrate small handheld controllers like the Wiimote or the Playstation Move Navigation controller in the future.

I do not think I will be able to get animated hands into the 3D view, as the render model you can assign to each controller is mostly a static object. There are some JSON files to map joystick axes and triggers to animated parts of the displayed controller. But the fingers do not directly map to joystick axes directly and hence cannot be shown. Also not all games make use of SteamVR's internal controller visualization.


## Building

### Install Dependencies

1. Install SteamVR.  It is under "Tools" in everyone's Steam Library.  steam://install/250820
2. Install "Leap Motion Orion SDK V3.1.2".  https://developer.leapmotion.com/get-started
3. Fetch the OpenVR SDK 0.9.19 from https://github.com/ValveSoftware/openvr .

The solution and project files are for Visual Studio 2015.

### Configure Paths

Under "Property Manager" in Visual Studio, expand any of the configurations and find "Paths".  Right click and select "Properties" and then "User Macros".  Modify the paths to match your setup.  InstallDir will be created, and will be configured as an external driver path for SteamVR.

### Build

You will probably want to build Release x86.  You can also build x64.  The post-build step will install the binaries and copy the resources to the configured InstallDir and register that path with SteamVR.

## Using The Leap Motion Driver 

After building, the InstallDir should be a complete binary distribution.  To use it:

1. Register it with the SteamVR runtime via "vrpathreg adddriver $(InstallDir)".  This is done automatically by a Post-Build step, but if you copy the files elsewhere you will have to do it by hand.
2. Edit your config/steamvr.vrsettings to enable "activateMultipleDrivers".  This is what allows the hydra driver to co-exist with any HMD.  **Be sure to mind your commas.** Check vrserver.txt log to see if there were parse errors.  Many of the settings are described at https://developer.valvesoftware.com/wiki/SteamVR/steamvr.vrsettings .
```{
	...
	"steamvr" : {
        "activateMultipleDrivers" : true
   }
}```
3. If you are trying to use the Hydra driver without an HMD, you might want to enable driver_null (no HMD) or set "requireHmd": false.

After starting SteamVR, you should see controllers blinking in the status window until you move your hands into the field of view.

You can use "vrcmd" (with no arguments) to see a list of devices to verify things are working.
use "vrcmd" to verify things are loading:

```...
Driver leap : 2 displays
        leap (Serial number leap0_lefthand)
        leap (Serial number leap0_righthand)
...
```

You can also use "vrcmd --pollposes" (followed by an index number to limit the output) to see things are working.

## Licenses

The code in this distribution is distributed under the terms of the LICENSE file in the root directory.

The render models are based on work by Zoltan Erdokovy <zoltan.erdokovy@gmail.com> with permission.

The compiled driver and the install directory use the Leap Motion Orion SDK.  Use subject to the terms of the Leap Motion SDK Agreement available at
https://developer.leapmotion.com/sdk_agreement.
