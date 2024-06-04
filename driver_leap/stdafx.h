#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <chrono>
#include <numeric>
#include <atomic>
#include <mutex>
#include <thread>
#include <array>

#include "openvr_driver.h"
#include "LeapC.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtx/intersect.hpp"

#include "pugixml.hpp"
#include "JoyShockLibrary.h"
