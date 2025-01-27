#pragma once
#include <array>
#include "not-things.hpp"


struct SolimInputModule : NTModule, DrawListener {
	enum ParamsIds {
		NUM_PARAMS
	};
	enum InputsIds {
		ENUMS(IN_INPUTS, 8),
		NUM_INPUTS
	};
	enum OutputsIds {
		NUM_OUTPUTS
	};
	enum LightIds {
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