#include "modules/solim-output-octaver.hpp"
#include "components/ntport.hpp"


extern Model* modelSolimRandom;
extern Model* modelSolim;
extern Model *modelSolimOutput;

SolimOutputOctaverModule::SolimOutputOctaverModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configInput(IN_ADD_OCTAVE + i, string::f("Channel %d Add Octave CV", i + 1));
		configInput(IN_REPLACE_ORIGINAL + i, string::f("Channel %d Replace/Remove Original CV", i + 1));

		configSwitch(PARAM_ADD_OCTAVE + i, -1.f, 1.f, 0.f, string::f("Channel %d Add Octave", i + 1), {"Lower", "None", "Higher"});
		configSwitch(PARAM_REPLACE_ORIGINAL + i, 0.f, 1.f, 0.f, string::f("Channel %d Replace/Remove Original", i + 1), {"No", "Yes"});
	}

	configSwitch(PARAM_RESORT, 0.f, 1.f, 0.f, "Resort end result", {"No", "Yes"});
}

void SolimOutputOctaverModule::process(const ProcessArgs& args) {
}

void SolimOutputOctaverModule::draw(const widget::Widget::DrawArgs& args) {
	for (int i = 0; i < 8; i++) {
		bool lighted = false;
		if (inputs[IN_REPLACE_ORIGINAL + i].isConnected()) {
			lighted = inputs[IN_REPLACE_ORIGINAL + i].getVoltage() > 0.f;
		} else if (params[PARAM_REPLACE_ORIGINAL + i].getValue() > 0.f) {
			lighted = true;
		}
		lights[LIGHT_REPLACE_ORIGINAL + i].setBrightness(lighted);
	}

	lights[LIGHT_RESORT].setBrightness(params[PARAM_RESORT].getValue() > 0.1);

	int outputCount = 0;
	bool hasSolimModule = false;
	bool hasSolimRandom = false;
	Expander* expanderModule = &getLeftExpander();
	while ((expanderModule->module != nullptr) && (outputCount < 8)) {
		if (expanderModule->module->getModel() == modelSolim) {
			hasSolimModule = true;
			break;
		} else if (expanderModule->module->getModel() == modelSolimOutput) {
			outputCount++;
		} else if (expanderModule->module->getModel() == modelSolimRandom && !hasSolimRandom) {
			hasSolimRandom = true;
		} else {
			break;
		}

		expanderModule = &expanderModule->module->getLeftExpander();
	}

	if (!hasSolimModule) {
		lights[LIGHT_CONNECTED].setBrightness(0.f);
		lights[LIGHT_NOT_CONNECTED].setBrightness(1.f);
	} else {
		lights[LIGHT_CONNECTED].setBrightness(1.f);
		lights[LIGHT_NOT_CONNECTED].setBrightness(0.f);
	}
}

SolimOutputOctaverWidget::SolimOutputOctaverWidget(SolimOutputOctaverModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "solim-output-octaver") {
	float y = 54.5f;
	float yDelta = 40;
	for (int i = 0; i < 8; i++) {
		addParam(createParamCentered<BefacoSwitch>(Vec(39.5f, y), module, SolimOutputOctaverModule::PARAM_ADD_OCTAVE + i));
		addInput(createInputCentered<NTPort>(Vec(72.5f, y), module, SolimOutputOctaverModule::IN_ADD_OCTAVE + i));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(Vec(39.5f + 82.5f, y), module, SolimOutputOctaverModule::PARAM_REPLACE_ORIGINAL + i, SolimOutputOctaverModule::LIGHT_REPLACE_ORIGINAL + i));
		addInput(createInputCentered<NTPort>(Vec(72.5f + 82.5f, y), module, SolimOutputOctaverModule::IN_REPLACE_ORIGINAL + i));

		y += yDelta;
	}

	addParam(createLightParamCentered<VCVLightLatch<SmallSimpleLight<GreenLight>>>(Vec(175.f, 360.f), module, SolimOutputOctaverModule::PARAM_RESORT, SolimOutputOctaverModule::LIGHT_RESORT));

	addChild(createLightCentered<TinyLight<GreenRedLight>>(Vec(5.f, 20.f), module, SolimOutputOctaverModule::LIGHT_CONNECTED));
}


Model* modelSolimOutputOctaver = createModel<SolimOutputOctaverModule, SolimOutputOctaverWidget>("solim-output-octaver-expander");
