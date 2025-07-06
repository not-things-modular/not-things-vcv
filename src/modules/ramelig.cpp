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
	configInput(IN_CHANCE_MOVE_UP, "Move up chance CV");
	configInput(IN_CHANCE_REMAIN, "Remain chance CV");
	configInput(IN_CHANCE_MOVE_DOWN, "Move down chance CV");
	configInput(IN_SCALE, "Scale CV");

	configOutput(OUT_CV, "CV");
	configOutput(OUT_GATE, "Gate");
	configOutput(OUT_RANDOM_JUMP, "Random jump triggered");
	configOutput(OUT_RANDOM_REMAIN, "Random remain triggered");

	configParam(PARAM_LOWER_LIMIT, -10.f, 10.f, -1.f, "Lower limit", " V");
	configParam(PARAM_UPPER_LIMIT, -10.f, 10.f, 1.f, "Upper limit", " V");

	configParam(PARAM_CHANCE_RANDOM_JUMP, 0.f, 10.f, 5.f, "Random jump chance");
	configButton(PARAM_TRIG_RANDOM_JUMP, "Random jump trigger");
	configParam(PARAM_FACTOR_JUMP_REMAIN, 0.f, 10.f, 7.f, "Random jump: remain to return factor");

	configParam(PARAM_CHANCE_MOVE_UP, 0.f, 10.f, 9.f, "Move up chance");
	configParam(PARAM_CHANCE_REMAIN, 0.f, 10.f, 2.f, "Remain chance");
	configParam(PARAM_CHANCE_MOVE_DOWN, 0.f, 10.f, 9.f, "Move down chance");
	configParam(PARAM_FACTOR_MOVE_TWO, 0.f, 10.f, 7.f, "Move by one or two steps factor");
	configParam(PARAM_FACTOR_REMAIN_REPEAT, 0.f, 10.f, 7.f, "Remain repeat factor");

	configParam(PARAM_SCALE, 0.f, 10.f, 0.f, "Scale");
	configParam(PARAM_SCALE_NOTES + 0, 0.f, 10.f, 0.f, "Scale");
}

void RameligModule::process(const ProcessArgs& args) {
	if (m_trigger.process(inputs[IN_GATE].getVoltage(0), 0.f, 1.f)) {
		float upperLimit = inputs[IN_UPPER_LIMIT].getVoltage();
		float lowerLimit = inputs[IN_LOWER_LIMIT].getVoltage();

		m_rameligCoreData.scale = { 0, 2, 4, 7, 9 };
		// m_rameligCoreData.scale = { 0, 2, 4, 5, 7, 9, 11 };
		m_rameligCoreData.distributionData.randomJumpChance = 0.65f;
		// m_rameligCoreData.distributionData.randomJumpChance = 0.f;
		m_rameligCoreData.distributionData.randomJumpStayOrReturnFactor = .55f;
		m_rameligCoreData.distributionData.moveUpChance = .9f;
		m_rameligCoreData.distributionData.remainChance = .2f;
		m_rameligCoreData.distributionData.moveDownChance = .9f;
		m_rameligCoreData.distributionData.moveTwoFactor = .3f;
		// m_rameligCoreData.distributionData.moveTwoFactor = 0.f;
		m_rameligCoreData.distributionData.remainRepeatFactor = .3f;

		outputs[OUT_CV].setVoltage(m_rameligCore.process(m_rameligCoreData, lowerLimit, upperLimit));
	}
}

