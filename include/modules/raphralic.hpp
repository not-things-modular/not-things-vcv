#pragma once
#include "not-things.hpp"


struct RaphralicModule : NTModule {
	enum ParamId {
		NUM_PARAMS
	};
	enum InputId {
		NUM_INPUTS
	};
	enum OutputId {
		NUM_OUTPUTS
	};
	enum LightId {
		NUM_LIGHTS
	};

	RaphralicModule();

	void process(const ProcessArgs& args) override;

	private:
};

struct RaphralicWidget : NTModuleWidget {
	RaphralicWidget(RaphralicModule* module);
};