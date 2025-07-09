#include "modules/ramelig.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"


extern Model* modelRamelig;

void reduceLight(Light& light, float deltaTime, float lambda) {
	if (light.getBrightness() > 0.f) {
		if (light.getBrightness() > 0.0001f) {
			light.setBrightnessSmooth(0.f, deltaTime, lambda);
		} else {
			light.setBrightness(0.f);
		}
	}
}

RameligModule::RameligModule() : m_rameligCore(this) {
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
	configOutput(OUT_TRIGGER, "Trigger");
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

	configButton(PARAM_TRIGGER, "Trigger");

	ParamQuantity* pq = configParam(PARAM_SCALE, 0.f, 9.f, 0.f, "Scale");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;
	pq->displayOffset = 1.f;

	const char* notes[] = { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
	for (int i = 0; i < 12; i++) {
		configButton(PARAM_SCALE_NOTES + i, notes[i]);
		m_scales[i] = { true, false, true, false, true, true, false, true, false, true, false, true };
	}

	m_scaleMode = ScaleMode::SCALE_MODE_DECIMAL;
	m_activeScaleIndex = 0;
	updateScale();

	m_lightDivider.setDivision(128);
}

json_t* RameligModule::dataToJson() {
	json_t* rootJ = NTModule::dataToJson();

	json_t* scales = json_array();
	for (int i = 0; i < 12; i++) {
		json_int_t scale = 0;
		for (size_t j = 0; j < m_scales[i].size(); j++) {
			if (m_scales[i][j]) {
				scale |= (1 << j);
			}
		}
		json_array_append_new(scales, json_integer(scale));
	}
	json_object_set(rootJ,"ntRameligScales", scales);
	json_decref(scales);

	json_object_set(rootJ, "ntRameligScaleMode", json_integer(m_scaleMode));

	return rootJ;
}

void RameligModule::dataFromJson(json_t* rootJ) {
	NTModule::dataFromJson(rootJ);

	json_t* ntRameligScaleMode = json_object_get(rootJ, "ntRameligScaleMode");
	if (json_is_integer(ntRameligScaleMode)) {
		json_int_t scaleMode = json_integer_value(ntRameligScaleMode);
		if ((scaleMode > 0) && (scaleMode < ScaleMode::NUM_SCALE_MODES)) {
			setScaleMode(static_cast<ScaleMode>(scaleMode));
		} else {
			setScaleMode(ScaleMode::SCALE_MODE_DECIMAL);
		}
	}

	json_t* ntRameligScales = json_object_get(rootJ, "ntRameligScales");
	if (json_is_array(ntRameligScales)) {
		for (size_t i = 0; i < 12; i++) {
			if (i < json_array_size(ntRameligScales)) {
				json_t* scaleJ = json_array_get(ntRameligScales, i);
				if (json_is_integer(scaleJ)) {
					json_int_t scale = json_integer_value(scaleJ);
					for (int j = 0; j < 12; j++) {
						m_scales[i][j] = ((scale & (1 << j))) != 0;
					}
				} else {
					m_scales[i] = { true, false, true, false, true, true, false, true, false, true, false, true };
				}
			} else {
				m_scales[i] = { true, false, true, false, true, true, false, true, false, true, false, true };
			}
		}
	}
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

	// Check if the trigger button was pushed
	bool triggerPushed = m_buttonTrigger.process(params[PARAM_TRIGGER].getValue());
	if (triggerPushed) {
		m_triggerPulse.trigger();
	}

	// Do the actual processing if needed
	if ((m_inputTrigger.process(inputs[IN_GATE].getVoltage(0), 0.f, 1.f)) || (triggerPushed)) {
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
		lights[LIGHT_TRIGGER].setBrightness(1.f);
	} else if (m_lightDivider.process()) {
		reduceLight(lights[LIGHT_TRIGGER], args.sampleTime * 128, 10.f);
		reduceLight(lights[LIGHT_RANDOM_JUMP], args.sampleTime * 128, 15.f);
		reduceLight(lights[LIGHT_RANDOM_REMAIN], args.sampleTime * 128, 15.f);
	}

	// Update the trigger output based on either the input trigger, or the trigger button pulse
	if ((m_triggerPulse.process(args.sampleTime)) || (inputs[IN_GATE].getVoltage(0) >= 1.f)) {
		outputs[OUT_TRIGGER].setVoltage(10.f);
	} else {
		outputs[OUT_TRIGGER].setVoltage(0.f);
	}
}

void RameligModule::rameligActionPerformed(RameligActions action) {
	if (action == RameligActions::RANDOM_JUMP) {
		lights[LIGHT_RANDOM_JUMP].setBrightness(1.f);
	} else if (action == RameligActions::RANDOM_MOVE) {
		lights[LIGHT_RANDOM_REMAIN].setBrightness(1.f);
	}
}

RameligModule::ScaleMode RameligModule::getScaleMode() {
	return m_scaleMode;
}

void RameligModule::setScaleMode(ScaleMode scaleMode) {
	m_scaleMode = scaleMode;

	float scale = params[ParamId::PARAM_SCALE].getValue();
	if (m_scaleMode == ScaleMode::SCALE_MODE_DECIMAL) {
		if (scale > 9.f) {
			scale = 9.f;
		}
		getParamQuantity(ParamId::PARAM_SCALE)->maxValue = 9.f;
	} else {
		getParamQuantity(ParamId::PARAM_SCALE)->maxValue = 11.f;
	}
	params[ParamId::PARAM_SCALE].setValue(scale);
	updateScale();
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

	addParam(createLightParamCentered<VCVLightButton<DimmedLight<MediumSimpleLight<RedLight>>>>(Vec(32.5f, 87.f), module, RameligModule::PARAM_TRIGGER, RameligModule::LIGHT_TRIGGER));

	addInput(createInputCentered<NTPort>(Vec(32.5f, 47.f), module, RameligModule::IN_GATE));

	addInput(createInputCentered<NTPort>(Vec(32.5f, 272.5f), module, RameligModule::IN_LOWER_LIMIT));
	addInput(createInputCentered<NTPort>(Vec(72.5f, 272.5f), module, RameligModule::IN_UPPER_LIMIT));

	addInput(createInputCentered<NTPort>(Vec(137.5f, 272.5f), module, RameligModule::IN_CHANCE_RANDOM_JUMP));
	addInput(createInputCentered<NTPort>(Vec(177.5f, 272.5f), module, RameligModule::IN_CHANCE_RANDOM_MOVE));
	addInput(createInputCentered<NTPort>(Vec(137.5f, 87.f), module, RameligModule::IN_CHANCE_MOVE_UP));
	addInput(createInputCentered<NTPort>(Vec(177.5f, 87.f), module, RameligModule::IN_CHANCE_REMAIN));
	addInput(createInputCentered<NTPort>(Vec(97.5f, 87.f), module, RameligModule::IN_CHANCE_MOVE_DOWN));

	addInput(createInputCentered<NTPort>(Vec(32.5f, 172.f), module, RameligModule::IN_SCALE));

	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(65.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 0, RameligModule::LIGHT_SCALE_NOTES + 0));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(85.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 2, RameligModule::LIGHT_SCALE_NOTES + 2));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(105.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 4, RameligModule::LIGHT_SCALE_NOTES + 4));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(125.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 5, RameligModule::LIGHT_SCALE_NOTES + 5));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(145.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 7, RameligModule::LIGHT_SCALE_NOTES + 7));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(165.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 9, RameligModule::LIGHT_SCALE_NOTES + 9));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(185.5f, 182.f), module, RameligModule::PARAM_SCALE_NOTES + 11, RameligModule::LIGHT_SCALE_NOTES + 11));

	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(75.5f, 162.f), module, RameligModule::PARAM_SCALE_NOTES + 1, RameligModule::LIGHT_SCALE_NOTES + 1));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(95.f, 162.f), module, RameligModule::PARAM_SCALE_NOTES + 3, RameligModule::LIGHT_SCALE_NOTES + 3));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(130.5f, 162.f), module, RameligModule::PARAM_SCALE_NOTES + 6, RameligModule::LIGHT_SCALE_NOTES + 6));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(155.5f, 162.f), module, RameligModule::PARAM_SCALE_NOTES + 8, RameligModule::LIGHT_SCALE_NOTES + 8));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(175.5f, 162.f), module, RameligModule::PARAM_SCALE_NOTES + 10, RameligModule::LIGHT_SCALE_NOTES + 10));

	addOutput(createOutputCentered<NTPort>(Vec(32.f, 332.5f), module, RameligModule::OUT_CV));
	addOutput(createOutputCentered<NTPort>(Vec(72.f, 332.5f), module, RameligModule::OUT_TRIGGER));
	addOutput(createOutputCentered<NTPort>(Vec(137.5f, 345.f), module, RameligModule::OUT_RANDOM_JUMP));
	addOutput(createOutputCentered<NTPort>(Vec(177.5f, 345.f), module, RameligModule::OUT_RANDOM_REMAIN));

	addChild(createLightCentered<SmallLight<DimmedLight<RedLight>>>(Vec(150.f, 332.5f), module, RameligModule::LIGHT_RANDOM_JUMP));
	addChild(createLightCentered<SmallLight<DimmedLight<RedLight>>>(Vec(190.f, 332.5f), module, RameligModule::LIGHT_RANDOM_REMAIN));
}

