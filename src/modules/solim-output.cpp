#include "modules/solim-output.hpp"
#include "components/ntport.hpp"
#include "components/bluegreenlight.hpp"


extern Model* modelSolimRandom;
extern Model* modelSolim;
extern Model* modelSolimOutput;
extern Model* modelSolimOutputOctaver;

SolimOutputModule::SolimOutputModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configOutput(OUT_OUTPUTS + i, string::f("Output %d", i + 1));
	}
}

json_t *SolimOutputModule::dataToJson() {
	json_t *rootJ = NTModule::dataToJson();
	json_object_set_new(rootJ, "ntSolimOutputMode", json_integer(m_outputMode));
	return rootJ;
}

void SolimOutputModule::dataFromJson(json_t *rootJ) {
	NTModule::dataFromJson(rootJ);

	json_t *ntSolimOutputModeJson = json_object_get(rootJ, "ntSolimOutputMode");
	if (ntSolimOutputModeJson) {
		json_int_t outputModeNumber = json_integer_value(ntSolimOutputModeJson);
		if (outputModeNumber > 0 && outputModeNumber < SolimOutputMode::NUM_OUTPUT_MODES) {
			setOutputMode(static_cast<SolimOutputMode>(outputModeNumber));
		} else {
			setOutputMode(SolimOutputMode::OUTPUT_MODE_MONOPHONIC);
		}
	}
}

void SolimOutputModule::draw(const widget::Widget::DrawArgs& args) {
	int counter = 0;
	int outputCount = 0;
	bool hasSolimModule = false;
	bool hasSolimRandom = false;
	bool hasOutputOctaver = false;
	Expander* expanderModule = &getLeftExpander();
	while ((expanderModule->module != nullptr) && (outputCount < 7)) {
		if (expanderModule->module->getModel() == modelSolim) {
			hasSolimModule = true;
			break;
		} else if (expanderModule->module->getModel() == modelSolimOutput) {
			outputCount++;
		} else if (expanderModule->module->getModel() == modelSolimOutputOctaver && !hasOutputOctaver) {
			hasOutputOctaver = true;
		} else if (expanderModule->module->getModel() == modelSolimRandom && !hasSolimRandom) {
			hasSolimRandom = true;
		} else {
			break;
		}

		expanderModule = &expanderModule->module->getLeftExpander();
		counter++;
	}

	if (!hasSolimModule) {
		for (int i = 0; i < 8; i++) {
			lights[OUT_LIGHTS + i].setBrightness(0.f);
		}
		lights[LIGHT_CONNECTED].setBrightness(0.f);
		lights[LIGHT_NOT_CONNECTED].setBrightness(1.f);
	} else {
		lights[LIGHT_CONNECTED].setBrightness(1.f);
		lights[LIGHT_NOT_CONNECTED].setBrightness(0.f);
	}
}

void SolimOutputModule::onPortChange(const PortChangeEvent& event) {
	for (int i = 0; i < 8; i++) {
		m_connectedPorts[i] = outputs[i].isConnected();
	}
}

SolimOutputMode SolimOutputModule::getOutputMode() {
	return m_outputMode;
}

void SolimOutputModule::setOutputMode(SolimOutputMode outputMode) {
	m_outputMode = outputMode;
}

std::array<bool, 8>& SolimOutputModule::getConnectedPorts() {
	return m_connectedPorts;
}


SolimOutputWidget::SolimOutputWidget(SolimOutputModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "solim-output") {
	float xOut = 22.5;
	float y = 41.5;
	float yDelta = 40;
	for (int i = 0; i < 8; i++) {
		addOutput(createOutputCentered<NTPort>(Vec(xOut, y), module, SolimOutputModule::OUT_OUTPUTS + i));
		if (i == 0) {
			addChild(createLightCentered<TinyLight<BlueGreenLight>>(Vec(xOut + 12.5, y + 12.5), module, SolimOutputModule::OUT_POLYPHONIC_LIGHT));
		} else {
			addChild(createLightCentered<TinyLight<GreenLight>>(Vec(xOut + 12.5, y + 12.5), module, SolimOutputModule::OUT_LIGHTS + i));
		}
		y += yDelta;
	}

	addChild(createLightCentered<TinyLight<GreenRedLight>>(Vec(5.f, 20.f), module, SolimOutputModule::LIGHT_CONNECTED));
}

void SolimOutputWidget::appendContextMenu(Menu* menu) {
	NTModuleWidget::appendContextMenu(menu);

	SolimOutputMode outputMode = getModule() ? dynamic_cast<SolimOutputModule *>(getModule())->getOutputMode() : SolimOutputMode::OUTPUT_MODE_MONOPHONIC;
	menu->addChild(createCheckMenuItem("Polyphonic output", "", [outputMode]() { return outputMode == SolimOutputMode::OUTPUT_MODE_POLYPHONIC; }, [this]() { switchOutputMode(); }));
}

void SolimOutputWidget::switchOutputMode() {
	SolimOutputModule* solimOutputModule = dynamic_cast<SolimOutputModule *>(getModule());
	if (solimOutputModule != nullptr) {
		if (solimOutputModule->getOutputMode() == SolimOutputMode::OUTPUT_MODE_POLYPHONIC) {
			solimOutputModule->setOutputMode(SolimOutputMode::OUTPUT_MODE_MONOPHONIC);
		} else {
			solimOutputModule->setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);
		}
	}
}


Model* modelSolimOutput = createModel<SolimOutputModule, SolimOutputWidget>("solim-output-expander");