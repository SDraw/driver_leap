#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include "openvr_driver.h"
#include "LeapC++.h"

#define GLM_FORCE_MESSAGES
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_access.hpp"

#include "pugixml.hpp"
