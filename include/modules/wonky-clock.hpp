#pragma once
#include <array>
#include "not-things.hpp"
#include "core/wonky-core.hpp"

struct LEDDisplay;

struct WonkyClockModule : NTModule, DrawListener, wonky::SampleRateReader, wonky::WonkyListener {
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
	enum TriggerId {
		TRIG_RUN,
		TRIG_RESET,
		NUM_TRIGGERS
	};

	LEDDisplay* m_bmpLed;

	WonkyClockModule();

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;

	float getSampleRate() const override;
	void clockGateChanged(bool high) override;

	private:
		int m_displayedBpm;
		std::unique_ptr<wonky::WonkyCore> m_core;

		dsp::BooleanTrigger m_buttonTrigger[TriggerId::NUM_TRIGGERS];
		dsp::TSchmittTrigger<float> m_trigTriggers[TriggerId::NUM_TRIGGERS];
	};

struct WonkyClockWidget : NTModuleWidget {
	WonkyClockWidget(WonkyClockModule* module);
};