void RameligWidget::appendContextMenu(Menu* menu) {
	NTModuleWidget::appendContextMenu(menu);

	RameligModule::ScaleMode scaleMode = getModule() ? dynamic_cast<RameligModule *>(getModule())->getScaleMode() : RameligModule::ScaleMode::SCALE_MODE_DECIMAL;
	menu->addChild(new MenuSeparator);
	menu->addChild(createSubmenuItem("Scale selection mode", "",
		[this, scaleMode](Menu* menu) {
			menu->addChild(createCheckMenuItem("Decimal", "", [scaleMode]() { return scaleMode == RameligModule::ScaleMode::SCALE_MODE_DECIMAL; }, [this, scaleMode]() { this->setScaleMode(RameligModule::ScaleMode::SCALE_MODE_DECIMAL); }));
			menu->addChild(createCheckMenuItem("Chromatic", "", [scaleMode]() { return scaleMode == RameligModule::ScaleMode::SCALE_MODE_CHROMATIC; }, [this, scaleMode]() { this->setScaleMode(RameligModule::ScaleMode::SCALE_MODE_CHROMATIC); }));
		}
	));
}

void RameligWidget::setScaleMode(RameligModule::ScaleMode scaleMode) {
	RameligModule* rameligModule = dynamic_cast<RameligModule *>(getModule());
	if ((rameligModule != nullptr) && (scaleMode != rameligModule->getScaleMode())) {
		rameligModule->setScaleMode(scaleMode);
	}
}



Model* modelRamelig = createModel<RameligModule, RameligWidget>("ramelig");