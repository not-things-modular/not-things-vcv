#include "modules/ramelig-expander.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"

RameligExpanderModule::RameligExpanderModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configInput(IN_TRIG_JUMP, "Jump trigger");
	configInput(IN_TRIG_MOVE, "Move trigger");
	configInput(IN_GUIDE_CV, "Guide melody CV");
	configInput(IN_GUIDE_GATE, "Guide melody gate");

	configOutput(OUT_TRIG_JUMP, "Jump trigger");
	configOutput(OUT_TRIG_MOVE, "Move trigger");

	configSwitch(PARAM_QUANTIZE, 0.f, 1.f, 0.f, "Quantize guide value to scale", {"No", "Yes"});
}

void RameligExpanderModule::process(const ProcessArgs& args) {
	outputs[OUT_TRIG_JUMP].setVoltage(m_jumpPulse.process(args.sampleTime) ? 10.f : 0.f);
	outputs[OUT_TRIG_MOVE].setVoltage(m_movePulse.process(args.sampleTime) ? 10.f : 0.f);

	lights[LIGHT_TRIG_JUMP].setBrightnessSmooth(0.f, args.sampleTime);
	lights[LIGHT_TRIG_MOVE].setBrightnessSmooth(0.f, args.sampleTime);
}

void RameligExpanderModule::triggerJump() {
	m_jumpPulse.trigger();
	lights[LIGHT_TRIG_JUMP].setBrightnessSmooth(1.f, .01f);
}

void RameligExpanderModule::triggerMove() {
	m_movePulse.trigger();
	lights[LIGHT_TRIG_MOVE].setBrightnessSmooth(1.f, .01f);
}

RameligExpanderWidget::RameligExpanderWidget(RameligExpanderModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "ramelig-expander") {
	addInput(createInputCentered<NTPort>(Vec(22.5f, 70.5f-18.f), module, RameligExpanderModule::IN_TRIG_JUMP));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 155.5f-18.f), module, RameligExpanderModule::IN_TRIG_MOVE));

	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 113.f-19.f), module, RameligExpanderModule::OUT_TRIG_JUMP));
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 198.f-19.f), module, RameligExpanderModule::OUT_TRIG_MOVE));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 81.5f), module, RameligExpanderModule::OUT_TRIG_JUMP));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 166.5f), module, RameligExpanderModule::OUT_TRIG_MOVE));

	addInput(createInputCentered<NTPort>(Vec(22.5f, 258.5f-18.f+7.f), module, RameligExpanderModule::IN_GUIDE_CV));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 303.5f-18.f+7.f), module, RameligExpanderModule::IN_GUIDE_GATE));

	addParam(createLightParamCentered<VCVLightLatch<SmallSimpleLight<DimmedLight<GreenLight>>>>(Vec(22.5f, 303.5f-18.f+45.f), module, RameligExpanderModule::PARAM_QUANTIZE, RameligExpanderModule::LIGHT_QUANTIZE));
}


Model* modelRameligExpander = createModel<RameligExpanderModule, RameligExpanderWidget>("ramelig-expander");