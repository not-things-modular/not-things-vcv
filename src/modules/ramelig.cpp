#include "modules/ramelig.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"


extern Model* modelRamelig;

RameligModule::RameligModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configInput(IN_GATE, "Clock");
	configInput(IN_LOWER_LIMIT, "Lower limit CV");
	configInput(IN_UPPER_LIMIT, "Upper limit CV");
	configInput(IN_CHANCE_RANDOM_JUMP, "Random jump CV");
	configInput(IN_CHANCE_RANDOM_MOVE, "Random move CV");
	configInput(IN_CHANCE_MOVE_UP, "Move up chance CV");
	configInput(IN_CHANCE_REMAIN, "Remain chance CV");
	configInput(IN_CHANCE_MOVE_DOWN, "Move down chance CV");
	configInput(IN_SCALE, "Scale CV");

	configOutput(OUT_CV, "CV");
	configOutput(OUT_RANDOM_JUMP, "Random jump triggered");
	configOutput(OUT_RANDOM_REMAIN, "Random remain triggered");

	configParam(PARAM_LOWER_LIMIT, -10.f, 10.f, -1.f, "Lower limit", " V");
	configParam(PARAM_UPPER_LIMIT, -10.f, 10.f, 1.f, "Upper limit", " V");

	configParam(PARAM_CHANCE_RANDOM_JUMP, 0.f, 10.f, 3.5f, "Random jump chance");
	configButton(PARAM_TRIG_RANDOM_JUMP, "Random jump trigger");
	configParam(PARAM_CHANCE_RANDOM_MOVE, 0.f, 10.f, 2.5f, "Random move chance");
	configButton(PARAM_TRIG_RANDOM_MOVE, "Random move trigger");

	configParam(PARAM_CHANCE_MOVE_UP, 0.f, 10.f, 9.f, "Move up chance");
	configParam(PARAM_CHANCE_REMAIN, 0.f, 10.f, 2.f, "Remain chance");
	configParam(PARAM_CHANCE_MOVE_DOWN, 0.f, 10.f, 9.f, "Move down chance");
	configParam(PARAM_FACTOR_MOVE_TWO, 0.f, 10.f, 7.f, "Move by one or two steps factor");
	configParam(PARAM_FACTOR_REMAIN_REPEAT, 0.f, 10.f, 7.f, "Remain repeat factor");

	ParamQuantity* pq = configParam(PARAM_SCALE, 0.f, 10.f, 0.f, "Scale");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;

	const char* notes[] = { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
	for (int i = 0; i < 12; i++) {
		configButton(PARAM_SCALE_NOTES + i, notes[i]);
		m_scales[i] = { true, false, true, false, true, true, false, true, false, true, false, true };
	}

	m_activeScaleIndex = 0;
	updateScale();
}

