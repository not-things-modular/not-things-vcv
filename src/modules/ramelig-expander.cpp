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
}

void RameligExpanderModule::process(const ProcessArgs& args) {
	lights[LIGHT_TRIG_JUMP].setBrightnessSmooth(0.f, args.sampleTime);
	lights[LIGHT_TRIG_MOVE].setBrightnessSmooth(0.f, args.sampleTime);
}

RameligExpanderWidget::RameligExpanderWidget(RameligExpanderModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "ramelig-expander") {
	addInput(createInputCentered<NTPort>(Vec(22.5f, 68.5f), module, RameligExpanderModule::IN_GUIDE_GATE));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 113.5f), module, RameligExpanderModule::IN_GUIDE_CV));

	addInput(createInputCentered<NTPort>(Vec(22.5f, 206.5f), module, RameligExpanderModule::IN_TRIG_JUMP));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 291.f), module, RameligExpanderModule::IN_TRIG_MOVE));
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 248.f), module, RameligExpanderModule::OUT_TRIG_JUMP));
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 332.5f), module, RameligExpanderModule::OUT_TRIG_MOVE));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 235.5f), module, RameligExpanderModule::OUT_TRIG_JUMP));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 320.f), module, RameligExpanderModule::OUT_TRIG_MOVE));
}


Model* modelRameligExpander = createModel<RameligExpanderModule, RameligExpanderWidget>("ramelig-expander");