# Leap Motion Driver for SteamVR

## Note about WORK IN PROGRESS

You're seeing an early version of this software. I've got positional tracking established now
as well as hand pose tracking (not including fingers).

I stil have to implement emulation of buttons and joypad axes using gestures.

I do not think I will be able to get animated hands into the 3D view, as the render model you
can give to each controller seems to be a static object. At best, I can swap out objects
if a specific gesture is recognized. Adding separate controllers for each finger seems
infeasible.

## Building

### Install Dependencies

1. Install SteamVR.  It is under "Tools" in everyone's Steam Library.  steam://install/250820
2. Install "Leap Motion Orion SDK V3.1.1".  https://developer.leapmotion.com/get-started
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
        leap (Serial number leap0_controller0)
        leap (Serial number leap0_controller1)
...
```

You can also use "vrcmd --pollposes" (followed by an index number to limit the output) to see things are working.

## Licenses

The code in this distribution is distributed under the terms of the LICENSE file in the root directory.

The render models are based on work by Zoltan Erdokovy <zoltan.erdokovy@gmail.com> with permission.

The compiled driver and the install directory use the Leap Motion Orion SDK.  Use subject to the terms of the Leap Motion SDK Agreement available at
https://developer.leapmotion.com/sdk_agreement.
