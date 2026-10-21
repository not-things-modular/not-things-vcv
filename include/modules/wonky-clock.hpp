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

		PARAM_WIGGLE_AMOUNT,
		PARAM_WIGGLE_RATE,
		PARAM_WANDER_AMOUNT,
		PARAM_WANDER_RATE,
		PARAM_WOBBLE_AMOUNT,
		PARAM_WOBBLE_RATE,

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
	WonkyClockDisplay* m_wiggleDisplay = nullptr;
	WonkyClockDisplay* m_wanderDisplay = nullptr;
	WonkyClockDisplay* m_wobbleDisplay = nullptr;

	WonkyClockModule();

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;

	void onSampleRateChange(const SampleRateChangeEvent& e) override;
	void onReset(const ResetEvent& e) override;

	float getSampleRate() const override;
	void clockGateChanged(int index, bool high) override;
	void wiggleChanged(float wiggle, float max) override;
	void wanderChanged(float wander, float max) override;
	void wobbleChanged(float wobble, float max) override;

	wonky::WonkyWiggleMode getWiggleMode() const;
	void setWiggleMode(wonky::WonkyWiggleMode wiggleMode);
	bool getWanderRandom() const;
	void setWanderRandom(bool wanderRandom);

	private:
		bool m_running = true;
		int m_displayedBpm;
		std::unique_ptr<wonky::WonkyCore> m_core;

		// The stored right-click menu settings
		wonky::WonkyWiggleMode m_wiggleMode = wonky::WonkyWiggleMode::MODE_WALK;
		bool m_wanderRandom = true;
		

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

	virtual void appendContextMenu(Menu* menu) override;
	void setWiggleMode(wonky::WonkyWiggleMode wiggleMode);
	void setWanderRandom(bool wanderRandom);
};