#pragma once
#include <array>
#include "not-things.hpp"


struct SolimInputModule : NTModule, DrawListener {
	enum ParamId {
		NUM_PARAMS
	};
	enum InputId {
		ENUMS(IN_INPUTS, 8),
		NUM_INPUTS
	};
	enum OutputId {
		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_CONNECTED,
		LIGHT_NOT_CONNECTED,
		NUM_LIGHTS
	};

	SolimInputModule();

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;
};

struct SolimInputWidget : NTModuleWidget {
	SolimInputWidget(SolimInputModule* module);
};