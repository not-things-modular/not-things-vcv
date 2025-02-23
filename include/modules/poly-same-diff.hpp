#pragma once
#include <array>
#include "not-things.hpp"


struct PolySameDiffModule : NTModule {
	enum ParamId {
		PARAM_DELTA,
		PARAM_MODE,
		NUM_PARAMS
	};
	enum InputId {
		IN_A,
		IN_B,
		NUM_INPUTS
	};
	enum OutputId {
		OUT_A,
		OUT_AB,
		OUT_B,
		NUM_OUTPUTS
	};
	enum LightId {
		NUM_LIGHTS
	};

	PolySameDiffModule();

	void process(const ProcessArgs& args) override;
};

struct PolySameDiffWidget : NTModuleWidget {
	PolySameDiffWidget(PolySameDiffModule* module);
};