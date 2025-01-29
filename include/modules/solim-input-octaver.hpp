#pragma once
#include <array>
#include "not-things.hpp"


struct SolimInputOctaverModule : NTModule, DrawListener {
	enum ParamId {
		ENUMS(PARAM_ADD_OCTAVE, 8),
		ENUMS(PARAM_SORT_POSITION, 8),
		ENUMS(PARAM_REPLACE_ORIGINAL, 8),
		NUM_PARAMS
	};
	enum InputId {
		ENUMS(IN_ADD_OCTAVE, 8),
		ENUMS(IN_SORT_POSITION, 8),
		ENUMS(IN_REPLACE_ORIGINAL, 8),
		NUM_INPUTS
	};
	enum OutputId {
		NUM_OUTPUTS
	};
	enum LightId {
		ENUMS(LIGHT_REPLACE_ORIGINAL, 8),
		LIGHT_CONNECTED,
		LIGHT_NOT_CONNECTED,
		NUM_LIGHTS
	};

	SolimInputOctaverModule();

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;
};

struct SolimInputOctaverWidget : NTModuleWidget {
	SolimInputOctaverWidget(SolimInputOctaverModule* module);
};
