#pragma once
#include "not-things.hpp"

#include "core/ratrilig-core.hpp"


struct RatriligData;
struct RatriligState;
struct RatriligDisplay;


struct RatriligModule : NTModule {
	enum ParamId {
		PARAM_CLUSTER_SIZE,
		PARAM_GROUP_SIZE,
		PARAM_PHRASE_SIZE,

		PARAM_CLUSTER_CHANCE,
		PARAM_GROUP_CHANCE,
		PARAM_PHRASE_CHANCE,

		PARAM_DENSITY,
		PARAM_CLUSTER_DENSITY_FACTOR,
		PARAM_GROUP_DENSITY_FACTOR,
		PARAM_PHRASE_DENSITY_FACTOR,

		PARAM_CLUSTER_BIAS_AMOUNT,
		PARAM_CLUSTER_BIAS_DIRECTION,
		PARAM_GROUP_BIAS_AMOUNT,
		PARAM_GROUP_BIAS_DIRECTION,

		PARAM_TRIGGER,
		PARAM_RESET,

		NUM_PARAMS
	};
	enum InputId {
		IN_GATE,
		IN_RESET,

		IN_CLUSTER_CHANCE,
		IN_GROUP_CHANCE,
		IN_PHRASE_CHANCE,

		IN_DENSITY,
		IN_CLUSTER_DENSITY,
		IN_GROUP_DENSITY,
		IN_PHRASE_DENSITY,

		NUM_INPUTS
	};
	enum OutputId {
		OUT_GATE,

		NUM_OUTPUTS
	};
	enum LightId {
		NUM_LIGHTS
	};

	RatriligModule();

	void process(const ProcessArgs& args) override;

	void updatePolyphony(bool forceUpdateOutputs);

	void setRatriligDisplay(RatriligDisplay* ratriligDisplay);

	private:
		RatriligCore m_ratriligCore;
		RatriligDisplay* m_ratriligDisplay = nullptr;

		rack::dsp::TSchmittTrigger<float> m_inputTrigger[16];
		rack::dsp::BooleanTrigger m_buttonTrigger;

		rack::dsp::TSchmittTrigger<float> m_inputReset[16];
		rack::dsp::BooleanTrigger m_buttonReset;

		int m_channelCount;
};

struct RatriligWidget : NTModuleWidget {
	RatriligWidget(RatriligModule* module);
};