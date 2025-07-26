#include "modules/ratrilig-expander.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"

RatriligExpanderModule::RatriligExpanderModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configInput(IN_SKIP_CLUSTER, "Cluster skip change CV");
	configInput(IN_SKIP_GROUP, "Group skip change CV");
	configInput(IN_SKIP_PHRASE, "Phrase skip change CV");

	configOutput(OUT_TRIG_CLUSTER, "Cluster started trigger");
	configOutput(OUT_TRIG_GROUP, "Group started trigger");
	configOutput(OUT_TRIG_PHRASE, "Phrase started trigger");
}

void RatriligExpanderModule::process(const ProcessArgs& args) {

}

RatriligExpanderWidget::RatriligExpanderWidget(RatriligExpanderModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "ratrilig-expander") {
	addInput(createInputCentered<NTPort>(Vec(22.5f, 52.5f), module, RatriligExpanderModule::IN_SKIP_CLUSTER));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 97.5f), module, RatriligExpanderModule::IN_SKIP_GROUP));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 142.5f), module, RatriligExpanderModule::IN_SKIP_PHRASE));

	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 232.5f), module, RatriligExpanderModule::OUT_TRIG_CLUSTER));
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 277.5f), module, RatriligExpanderModule::OUT_TRIG_GROUP));
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 322.5f), module, RatriligExpanderModule::OUT_TRIG_PHRASE));

	addChild(createLightCentered<TinyLight<DimmedLight<GreenRedLight>>>(Vec(35.f, 220.f), module, RatriligExpanderModule::IN_SKIP_CLUSTER));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenRedLight>>>(Vec(35.f, 265.f), module, RatriligExpanderModule::IN_SKIP_GROUP));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenRedLight>>>(Vec(35.f, 310.f), module, RatriligExpanderModule::IN_SKIP_PHRASE));
}


Model* modelRatriligExpander = createModel<RatriligExpanderModule, RatriligExpanderWidget>("ratrilig-expander");