RameligWidget::RameligWidget(RameligModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "ramelig") {
	addParam(createParamCentered<Rogan1PWhite>(Vec(20, 40), module, RameligModule::PARAM_LOWER_LIMIT));
	addParam(createParamCentered<Rogan1PWhite>(Vec(100, 40), module, RameligModule::PARAM_UPPER_LIMIT));

	addParam(createParamCentered<Rogan1PWhite>(Vec(20, 80), module, RameligModule::PARAM_CHANCE_RANDOM_JUMP));
	addParam(createParamCentered<VCVButton>(Vec(100, 80), module, RameligModule::PARAM_TRIG_RANDOM_JUMP));
	addParam(createParamCentered<Trimpot>(Vec(140, 80), module, RameligModule::PARAM_FACTOR_JUMP_REMAIN));

	addParam(createParamCentered<Rogan1PWhite>(Vec(20, 120), module, RameligModule::PARAM_CHANCE_MOVE_UP));
	addParam(createParamCentered<Rogan1PWhite>(Vec(60, 160), module, RameligModule::PARAM_CHANCE_REMAIN));
	addParam(createParamCentered<Rogan1PWhite>(Vec(100, 120), module, RameligModule::PARAM_CHANCE_MOVE_DOWN));
	addParam(createParamCentered<Trimpot>(Vec(180, 120), module, RameligModule::PARAM_FACTOR_MOVE_TWO));
	addParam(createParamCentered<Trimpot>(Vec(100, 160), module, RameligModule::PARAM_FACTOR_REMAIN_REPEAT));

	addParam(createParamCentered<Trimpot>(Vec(20, 200), module, RameligModule::PARAM_SCALE));
	
	addInput(createInputCentered<NTPort>(Vec(20, 350), module, RameligModule::IN_GATE));

	addInput(createInputCentered<NTPort>(Vec(60, 40), module, RameligModule::IN_LOWER_LIMIT));
	addInput(createInputCentered<NTPort>(Vec(140, 40), module, RameligModule::IN_UPPER_LIMIT));

	addInput(createInputCentered<NTPort>(Vec(60, 80), module, RameligModule::IN_CHANCE_RANDOM_JUMP));
	addInput(createInputCentered<NTPort>(Vec(60, 120), module, RameligModule::IN_CHANCE_MOVE_UP));
	addInput(createInputCentered<NTPort>(Vec(20, 160), module, RameligModule::IN_CHANCE_REMAIN));
	addInput(createInputCentered<NTPort>(Vec(140, 120), module, RameligModule::IN_CHANCE_MOVE_DOWN));

	addInput(createInputCentered<NTPort>(Vec(60, 200), module, RameligModule::IN_SCALE));
	
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 200), module, RameligModule::PARAM_SCALE_NOTES + 11, RameligModule::LIGHT_SCALE_NOTES + 11));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 220), module, RameligModule::PARAM_SCALE_NOTES + 9, RameligModule::LIGHT_SCALE_NOTES + 9));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 240), module, RameligModule::PARAM_SCALE_NOTES + 7, RameligModule::LIGHT_SCALE_NOTES + 7));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(100, 260), module, RameligModule::PARAM_SCALE_NOTES + 5, RameligModule::LIGHT_SCALE_NOTES + 5));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 280), module, RameligModule::PARAM_SCALE_NOTES + 4, RameligModule::LIGHT_SCALE_NOTES + 4));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 300), module, RameligModule::PARAM_SCALE_NOTES + 2, RameligModule::LIGHT_SCALE_NOTES + 2));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 320), module, RameligModule::PARAM_SCALE_NOTES + 0, RameligModule::LIGHT_SCALE_NOTES + 0));

	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(120, 210), module, RameligModule::PARAM_SCALE_NOTES + 10, RameligModule::LIGHT_SCALE_NOTES + 7));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(120, 230), module, RameligModule::PARAM_SCALE_NOTES + 8, RameligModule::LIGHT_SCALE_NOTES + 5));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(120, 250), module, RameligModule::PARAM_SCALE_NOTES + 6, RameligModule::LIGHT_SCALE_NOTES + 4));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(120, 290), module, RameligModule::PARAM_SCALE_NOTES + 3, RameligModule::LIGHT_SCALE_NOTES + 2));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(120, 310), module, RameligModule::PARAM_SCALE_NOTES + 1, RameligModule::LIGHT_SCALE_NOTES + 0));

	addOutput(createOutputCentered<NTPort>(Vec(100, 350), module, RameligModule::OUT_CV));
	addOutput(createOutputCentered<NTPort>(Vec(140, 350), module, RameligModule::OUT_GATE));
	addOutput(createOutputCentered<NTPort>(Vec(180, 350), module, RameligModule::OUT_RANDOM_JUMP));
	addOutput(createOutputCentered<NTPort>(Vec(220, 350), module, RameligModule::OUT_RANDOM_REMAIN));
}


Model* modelRamelig = createModel<RameligModule, RameligWidget>("ramelig");