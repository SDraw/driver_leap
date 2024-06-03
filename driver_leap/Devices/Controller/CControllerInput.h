#pragma once

#include <stdint.h>
#include <map>

#include "Devices/Controller/CLeapIndexController.h"

// NOTE: these are directly mapped to JoyShockLibrary types (refer: https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShockLibrary.h)
enum ControllerType : uint8_t {
	CONTROLLER_JOYCON_LEFT = 1,
	CONTROLLER_JOYCON_RIGHT = 2,
	CONTROLLER_JOYCON_DS4 = 4
};

class CControllerInput {
	struct Device {
		bool connected{ false };
		int handle{ -1 };
	};

	int32_t m_deviceCount;
	std::map<uint8_t, Device> m_devices;
public:
	explicit CControllerInput();
	~CControllerInput();

	bool IsConnected();
	void Update(CLeapIndexController* left, CLeapIndexController* right);
};