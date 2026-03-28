#pragma once
#include "not-things.hpp"


struct RameligExpanderModule : NTModule {
	enum ParamId {
		NUM_PARAMS
	};
	enum InputId {
		IN_TRIG_JUMP,
		IN_TRIG_SHIFT,
		IN_GUIDE_CV,
		IN_GUIDE_GATE,
		NUM_INPUTS
	};
	enum OutputId {
		OUT_TRIG_JUMP,
		OUT_TRIG_SHIFT,
		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_TRIG_JUMP,
		LIGHT_TRIG_SHIFT,
		NUM_LIGHTS
	};

	RameligExpanderModule();

	void process(const ProcessArgs& args) override;
};

struct RameligExpanderWidget : NTModuleWidget {
	RameligExpanderWidget(RameligExpanderModule* module);
};