#pragma once
#include <array>
#include "not-things.hpp"


struct SolimOutputOctaverModule : NTModule, DrawListener {
	enum ParamId {
		ENUMS(PARAM_ADD_OCTAVE, 8),
		ENUMS(PARAM_REPLACE_ORIGINAL, 8),
		PARAM_RESORT,
		NUM_PARAMS
	};
	enum InputId {
		ENUMS(IN_ADD_OCTAVE, 8),
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
		LIGHT_RESORT,
		LIGHT_DONT_RESORT,
		NUM_LIGHTS
	};

	SolimOutputOctaverModule();

	void draw(const widget::Widget::DrawArgs& args) override;

};

struct SolimOutputOctaverWidget : NTModuleWidget {
	SolimOutputOctaverWidget(SolimOutputOctaverModule* module);
};