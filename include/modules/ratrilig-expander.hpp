#pragma once
#include <array>
#include "not-things.hpp"


struct RatriligExpanderModule : NTModule {
	enum ParamId {
		NUM_PARAMS
	};
	enum InputId {
		IN_SKIP_CLUSTER,
		IN_SKIP_PHRASE,
		IN_SKIP_CYCLE,
		NUM_INPUTS
	};
	enum OutputId {
		OUT_TRIG_CLUSTER,
		OUT_TRIG_PHRASE,
		OUT_TRIG_CYCLE,
		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_TRIG_CLUSTER,
		LIGHT_TRIG_PHRASE,
		LIGHT_TRIG_CYCLE,
		NUM_LIGHTS
	};

	RatriligExpanderModule();

	void process(const ProcessArgs& args) override;

	void triggerCluster(int channel);
	void triggerPhrase(int channel);
	void triggerCycle(int channel);

	private:
		rack::dsp::PulseGenerator m_clusterPulse[16];
		rack::dsp::PulseGenerator m_phrasePulse[16];
		rack::dsp::PulseGenerator m_cyclePulse[16];
};

struct RatriligExpanderWidget : NTModuleWidget {
	RatriligExpanderWidget(RatriligExpanderModule* module);
};