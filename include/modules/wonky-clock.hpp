#pragma once
#include <array>
#include "not-things.hpp"

struct LEDDisplay;


struct WonkyClockModule : NTModule, DrawListener {
	enum ParamId {
		PARAM_BPM,
		PARAM_WOBBLE_AMOUNT,
		PARAM_WOBBLE_PROBABILITY,
		PARAM_WANDER_AMOUNT,
		PARAM_WANDER_PROBABILITY,
		PARAM_LINK,
		PARAM_WEIGHT,
		NUM_PARAMS
	};
	enum InputId {
		IN_BPM,
		IN_WOBBLE_AMOUNT,
		IN_WOBBLE_PROBABILITY,
		IN_WANDER_AMOUNT,
		IN_WANDER_PROBABILITY,
		NUM_INPUTS
	};
	enum OutputId {
		OUT_CLOCK,
		NUM_OUTPUTS
	};
	enum LightId {
		NUM_LIGHTS
	};

	LEDDisplay* m_bmpLed;

	WonkyClockModule();

	void draw(const widget::Widget::DrawArgs& args) override;

	private:
		int m_bpm;
};

struct WonkyClockWidget : NTModuleWidget {
	WonkyClockWidget(WonkyClockModule* module);
};