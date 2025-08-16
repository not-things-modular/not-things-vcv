#pragma once
#include "not-things.hpp"

#include "../util/valuemonitor.hpp"


struct RaphralicModule : NTModule {
	enum ParamId {
		PARAM_CLUSTER_SIZE,
		PARAM_PHRASE_SIZE,

		PARAM_CLUSTER_SLOTS,
		PARAM_PHRASE_SLOTS,

		PARAM_CLUSTER_CAPTURE_CHANCE,
		PARAM_PHRASE_CAPTURE_CHANCE,

		PARAM_CLUSTER_REPLAY_CHANCE,
		PARAM_PHRASE_REPLAY_CHANCE,

		PARAM_RESET_POS,
		PARAM_RESET_MEM,

		NUM_PARAMS
	};
	enum InputId {
		IN_CLOCK,
		IN_GATE,
		IN_CV,
		IN_SKIP,

		IN_RESET_POS,
		IN_RESET_MEM,

		NUM_INPUTS
	};
	enum OutputId {
		OUT_GATE,
		OUT_CV,
		OUT_REPLAY,

		NUM_OUTPUTS
	};
	enum LightId {
		NUM_LIGHTS
	};

	RaphralicModule();

	void process(const ProcessArgs& args) override;

	private:
		GateValueMonitor<1> m_gateMonitor;
		VoltageValueMonitor<> m_cvMonitor;
};

struct RaphralicWidget : NTModuleWidget {
	RaphralicWidget(RaphralicModule* module);
};