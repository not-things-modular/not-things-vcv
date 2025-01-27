#pragma once
#include <array>
#include "not-things.hpp"


struct SolimOutputModule : NTModule, DrawListener {
	enum ParamsIds {
		NUM_PARAMS
	};
	enum InputsIds {
		NUM_INPUTS
	};
	enum OutputsIds {
		ENUMS(OUT_OUTPUTS, 8),
		NUM_OUTPUTS
	};
	enum LightIds {
		OUT_POLYPHONIC_LIGHT,
		ENUMS(OUT_LIGHTS, 8),
		LIGHT_CONNECTED,
		LIGHT_NOT_CONNECTED,
		NUM_LIGHTS
	};

	SolimOutputModule();

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;

	SolimOutputMode getOutputMode();
	void setOutputMode(SolimOutputMode outputMode);

	private:
		SolimOutputMode m_outputMode = OUTPUT_MODE_MONOPHONIC;
};

struct SolimOutputWidget : NTModuleWidget {
	SolimOutputWidget(SolimOutputModule* module);

	virtual void appendContextMenu(Menu* menu) override;
	void switchOutputMode();
};