#include "modules/ratrilig-expander.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"


extern Model* modelRatrilig;


RatriligExpanderModule::RatriligExpanderModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configInput(IN_SKIP_CLUSTER, "Cluster skip chance CV");
	configInput(IN_SKIP_PHRASE, "Phrase skip chance CV");
	configInput(IN_SKIP_CYCLE, "Cycle skip chance CV");

	configOutput(OUT_TRIG_CLUSTER, "Cluster started trigger");
	configOutput(OUT_TRIG_PHRASE, "Phrase started trigger");
	configOutput(OUT_TRIG_CYCLE, "Cycle started trigger");
}

void RatriligExpanderModule::process(const ProcessArgs& args) {
	lights[LIGHT_TRIG_CLUSTER].setBrightnessSmooth(0.f, args.sampleTime);
	lights[LIGHT_TRIG_PHRASE].setBrightnessSmooth(0.f, args.sampleTime);
	lights[LIGHT_TRIG_CYCLE].setBrightnessSmooth(0.f, args.sampleTime);
}

RatriligExpanderWidget::RatriligExpanderWidget(RatriligExpanderModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "ratrilig-expander") {
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 68.5f), module, RatriligExpanderModule::OUT_TRIG_CLUSTER));
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 113.5f), module, RatriligExpanderModule::OUT_TRIG_PHRASE));
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 158.5f), module, RatriligExpanderModule::OUT_TRIG_CYCLE));

	addInput(createInputCentered<NTPort>(Vec(22.5f, 232.5f), module, RatriligExpanderModule::IN_SKIP_CLUSTER));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 277.5f), module, RatriligExpanderModule::IN_SKIP_PHRASE));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 322.5f), module, RatriligExpanderModule::IN_SKIP_CYCLE));

	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 56.f), module, RatriligExpanderModule::LIGHT_TRIG_CLUSTER));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 101.f), module, RatriligExpanderModule::LIGHT_TRIG_PHRASE));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 146.f), module, RatriligExpanderModule::LIGHT_TRIG_CYCLE));
}


Model* modelRatriligExpander = createModel<RatriligExpanderModule, RatriligExpanderWidget>("ratrilig-expander");