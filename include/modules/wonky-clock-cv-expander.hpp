#pragma once
#include <array>
#include "not-things.hpp"

struct LEDDisplay;

struct WonkyClockCVExpanderModule : NTModule {
	enum ParamId {
		NUM_PARAMS
	};
	enum InputId {
		IN_WIGGLE_AMOUNT,
		IN_WIGGLE_RATE,
		IN_WANDER_AMOUNT,
		IN_WANDER_RATE,
		IN_WOBBLE_AMOUNT,
		IN_WOBBLE_RATE,
		NUM_INPUTS
	};
	enum OutputId {
		NUM_OUTPUTS
	};
	enum LightId {
		NUM_LIGHTS
	};

	WonkyClockCVExpanderModule();
};

struct WonkyClockCVExpanderWidget : NTModuleWidget {
	WonkyClockCVExpanderWidget(WonkyClockCVExpanderModule* module);
};