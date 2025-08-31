#pragma once
#include "not-things.hpp"

#include "core/ratrilig-core.hpp"
#include "ratrilig-expander.hpp"
#include <array>


struct RatriligData;
struct RatriligState;
struct RatriligProgress;
struct RatriligBias;
struct RatriligProbability;


struct RatriligModule : NTModule, DrawListener, RatriligCoreListener {
	enum ParamId {
		PARAM_CLUSTER_SIZE,
		PARAM_PHRASE_SIZE,
		PARAM_CYCLE_SIZE,

		PARAM_CLUSTER_CHANCE,
		PARAM_PHRASE_CHANCE,
		PARAM_CYCLE_CHANCE,

		PARAM_DENSITY,
		PARAM_CLUSTER_DENSITY_FACTOR,
		PARAM_PHRASE_DENSITY_FACTOR,
		PARAM_CYCLE_DENSITY_FACTOR,

		PARAM_CLUSTER_BIAS_AMOUNT,
		PARAM_CLUSTER_BIAS_DIRECTION,
		PARAM_PHRASE_BIAS_AMOUNT,
		PARAM_PHRASE_BIAS_DIRECTION,

		PARAM_TRIGGER,
		PARAM_RESET,

		NUM_PARAMS
	};
	enum InputId {
		IN_GATE,
		IN_RESET,

		IN_DENSITY,
		IN_CLUSTER_DENSITY,
		IN_PHRASE_DENSITY,
		IN_CYCLE_DENSITY,

		NUM_INPUTS
	};
	enum OutputId {
		OUT_GATE,

		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_TRIGGER,
		NUM_LIGHTS
	};

	RatriligModule();

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;
	void onExpanderChange(const ExpanderChangeEvent& e) override;

	void clusterStateChanged(int channel, bool enabled, float density, float bias) override;
	void phraseStateChanged(int channel, bool enabled, float density, float bias) override;
	void cycleStateChanged(int channel, bool enabled, float density) override;
	void valueChanged(int channel, int cycle, int phrase, int cluster, float target, float value, bool enabled) override;
	void clusterStarted(int channel) override;
	void phraseStarted(int channel) override;
	void cycleStarted(int channel) override;

	void setRatriligProgress(RatriligProgress* ratriligProgress);
	void setRatriligClusterBias(RatriligBias* ratriligBias);
	void setRatriligPhraseBias(RatriligBias* ratriligBias);
	void setRatriligClusterProbability(RatriligProbability* ratriligClusterProbability);
	void setRatriligPhraseProbability(RatriligProbability* ratriligPhraseProbability);
	void setRatriligCycleProbability(RatriligProbability* ratriligCycleProbability);
	void setRatriligGlobalProbability(RatriligProbability* ratriligGlobalProbability);

	private:
		RatriligProgress* m_ratriligProgress = nullptr;
		RatriligBias* m_ratriligClusterBias = nullptr;
		RatriligBias* m_ratriligPhraseBias = nullptr;
		RatriligProbability* m_ratriligClusterProbability = nullptr;
		RatriligProbability* m_ratriligPhraseProbability = nullptr;
		RatriligProbability* m_ratriligCycleProbability = nullptr;
		RatriligProbability* m_ratriligGlobalProbability = nullptr;

		RatriligCore m_ratriligCore;

		rack::dsp::TSchmittTrigger<float> m_inputTrigger[16];
		rack::dsp::BooleanTrigger m_buttonTrigger;
		dsp::ClockDivider m_triggerLightDivider;

		rack::dsp::TSchmittTrigger<float> m_inputReset[16];
		rack::dsp::BooleanTrigger m_buttonReset;

		int m_channelCount;

		std::array<bool, 16> m_clusterStarted;
		std::array<bool, 16> m_phraseStarted;
		std::array<bool, 16> m_cycleStarted;

		void updatePolyphony(bool forceUpdateOutputs);

		float getValue(ParamId paramId, InputId inputId, int channel);
		float getValue(ParamId paramId, Module* expander, RatriligExpanderModule::InputId inputId, int channel);

		RatriligExpanderModule* getRatriligExpander();
};

struct RatriligWidget : NTModuleWidget {
	RatriligWidget(RatriligModule* module);
};