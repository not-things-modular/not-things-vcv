#pragma once
#include <array>
#include "not-things.hpp"

struct LEDDisplay;


struct WonkyClockModule : NTModule, DrawListener {
	enum ParamId {
		PARAM_BPM,
		PARAM_RUN,
		PARAM_RESET,

		PARAM_WOBBLE_AMOUNT,
		PARAM_WOBBLE_PROBABILITY,
		PARAM_WAVER_AMOUNT,
		PARAM_WAVER_PROBABILITY,
		PARAM_WANDER_AMOUNT,
		PARAM_WANDER_RATE,

		PARAM_LINK,
		PARAM_WEIGHT,

		NUM_PARAMS
	};
	enum InputId {
		IN_BPM,
		IN_RUN,
		IN_RESET,
		NUM_INPUTS
	};
	enum OutputId {
		OUT_RUN,
		OUT_RESET,
		OUT_CLOCK,
		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_RUN,
		LIGHT_RESET,
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