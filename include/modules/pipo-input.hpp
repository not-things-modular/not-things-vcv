#pragma once
#include <array>
#include "not-things.hpp"

struct LEDDisplay;


struct PipoInputModule : NTModule, DrawListener {
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
	std::array<LEDDisplay*, 8> m_ledDisplays;

	PipoInputModule();

	void draw(const widget::Widget::DrawArgs& args) override;
};

struct PipoInputWidget : NTModuleWidget {
	PipoInputWidget(PipoInputModule* module);
};