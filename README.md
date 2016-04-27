# Leap Motion Driver for SteamVR

## Note about WORK IN PROGRESS

You're seeing an early version of this software. I've got positional tracking established now as well as hand pose tracking. Some experimental mappings of hand gestures to triggers and buttons were added:

- Bending of the index finger maps to the trigger button, like you would fire a gun.
- clenching the middle, ring, pinky finger to a fist maps to the grabbing buttons
- the thumb inwards gesture (just point the thumb in the direction of your palm) clicks the trackpad

- a pinch gesture between index finger and thumb is detected, but currently not mapped to any button
  as there is too much crosstalk with the trigger finger.

### Games/Experiences that work well

- the Blu (all three stages)
- Irrational Exuberance: Prologue
- the Rose and I
 
### Games/Experiences that are starting but not quite playable yet.

- Audioshield: somehow the controllers are swapped? Control is tricky and not very precise. Semi-playable though.
- Tilt Brush & Final Approach: these start and you can start doing things, but there is lack of trackpad support in my driver.
- Brookhaven Experiment: tracking only works while SteamVR window is in focus. Why? Gun in right hand needs a 60 degree uptilt angle (define this in steamvr.vrsettings config file in Steam config folder). Trigger gesture detection is way to imprecise, you won't even survive the first wave of Zombies.
- The Lab: Lack of the Menu button prevents leaving any stages

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
