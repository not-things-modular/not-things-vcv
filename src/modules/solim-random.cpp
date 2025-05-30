#include "modules/solim-random.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"


extern Model* modelSolimRandom;
extern Model* modelSolim;
extern Model* modelSolimInput;
extern Model* modelSolimInputOctaver;
extern Model* modelSolimOutput;
extern Model* modelSolimOutputOctaver;

SolimRandomModule::SolimRandomModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configInput(INPUT_TRIG_MOVE, "Move One Up or Down");
	configInput(INPUT_TRIG_ONE, "Switch Two at Random");
	configInput(INPUT_TRIG_ALL, "Move All to Random Positions");
	configInput(INPUT_TRIG_RESET, "Reset All");

	configButton(PARAM_TRIG_MOVE, "Move One Up or Down");
	configButton(PARAM_TRIG_ONE, "Switch Two at Random");
	configButton(PARAM_TRIG_ALL, "Move All to Random Positions");
	configButton(PARAM_TRIG_RESET, "Reset All");
}

void SolimRandomModule::process(const ProcessArgs& args) {
	lights[LIGHT_TRIG_MOVE].setBrightnessSmooth(
		processTriggers(ParamId::PARAM_TRIG_MOVE, InputId::INPUT_TRIG_MOVE, TriggerId::TRIG_MOVE, m_moveCounters),
		args.sampleTime);
	lights[PARAM_TRIG_ONE].setBrightnessSmooth(
		processTriggers(ParamId::PARAM_TRIG_ONE, InputId::INPUT_TRIG_ONE, TriggerId::TRIG_ONE, m_oneCounters),
		args.sampleTime);
	lights[PARAM_TRIG_ALL].setBrightnessSmooth(
		processTriggers(ParamId::PARAM_TRIG_ALL, InputId::INPUT_TRIG_ALL, TriggerId::TRIG_ALL, m_allCounters),
		args.sampleTime);
	lights[PARAM_TRIG_RESET].setBrightnessSmooth(
		processTriggers(ParamId::PARAM_TRIG_RESET, InputId::INPUT_TRIG_RESET, TriggerId::TRIG_RESET, m_resetCounters),
		args.sampleTime);
}

void SolimRandomModule::draw(const widget::Widget::DrawArgs& args) {
	int inputCount = 0;
	bool hasRightSolimModule = false;
	bool encounteredInputOctaver = false;
	Expander* expanderModule = &getRightExpander();
	while (expanderModule->module != nullptr) {
		if (expanderModule->module->getModel() == modelSolim) {
			hasRightSolimModule = true;
			break;
		} else if (expanderModule->module->getModel() == modelSolimInput && inputCount < 7) {
			// Allow up to 7 input expander modules
			inputCount++;
		} else if (expanderModule->module->getModel() == modelSolimInputOctaver && !encounteredInputOctaver) {
			// Allow 1 input octaver expander module
			encounteredInputOctaver = true;
		} else {
			// Don't allow any other type of modules in between
			break;
		}

		expanderModule = &expanderModule->module->getRightExpander();
	}

	inputCount = 0;
	int outputCount = 0;
	bool hasLeftSolimModule = false;
	encounteredInputOctaver = false;
	bool encounteredOutputOctaver = false;
	expanderModule = &getLeftExpander();
	while (expanderModule->module != nullptr) {
		if (!hasLeftSolimModule) {
			if (expanderModule->module->getModel() == modelSolim) {
				// Don't stop yet, because there might be another random module to the left of the core module that overwrites this one.
				hasLeftSolimModule = true;
			} else if (expanderModule->module->getModel() == modelSolimOutput && outputCount < 7) {
				// On the left side, there can be up to 7 output expanders
				outputCount++;
			} else if (expanderModule->module->getModel() == modelSolimOutputOctaver && !encounteredOutputOctaver) {
				// Allow 1 output octaver expander module
				encounteredOutputOctaver = true;
			} else {
				// Don't allow any other type of modules in between
				break;
			}
		} else {
			if (expanderModule->module->getModel() == modelSolimRandom) {
				// There is another random module to the left of the core module, so that will overwrite this one.
				hasLeftSolimModule = false;
				break;
			} else if (expanderModule->module->getModel() == modelSolimInput && inputCount < 7) {
				// Allow up to 7 input modules after the core module
				inputCount++;
			} else if (expanderModule->module->getModel() == modelSolimInputOctaver && !encounteredInputOctaver) {
				// Allow 1 input octaver expander module
				encounteredInputOctaver = true;
			} else {
				// Don't allow any other type of modules in between
				break;
			}
		}

		expanderModule = &expanderModule->module->getLeftExpander();
	}

	if (hasLeftSolimModule) {
		lights[LIGHT_CONNECTED_LEFT].setBrightness(1.f);
	} else {
		lights[LIGHT_CONNECTED_LEFT].setBrightness(0.f);
	}
	if (hasRightSolimModule) {
		lights[LIGHT_CONNECTED_RIGHT].setBrightness(1.f);
	} else {
		lights[LIGHT_CONNECTED_RIGHT].setBrightness(0.f);
	}
}

bool SolimRandomModule::processTriggers(ParamId paramId, InputId inputId, TriggerId triggerId, std::array<int, 8>& counters) {
	bool result = false;
	bool triggered = m_buttonTrigger[triggerId].process(params[paramId].getValue());
	if (triggered) {
		result = true;
		for (int i = 0; i < 8; i++) {
			counters[i]++;
		}
	} else {
		int channels = inputs[inputId].getChannels();
		for (int i = 0; i < channels && i < 8; i++) {
			if (m_trigTriggers[triggerId][i].process(inputs[inputId].getVoltage(i), 0.1f, 1.f)) {
				result = true;
				counters[i]++;
			}
		}
	}

	return result;
}

SolimRandomWidget::SolimRandomWidget(SolimRandomModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "solim-random") {
	addParam(createLightParamCentered<LEDLightBezel<DimmedLight<RedLight>>>(Vec(22.5f, 49.5f), module, SolimRandomModule::PARAM_TRIG_MOVE, SolimRandomModule::LIGHT_TRIG_MOVE));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 82.5f), module, SolimRandomModule::INPUT_TRIG_MOVE));

	addParam(createLightParamCentered<LEDLightBezel<DimmedLight<RedLight>>>(Vec(22.5f, 133.5f), module, SolimRandomModule::PARAM_TRIG_ONE, SolimRandomModule::LIGHT_TRIG_ONE));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 166.5f), module, SolimRandomModule::INPUT_TRIG_ONE));

	addParam(createLightParamCentered<LEDLightBezel<DimmedLight<RedLight>>>(Vec(22.5f, 217.5f), module, SolimRandomModule::PARAM_TRIG_ALL, SolimRandomModule::LIGHT_TRIG_ALL));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 250.5f), module, SolimRandomModule::INPUT_TRIG_ALL));

	addParam(createLightParamCentered<LEDLightBezel<DimmedLight<RedLight>>>(Vec(22.5f, 301.5f), module, SolimRandomModule::PARAM_TRIG_RESET, SolimRandomModule::LIGHT_TRIG_RESET));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 334.5f), module, SolimRandomModule::INPUT_TRIG_RESET));

	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(5.f, 20.f), module, SolimRandomModule::LIGHT_CONNECTED_LEFT));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(40.f, 20.f), module, SolimRandomModule::LIGHT_CONNECTED_RIGHT));
}


Model* modelSolimRandom = createModel<SolimRandomModule, SolimRandomWidget>("solim-random-expander");