void RameligModule::process(const ProcessArgs& args) {
	// Detect when the active scale is changed
	int activeScaleIndex = determineActiveScale();
	if (activeScaleIndex != m_activeScaleIndex) {
		m_activeScaleIndex = activeScaleIndex;
		updateScale();
	}

	// Detect when the user changes the scale through the scale buttons
	bool scaleChanged = false;
	for (int i = 0; i < 12; i++) {
		if (m_scaleButtonTriggers[i].process(params[PARAM_SCALE_NOTES + i].getValue(), 0.f, 1.f)) {
			m_scales[m_activeScaleIndex][i] = !m_scales[m_activeScaleIndex][i];
			scaleChanged = true;
		}
	}
	if (scaleChanged) {
		updateScale();
	}

	if (m_inputTrigger.process(inputs[IN_GATE].getVoltage(0), 0.f, 1.f)) {
		float upperLimit = getParamValue(PARAM_UPPER_LIMIT, -10.f, 10.f, IN_UPPER_LIMIT, 1.f);
		float lowerLimit = getParamValue(PARAM_LOWER_LIMIT, -10.f, 10.f, IN_LOWER_LIMIT, 1.f);

		m_rameligCoreData.scale = m_activeScaleIndices;
		m_rameligCoreData.distributionData.randomJumpChance = getParamValue(PARAM_CHANCE_RANDOM_JUMP, 0.f, 10.f, IN_CHANCE_RANDOM_JUMP, 1.f) / 10.f;
		m_rameligCoreData.distributionData.randomMoveChance = getParamValue(PARAM_CHANCE_RANDOM_MOVE, 0.f, 10.f, IN_CHANCE_RANDOM_MOVE, 1.f) / 10.f;
		m_rameligCoreData.distributionData.moveUpChance = getParamValue(PARAM_CHANCE_MOVE_UP, 0.f, 10.f, IN_CHANCE_MOVE_UP, 1.f) / 10.f;
		m_rameligCoreData.distributionData.remainChance = getParamValue(PARAM_CHANCE_REMAIN, 0.f, 10.f, IN_CHANCE_REMAIN, 1.f) / 10.f;
		m_rameligCoreData.distributionData.moveDownChance = getParamValue(PARAM_CHANCE_MOVE_DOWN, 0.f, 10.f, IN_CHANCE_MOVE_DOWN, 1.f) / 10.f;
		m_rameligCoreData.distributionData.moveTwoFactor = params[PARAM_FACTOR_MOVE_TWO].getValue() / 10.f;
		m_rameligCoreData.distributionData.remainRepeatFactor = params[PARAM_FACTOR_REMAIN_REPEAT].getValue() / 10.f;

		outputs[OUT_CV].setVoltage(m_rameligCore.process(m_rameligCoreData, lowerLimit, upperLimit));
	}
}

int RameligModule::determineActiveScale() {
	return params[PARAM_SCALE].getValue();
}

void RameligModule::updateScale() {
	std::vector<int> scaleIndices = {};
	
	for (int i = 0; i < 12; i++) {
		if (m_scales[m_activeScaleIndex][i]) {
			lights[LIGHT_SCALE_NOTES + i].setBrightness(1.f);
			scaleIndices.push_back(i);
		} else {
			lights[LIGHT_SCALE_NOTES + i].setBrightness(0.f);
		}
	}

	if (!scaleIndices.empty()) {
		m_activeScaleIndices = scaleIndices;
	} else {
		m_activeScaleIndices = { 0 };
	}
}

float RameligModule::getParamValue(ParamId paramId, float lowerLimit, float upperLimit, InputId inputId, float inputScaling) {
	float value = params[paramId].getValue() + (inputs[inputId].getVoltage(0) * inputScaling);
	return std::max(std::min(value, upperLimit), lowerLimit);
}

