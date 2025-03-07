#pragma once
#include <array>
#include "not-things.hpp"

#include "core/timeseq-core.hpp"


struct TimeSeqModule : NTModule, timeseq::PortReader, timeseq::PortWriter, timeseq::SampleRateReader {
	enum ParamId {
		PARAM_TRIG_RUN,
		PARAM_TRIG_RESET,
		NUM_PARAMS
	};
	enum InputId {
		ENUMS(IN_INPUTS, 8),
		IN_TRIG_RUN,
		IN_TRIG_RESET,
		NUM_INPUTS
	};
	enum OutputId {
		ENUMS(OUT_OUTPUTS, 8),
		OUT_RUN,
		OUT_RESET,
		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_TRIG_RUN,
		LIGHT_TRIG_RESET,
		NUM_LIGHTS
	};

	enum Limits {
		LIMIT_UPPER,
		LIMIT_LOWER
	};

	enum ProcessRate {
		DIVIDED,
		AUDIO,
		RATE_COUNT
	};

	TimeSeqModule();
	~TimeSeqModule();

	void process(const ProcessArgs& args) override;

	float getInputPortVoltage(int index, int channel) override;
	float getOutputPortVoltage(int index, int channel) override;
	float getSampleRate() override;
	void setOutputPortVoltage(int index, int channel, float voltage) override;
	void setOutputPortChannels(int index, int channels) override;

	std::shared_ptr<std::string> getScript();
	std::string loadScript(std::shared_ptr<std::string> script);
	std::list<std::string>& getLastScriptLoadErrors();

	private:
		timeseq::TimeSeqCore *m_timeSeqCore;
		std::shared_ptr<std::string> m_script;
		std::list<std::string> m_lastScriptLoadErrors;
};

struct TimeSeqWidget : NTModuleWidget {
	TimeSeqWidget(TimeSeqModule* module);

	virtual void appendContextMenu(Menu* menu) override;

	private:
		void loadScript();
		void saveScript();
		void copyLastLoadErrors();
};