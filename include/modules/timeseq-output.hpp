#pragma once
#include <array>
#include "not-things.hpp"

struct LEDDisplay;


struct TimeSeqOutputModule : NTModule, DrawListener {
	enum ParamId {
		NUM_PARAMS
	};
	enum InputId {
		NUM_INPUTS
	};
	enum OutputId {
		ENUMS(OUT_OUTPUTS, 8),
		NUM_OUTPUTS
	};
	enum LightId {
		NUM_LIGHTS
	};
	std::array<LEDDisplay*, 8> m_ledDisplays;

	TimeSeqOutputModule();

	void draw(const widget::Widget::DrawArgs& args) override;
	void onPortChange(const PortChangeEvent& e) override;

	private:
		int m_lastOffset = -2;
};

struct TimeSeqOutputWidget : NTModuleWidget {
	TimeSeqOutputWidget(TimeSeqOutputModule* module);
};