RameligWidget::RameligWidget(RameligModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "ramelig") {
	addParam(createParamCentered<Rogan1PWhite>(Vec(32.5f, 232.5f), module, RameligModule::PARAM_LOWER_LIMIT));
	addParam(createParamCentered<Rogan1PWhite>(Vec(72.5f, 232.5f), module, RameligModule::PARAM_UPPER_LIMIT));

	addParam(createParamCentered<Rogan1PWhite>(Vec(137.5f, 232.5f), module, RameligModule::PARAM_CHANCE_RANDOM_JUMP));
	addParam(createParamCentered<VCVButton>(Vec(137.5f, 310.5f), module, RameligModule::PARAM_TRIG_RANDOM_JUMP));
	addParam(createParamCentered<Rogan1PWhite>(Vec(177.5f, 232.5f), module, RameligModule::PARAM_CHANCE_RANDOM_MOVE));
	addParam(createParamCentered<VCVButton>(Vec(177.5f, 310.5f), module, RameligModule::PARAM_TRIG_RANDOM_MOVE));

	addParam(createParamCentered<Rogan1PWhite>(Vec(137.5f, 47.f), module, RameligModule::PARAM_CHANCE_MOVE_UP));
	addParam(createParamCentered<Rogan1PWhite>(Vec(177.5f, 47.f), module, RameligModule::PARAM_CHANCE_REMAIN));
	addParam(createParamCentered<Rogan1PWhite>(Vec(97.5f, 47.f), module, RameligModule::PARAM_CHANCE_MOVE_DOWN));
	addParam(createParamCentered<Trimpot>(Vec(117.5f, 127.5f), module, RameligModule::PARAM_FACTOR_MOVE_TWO));
	addParam(createParamCentered<Trimpot>(Vec(177.5f, 127.f), module, RameligModule::PARAM_FACTOR_REMAIN_REPEAT));

	addParam(createParamCentered<Trimpot>(Vec(32.5f, 137.f), module, RameligModule::PARAM_SCALE));
	
	addInput(createInputCentered<NTPort>(Vec(32.5f, 47.f), module, RameligModule::IN_GATE));

	addInput(createInputCentered<NTPort>(Vec(32.5f, 272.5f), module, RameligModule::IN_LOWER_LIMIT));
	addInput(createInputCentered<NTPort>(Vec(72.5f, 272.5f), module, RameligModule::IN_UPPER_LIMIT));

	addInput(createInputCentered<NTPort>(Vec(137.5f, 272.5f), module, RameligModule::IN_CHANCE_RANDOM_JUMP));
	addInput(createInputCentered<NTPort>(Vec(177.5f, 272.5f), module, RameligModule::IN_CHANCE_RANDOM_MOVE));
	addInput(createInputCentered<NTPort>(Vec(137.5f, 87.f), module, RameligModule::IN_CHANCE_MOVE_UP));
	addInput(createInputCentered<NTPort>(Vec(177.5f, 87.f), module, RameligModule::IN_CHANCE_REMAIN));
	addInput(createInputCentered<NTPort>(Vec(97.5f, 87.f), module, RameligModule::IN_CHANCE_MOVE_DOWN));

	addInput(createInputCentered<NTPort>(Vec(32.5f, 172.f), module, RameligModule::IN_SCALE));
	
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(65.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 11, RameligModule::LIGHT_SCALE_NOTES + 11));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(85.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 9, RameligModule::LIGHT_SCALE_NOTES + 9));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(105.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 7, RameligModule::LIGHT_SCALE_NOTES + 7));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(125.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 5, RameligModule::LIGHT_SCALE_NOTES + 5));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(145.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 4, RameligModule::LIGHT_SCALE_NOTES + 4));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(165.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 2, RameligModule::LIGHT_SCALE_NOTES + 2));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(185.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 0, RameligModule::LIGHT_SCALE_NOTES + 0));

	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(75.5f, 162.f), module, RameligModule::PARAM_SCALE_NOTES + 10, RameligModule::LIGHT_SCALE_NOTES + 10));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(95.f, 162.f), module, RameligModule::PARAM_SCALE_NOTES + 8, RameligModule::LIGHT_SCALE_NOTES + 8));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(130.5f, 162.f), module, RameligModule::PARAM_SCALE_NOTES + 6, RameligModule::LIGHT_SCALE_NOTES + 6));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(155.5f, 162.f), module, RameligModule::PARAM_SCALE_NOTES + 3, RameligModule::LIGHT_SCALE_NOTES + 3));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(175.5f, 162.f), module, RameligModule::PARAM_SCALE_NOTES + 1, RameligModule::LIGHT_SCALE_NOTES + 1));

	addOutput(createOutputCentered<NTPort>(Vec(61.f, 332.5f), module, RameligModule::OUT_CV));
	addOutput(createOutputCentered<NTPort>(Vec(137.5f, 345.f), module, RameligModule::OUT_RANDOM_JUMP));
	addOutput(createOutputCentered<NTPort>(Vec(177.5f, 345.f), module, RameligModule::OUT_RANDOM_REMAIN));
}


Model* modelRamelig = createModel<RameligModule, RameligWidget>("ramelig");