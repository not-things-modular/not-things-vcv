#include "modules/solim-input-octaver.hpp"
#include "components/ntport.hpp"
#include "components/dualbefacoswitch.hpp"


extern Model* modelSolimRandom;
extern Model* modelSolim;
extern Model *modelSolimInput;

SolimInputOctaverModule::SolimInputOctaverModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configInput(IN_ADD_OCTAVE + i, string::f("Channel %d Add Octave CV", i + 1));
		configInput(IN_SORT_POSITION + i, string::f("Channel %d Sort Position CV", i + 1));
		configInput(IN_REPLACE_ORIGINAL + i, string::f("Channel %d Replace/Remove Original CV", i + 1));

		configSwitch(PARAM_ADD_OCTAVE + i, -1.f, 1.f, 0.f, string::f("Channel %d Add Octave", i + 1), {"Lower", "None", "Higher"});
		configSwitch(PARAM_SORT_POSITION + i, 0.f, 1.f, 0.f, string::f("Channel %d Apply Octave", i + 1), {"Before Sort", "After Sort"});
		configSwitch(PARAM_REPLACE_ORIGINAL + i, 0.f, 1.f, 0.f, string::f("Channel %d Replace/Remove Original", i + 1), {"No", "Yes"});
	}
}

void SolimInputOctaverModule::process(const ProcessArgs& args) {
}

void SolimInputOctaverModule::draw(const widget::Widget::DrawArgs& args) {
	for (int i = 0; i < 8; i++) {
		bool lighted = false;
		if (inputs[IN_REPLACE_ORIGINAL + i].isConnected()) {
			lighted = inputs[IN_REPLACE_ORIGINAL + i].getVoltage() > 0.f;
		} else if (params[PARAM_REPLACE_ORIGINAL + i].getValue() > 0.f) {
			lighted = true;
		}
		lights[LIGHT_REPLACE_ORIGINAL + i].setBrightness(lighted);
	}

	int inputCount = 0;
	bool hasSolimModule = false;
	bool hasSolimRandom = false;
	Expander* expanderModule = &getRightExpander();
	while ((expanderModule->module != nullptr) && (inputCount < 8)) {
		if (expanderModule->module->getModel() == modelSolim) {
			hasSolimModule = true;
			break;
		} else if (expanderModule->module->getModel() == modelSolimInput) {
			inputCount++;
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

SolimInputOctaverWidget::SolimInputOctaverWidget(SolimInputOctaverModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "solim-input-octaver") {
	float y = 54.5f;
	float yDelta = 40;
	for (int i = 0; i < 8; i++) {
		addParam(createParamCentered<BefacoSwitch>(Vec(39.5f, y), module, SolimInputOctaverModule::PARAM_ADD_OCTAVE + i));
		addInput(createInputCentered<NTPort>(Vec(72.5f, y), module, SolimInputOctaverModule::IN_ADD_OCTAVE + i));

		addParam(createParamCentered<DualBefacoSwitch>(Vec(39.5f + 80.f, y), module, SolimInputOctaverModule::PARAM_SORT_POSITION + i));
		addInput(createInputCentered<NTPort>(Vec(72.5f + 80.f, y), module, SolimInputOctaverModule::IN_SORT_POSITION + i));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(Vec(39.5f + 160.f, y), module, SolimInputOctaverModule::PARAM_REPLACE_ORIGINAL + i, SolimInputOctaverModule::LIGHT_REPLACE_ORIGINAL + i));
		addInput(createInputCentered<NTPort>(Vec(72.5f + 160.f, y), module, SolimInputOctaverModule::IN_REPLACE_ORIGINAL + i));

		y += yDelta;
	}

	addChild(createLightCentered<TinyLight<GreenRedLight>>(Vec(265.f, 20.f), module, SolimInputOctaverModule::LIGHT_CONNECTED));
}


Model* modelSolimInputOctaver = createModel<SolimInputOctaverModule, SolimInputOctaverWidget>("solim-input-octaver-expander");
