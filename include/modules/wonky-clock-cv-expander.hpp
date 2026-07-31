#pragma once
#include <array>
#include "not-things.hpp"

struct LEDDisplay;

struct WonkyClockCVExpanderModule : NTModule, DrawListener {
	enum ParamId {
		NUM_PARAMS
	};
	enum InputId {
		IN_BPM,
		IN_WOBBLE_AMOUNT,
		IN_WOBBLE_PROBABILITY,
		IN_WAVER_AMOUNT,
		IN_WAVER_PROBABILITY,
		IN_WANDER_AMOUNT,
		IN_WANDER_RATE,
		NUM_INPUTS
	};
	enum OutputId {
		OUT_RUN,
		OUT_RESET,
		NUM_OUTPUTS
	};
	enum LightId {
		NUM_LIGHTS
	};

	LEDDisplay* m_bpmLed = nullptr;

	WonkyClockCVExpanderModule();

	void setCurrentBpm(float currentBpm);

	void draw(const widget::Widget::DrawArgs& args) override;
	void onExpanderChange(const ExpanderChangeEvent& changeEvent) override;

	private:
		int m_displayedBpm;
		int m_currentBpm;
	};

struct WonkyClockCVExpanderWidget : NTModuleWidget {
	WonkyClockCVExpanderWidget(WonkyClockCVExpanderModule* module);
};