#pragma once
#include <array>
#include "not-things.hpp"


struct SolimOutputModule : NTModule, DrawListener {
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
		OUT_POLYPHONIC_LIGHT,
		ENUMS(OUT_LIGHTS, 8),
		LIGHT_CONNECTED,
		LIGHT_NOT_CONNECTED,
		NUM_LIGHTS
	};

	SolimOutputModule();

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;

	void draw(const widget::Widget::DrawArgs& args) override;

	SolimOutputMode getOutputMode();
	void setOutputMode(SolimOutputMode outputMode);

	private:
		SolimOutputMode m_outputMode = OUTPUT_MODE_MONOPHONIC;
		std::array<bool, 8> m_portConnected = { false };
};

struct SolimOutputWidget : NTModuleWidget {
	SolimOutputWidget(SolimOutputModule* module);

	virtual void appendContextMenu(Menu* menu) override;
	void switchOutputMode();
};