#pragma once
#include <array>
#include "not-things.hpp"

#include "core/timeseq-core.hpp"

struct TimeSeqDisplay;
struct LEDDisplay;

struct TimeSeqModule : NTModule, DrawListener, timeseq::PortHandler, timeseq::SampleRateReader, timeseq::EventListener, timeseq::AssertListener {
	enum ParamId {
		PARAM_RUN,
		PARAM_RESET,
		PARAM_RATE,
		PARAM_RESET_CLOCK,
		NUM_PARAMS
	};
	enum InputId {
		ENUMS(IN_INPUTS, 8),
		IN_RUN,
		IN_RESET,
		IN_RATE,
		NUM_INPUTS
	};
	enum OutputId {
		ENUMS(OUT_OUTPUTS, 8),
		OUT_RUN,
		OUT_RESET,
		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_RUN,
		LIGHT_RESET,
		LIGHT_LANE_LOOPED,
		LIGHT_SEGMENT_STARTED,
		LIGHT_TRIGGER_TRIGGERED,
		NUM_LIGHTS
	};
	enum TriggerId {
		TRIG_RUN,
		TRIG_RESET,
		TRIG_RESET_CLOCK,
		NUM_TRIGGERS
	};

	TimeSeqModule();
	~TimeSeqModule();

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;
	void onPortChange(const PortChangeEvent& e) override;
	void onSampleRateChange(const SampleRateChangeEvent& sampleRateChangeEvent) override;
	void onRemove(const RemoveEvent& e) override;

	float getInputPortVoltage(int index, int channel) override;
	float getOutputPortVoltage(int index, int channel) override;
	float getSampleRate() override;
	void setOutputPortVoltage(int index, int channel, float voltage) override;
	void setOutputPortChannels(int index, int channels) override;
	void setOutputPortLabel(int index, std::string& label) override;

	void laneLooped() override;
	void segmentStarted() override;
	void triggerTriggered() override;
	void scriptReset() override;

	void assertFailed(std::string& name, std::string& message, bool stop) override;


	std::shared_ptr<std::string> getScript();
	std::string loadScript(std::shared_ptr<std::string> script);
	void clearScript();
	std::list<std::string>& getLastScriptLoadErrors();

	std::vector<std::string>& getFailedAsserts();

	void setTimeSeqDisplay(TimeSeqDisplay* timeSeqDisplay);
	void setLEDDisplay(LEDDisplay* ledDisplay);

	private:
		TimeSeqDisplay* m_timeSeqDisplay = nullptr;
		LEDDisplay* m_ledDisplay = nullptr;

		timeseq::TimeSeqCore *m_timeSeqCore;
		std::shared_ptr<std::string> m_script;
		std::list<std::string> m_lastScriptLoadErrors;

		dsp::BooleanTrigger m_buttonTrigger[TriggerId::NUM_TRIGGERS];
		dsp::TSchmittTrigger<float> m_trigTriggers[TriggerId::NUM_TRIGGERS];
		dsp::PulseGenerator m_runPulse;
		dsp::PulseGenerator m_resetPulse;

		std::array<std::array<float, 16>, 8> m_outputVoltages;
		std::array<int, 8> m_outputChannels;

		bool m_laneLooped = false;
		bool m_segmentStarted = false;
		bool m_triggerTriggered = false;
		int m_rateDivision = 0;

		std::vector<int> m_changedPortChannelVoltages;
		dsp::ClockDivider m_portChannelChangeClockDivider;

		std::vector<std::string> m_failedAsserts;
		dsp::ClockDivider m_failedAssertBlinkClockDivider;

		// There was an error loading the latest script
		bool m_scriptError = false;

		void resetUi();
		void resetOutputs();
		void updateOutputs();
		void setDisplayScriptError(bool error);

		int getRate();
};

struct TimeSeqWidget : NTModuleWidget {
	TimeSeqWidget(TimeSeqModule* module);

	virtual void appendContextMenu(Menu* menu) override;
	virtual void onRemove(const RemoveEvent& e) override;

	private:
		void loadScript();
		void saveScript();
		void copyScript();
		void pasteScript();
		void clearScript();
		void copyLastLoadErrors();

		void copyAssertions();

		bool hasScript();
		bool hasFailedAsserts();
};