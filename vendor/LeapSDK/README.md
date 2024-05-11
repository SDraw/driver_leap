# Ultraleap SDK

--------------------------------------------------------------------------------

## Package contents:

LeapSDK
- include
	* API headers.
- lib
	* dynamic API library and CMake scripts.
- samples
	* Various samples demonstrating several different usages.
- LICENSE.md
    * Ultraleap Tracking SDK license.
- README.md
    * Ultraleap Tracking SDK readme.
- ThirdPartyNotices.md
    * Ultraleap Tracking SDK third party licenses.

## Requirements:

1. Running requires
    * Ultraleap Tracking Software https://developer.leapmotion.com/get-started/

2. Building Samples requires
    * CMake 3.16.3+ (https://cmake.org/)
    * Microsoft Visual Studio 15+ (Windows)
    * GCC (Linux - tested on v9.4.0)

## Installation:

The LeapSDK is installed with Ultraleap Tracking.

## Usage:

1. For CMake projects
    * Ensure LeapSDK is in a directory considered as a prefix by find_package.
        (https://cmake.org/cmake/help/v3.16/command/find_package.html)
        * Or : directly set LeapSDK_DIR to <install_dir>/LeapSDK/lib/cmake/LeapSDK
        * Or : Pass the LeapSDK's path to find_package with the PATHS option.
    * call find_package(LeapSDK 5 [PATHS ...]).
    * call target_link_libraries(<your project> PUBLIC|PRIVATE LeapSDK::LeapC).
    * Ensure LeapC.dll/LeapC.so is in your dynamic library search path.
        * A popular option is to add a post-build step that copies it to your project's output directory.

2. For non-CMake projects
    * Use a C/C++ compiler such as MSVC, Clang or GCC.
    * Add LeapSDK/include to the compiler include search paths.
    * Either add a linker reference to LeapC.lib or dynamically load LeapC.dll/LeapC.so.

## Building Samples:

### Windows

1. Open CMake using LeapSDK/samples as the source directory

2. Select a build directory (often LeapSDK/samples/build) to use

3. Configure & Generate CMake with the generator of your choice

4. Open and build the CMake generated project files. For more help, see the CMake documentation.
    * An example script would be :
```powershell
$env:BUILD_TYPE = 'Release'
$env:REPOS_BUILD_ROOT = 'C:/build'
$env:REPOS_INSTALL_ROOT = 'C:/Program Files'

cmake -S "C:/Program Files/Ultraleap/LeapSDK/samples" -B $env:REPOS_BUILD_ROOT/$env:BUILD_TYPE/LeapSDK/leapc_example `
		-DCMAKE_INSTALL_PREFIX="$env:REPOS_INSTALL_ROOT/leapc_example" `
		-DCMAKE_BUILD_TYPE="$env:BUILD_TYPE"

cmake --build $env:REPOS_BUILD_ROOT/$env:BUILD_TYPE/LeapSDK/leapc_example -j --config $env:BUILD_TYPE
```

### x64 Linux

1. Open CMake using /usr/share/doc/ultraleap-hand-tracking-service/samples as the source directory    
    
2. Select a build directory (eg. ~/ultraleap-tracking-samples/build) to use    
    
3. Configure & Generate CMake with the generator of your choice

4. Open and build the CMake generated project files. For more help, see the CMake documentation.
    * An example script would be :    
```bash    
SRC_DIR=/usr/share/doc/ultraleap-hand-tracking-service/samples
BUILD_TYPE='Release'
REPOS_BUILD_ROOT=~/ultraleap-tracking-samples/build
REPOS_INSTALL_ROOT=/usr/bin/ultraleap-tracking-samples
cmake -S ${SRC_DIR} -B ${REPOS_BUILD_ROOT}/${BUILD_TYPE}/LeapSDK/leapc_example `       
        -DCMAKE_INSTALL_PREFIX="${REPOS_INSTALL_ROOT}/leapc_example" `    
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"    
    
cmake --build ${REPOS_BUILD_ROOT}/${BUILD_TYPE}/LeapSDK/leapc_example -j --config ${BUILD_TYPE}
``` 

### ARM64 Linux

1. Open the top level directory of the untarred release and select a build directory (eg. ~/ultraleap-tracking-samples/build) to use

2. Set the CMAKE_PREFIX_PATH directory to the absolute location of LeapSDK

3. Configure & Generate CMake with the generator of your choice

4. Open and build the CMake generated project files. For more help, see the CMake documentation.
    * An example script would be :    
```bash
SRC_DIR='LeapSDK/samples'
BUILD_TYPE='Release'
REPOS_BUILD_ROOT=~/ultraleap-tracking-samples/build
REPOS_INSTALL_ROOT=~/ultraleap-tracking-samples/bin
PREFIX_PATH=$(pwd)/LeapSDK

cmake -S ${SRC_DIR} -B ${REPOS_BUILD_ROOT}/${BUILD_TYPE}/LeapSDK/leapc_example `
-DCMAKE_INSTALL_PREFIX="${REPOS_INSTALL_ROOT}/leapc_example"`
-DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DCMAKE_PREFIX_PATH="${PREFIX_PATH}"
cmake --build ${REPOS_BUILD_ROOT}/${BUILD_TYPE}/LeapSDK/leapc_example -j --config ${BUILD_TYPE}
``` 

### MacOS

1. Open CMake using /Applications/Ultraleap\ Hand\ Tracking.app/Contents/LeapSDK/samples as the source directory

2. Select a build directory (eg. ~/ultraleap-tracking-samples/build) to use

3. Configure & Generate CMake with the generator of your choice

4. Open and build the CMake generated project files. For more help, see the CMake documentation.
    * An example script would be :
```bash    
SRC_DIR='/Applications/Ultraleap\ Hand\ Tracking.app/Contents/LeapSDK/samples'
BUILD_TYPE='Release'
REPOS_BUILD_ROOT=~/ultraleap-tracking-samples/build
REPOS_INSTALL_ROOT=~/ultraleap-tracking-samples/bin

cmake -S ${SRC_DIR} -B ${REPOS_BUILD_ROOT}/${BUILD_TYPE}/LeapSDK/leapc_example `
-DCMAKE_INSTALL_PREFIX="${REPOS_INSTALL_ROOT}/leapc_example" `
-DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

cmake --build ${REPOS_BUILD_ROOT}/${BUILD_TYPE}/LeapSDK/leapc_example -j --config ${BUILD_TYPE}   
``` 

## Resources:

1. Ultraleap For Developers Site (https://developer.leapmotion.com)
     provides examples, community forums, Ultraleap news, and documentation
     to help you to learn how to develop applications using the Ultraleap Tracking
     SDK.

2. C# and Unity bindings (https://github.com/leapmotion/UnityModules)

3. C++ bindings matching the old API (https://github.com/leapmotion/LeapCxx)

--------------------------------------------------------------------------------

Copyright © 2012-2020 Ultraleap Ltd. All rights reserved.

Use subject to the terms of the Ultraleap Tracking SDK Agreement `LICENSE.md` next to this `README.md` file.
