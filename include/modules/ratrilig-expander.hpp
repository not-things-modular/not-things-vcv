#pragma once
#include <array>
#include "not-things.hpp"


struct RatriligExpanderModule : NTModule {
	enum ParamId {
		NUM_PARAMS
	};
	enum InputId {
		IN_SKIP_CLUSTER,
		IN_SKIP_GROUP,
		IN_SKIP_PHRASE,
		NUM_INPUTS
	};
	enum OutputId {
		OUT_TRIG_CLUSTER,
		OUT_TRIG_GROUP,
		OUT_TRIG_PHRASE,
		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_TRIG_CLUSTER,
		LIGHT_TRIG_GROUP,
		LIGHT_TRIG_PHRASE,
		NUM_LIGHTS
	};

	RatriligExpanderModule();

	void process(const ProcessArgs& args) override;

	void triggerCluster(int channel);
	void triggerGroup(int channel);
	void triggerPhrase(int channel);

	private:
		rack::dsp::PulseGenerator m_clusterPulse[16];
		rack::dsp::PulseGenerator m_groupPulse[16];
		rack::dsp::PulseGenerator m_phrasePulse[16];
};

struct RatriligExpanderWidget : NTModuleWidget {
	RatriligExpanderWidget(RatriligExpanderModule* module);
};