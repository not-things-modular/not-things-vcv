#pragma once
#include <array>
#include "not-things.hpp"


struct PolySameDiffModule : NTModule {
	enum ParamId {
		PARAM_DELTA,
		PARAM_MODE,
		NUM_PARAMS
	};
	enum InputId {
		IN_A,
		IN_B,
		NUM_INPUTS
	};
	enum OutputId {
		OUT_A,
		OUT_AB,
		OUT_B,
		NUM_OUTPUTS
	};
	enum LightId {
		NUM_LIGHTS
	};

	PolySameDiffModule();

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;

	void process(const ProcessArgs& args) override;

	bool getOutputDuplicates();
	void setOutputDuplicates(bool outputDuplicates);

	private:
		bool m_outputDuplicates = false;
		float m_floatBuffA[16];
		float m_floatBuffB[16];
	};

struct PolySameDiffWidget : NTModuleWidget {
	PolySameDiffWidget(PolySameDiffModule* module);

	virtual void appendContextMenu(Menu* menu) override;
	void switchOutputDuplicates();
};
