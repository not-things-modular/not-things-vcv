#pragma once
#include <array>
#include "not-things.hpp"
#include "core/wonky-core.hpp"

struct LEDDisplay;
struct WonkyClockDisplay;

struct WonkyClockCVExpanderModule;
struct WonkyClockOutputExpanderModule;

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

		PARAM_MODE,
		PARAM_LINK,

		NUM_PARAMS
	};
	enum InputId {
		IN_RUN,
		IN_RESET,
		NUM_INPUTS
	};
	enum OutputId {
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

	LEDDisplay* m_bpmLed;
	WonkyClockDisplay* m_wanderDisplay = nullptr;
	WonkyClockDisplay* m_waverDisplay = nullptr;
	WonkyClockDisplay* m_wobbleDisplay = nullptr;

	WonkyClockModule();

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;

	void onSampleRateChange(const SampleRateChangeEvent& e) override;

	float getSampleRate() const override;
	void clockGateChanged(int index, bool high) override;
	void wanderChanged(float wander, float max) override;
	void waverChanged(float waver, float max) override;
	void wobbleChanged(float wobble, float max) override;

	private:
		bool m_running = true;
		int m_displayedBpm;
		std::unique_ptr<wonky::WonkyCore> m_core;

		// Flag to indicate if we're currently inside a process invocation.
		// Process should only be called single-threaded by VCV, so keeping a simple boolean should be safe
		bool m_processing = false;

		dsp::BooleanTrigger m_buttonTrigger[TriggerId::NUM_TRIGGERS];
		dsp::TSchmittTrigger<float> m_trigTriggers[TriggerId::NUM_TRIGGERS];

		dsp::PulseGenerator m_runPulse;
		dsp::PulseGenerator m_resetPulse;

		WonkyClockCVExpanderModule* m_cvExpander;
		WonkyClockOutputExpanderModule* m_outputExpander;

		WonkyClockCVExpanderModule* getCVExpander();
		WonkyClockOutputExpanderModule* getOutputExpander();
		void updateExpanders();
};

struct WonkyClockWidget : NTModuleWidget {
	WonkyClockWidget(WonkyClockModule* module);
};