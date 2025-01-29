#include "modules/solim-input.hpp"
#include "components/ntport.hpp"


extern Model* modelSolimRandom;
extern Model* modelSolim;
extern Model *modelSolimInput;
extern Model *modelSolimInputOctaver;

SolimInputModule::SolimInputModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configInput(IN_INPUTS + i, string::f("Input %d", i + 1));
	}
}

void SolimInputModule::draw(const widget::Widget::DrawArgs& args) {
	int inputCount = 0;
	bool hasSolimModule = false;
	bool hasSolimRandom = false;
	bool hasSolimInputOctaver = false;
	Expander* expanderModule = &getRightExpander();
	while ((expanderModule->module != nullptr) && (inputCount < 7)) {
		if (expanderModule->module->getModel() == modelSolim) {
			hasSolimModule = true;
			break;
		} else if (expanderModule->module->getModel() == modelSolimInput) {
			inputCount++;
		} else if (expanderModule->module->getModel() == modelSolimInputOctaver && !hasSolimInputOctaver) {
			hasSolimInputOctaver = true;
		} else if (expanderModule->module->getModel() == modelSolimRandom && !hasSolimRandom) {
			hasSolimRandom = true;
		} else {
			break;
		}

		expanderModule = &expanderModule->module->getRightExpander();
	}

	if (!hasSolimModule) {
		lights[LIGHT_CONNECTED].setBrightness(0.f);
		lights[LIGHT_NOT_CONNECTED].setBrightness(1.f);
	} else {
		lights[LIGHT_CONNECTED].setBrightness(1.f);
		lights[LIGHT_NOT_CONNECTED].setBrightness(0.f);
	}
}

SolimInputWidget::SolimInputWidget(SolimInputModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "solim-input") {
	float xIn = 22.5;
	float y = 41.5;
	float yDelta = 40;
	for (int i = 0; i < 8; i++) {
		addInput(createInputCentered<NTPort>(Vec(xIn, y), module, SolimInputModule::IN_INPUTS + i));
		y += yDelta;
	}

	addChild(createLightCentered<TinyLight<GreenRedLight>>(Vec(40.f, 20.f), module, SolimInputModule::LIGHT_CONNECTED));
}


Model* modelSolimInput = createModel<SolimInputModule, SolimInputWidget>("solim-input-expander");