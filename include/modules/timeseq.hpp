#pragma once
#include <array>
#include "not-things.hpp"

#include "core/timeseq-core.hpp"


struct TimeSeqModule : NTModule {
	enum ParamId {
		PARAM_LOWER_LIMIT,
		PARAM_UPPER_LIMIT,
		PARAM_SORT,
		NUM_PARAMS
	};
	enum InputId {
		ENUMS(IN_INPUTS, 8),
		IN_LOWER_LIMIT,
		IN_UPPER_LIMIT,
		IN_SORT,
		NUM_INPUTS
	};
	enum OutputId {
		ENUMS(OUT_OUTPUTS, 8),
		OUT_DEBUG_PROCESS_DURATION,
		NUM_OUTPUTS
	};
	enum LightId {
		OUT_POLYPHONIC_LIGHT,
		ENUMS(OUT_LIGHTS, 8),
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