#pragma once
#include <array>
#include "not-things.hpp"


struct PipoOutputProcessingData {
	bool exhaustedInputs;
	bool exhaustedOutputs;

	Module *currentOutputModule;
	int outputIndex;
	int outputChannelCount;
	int outputChannelIndex;
	bool hasOutputChannelsAssigned;

	Module *currentInputModule;
	int inputIndex;
	int inputChannelCount;
	int inputChannelIndex;
};

struct PipoOutputModule : NTModule, DrawListener {
	enum ParamId {
		ENUMS(PARAM_OUTPUT_CHANNELS, 8),
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
		ENUMS(OUT_LIGHTS, 8*3),
		LIGHT_CONNECTED,
		LIGHT_NOT_CONNECTED,
		NUM_LIGHTS
	};

	PipoOutputModule();

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;

	private:
		PipoOutputProcessingData m_processingData;

		void resetProcessingData();
		bool moveToNextInput();
		bool moveToNextOutput();
};

struct PipoOutputWidget : NTModuleWidget {
	PipoOutputWidget(PipoOutputModule* module);
};