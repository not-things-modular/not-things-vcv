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
	outputs[OUT_TRIG_CLUSTER].setVoltage(m_clusterPulse.process(args.sampleTime));
	outputs[OUT_TRIG_GROUP].setVoltage(m_groupPulse.process(args.sampleTime));
	outputs[OUT_TRIG_PHRASE].setVoltage(m_phrasePulse.process(args.sampleTime));

	lights[LIGHT_TRIG_CLUSTER].setBrightnessSmooth(0.f, args.sampleTime);
	lights[LIGHT_TRIG_GROUP].setBrightnessSmooth(0.f, args.sampleTime);
	lights[LIGHT_TRIG_PHRASE].setBrightnessSmooth(0.f, args.sampleTime);
}

void RatriligExpanderModule::triggerCluster() {
	m_clusterPulse.trigger();
	lights[LIGHT_TRIG_CLUSTER].setBrightnessSmooth(1.f, .01f);
}

void RatriligExpanderModule::triggerGroup() {
	m_groupPulse.trigger();
	lights[LIGHT_TRIG_GROUP].setBrightnessSmooth(1.f, .01f);
}

void RatriligExpanderModule::triggerPhrase() {
	m_phrasePulse.trigger();
	lights[LIGHT_TRIG_PHRASE].setBrightnessSmooth(1.f, .01f);
}

RatriligExpanderWidget::RatriligExpanderWidget(RatriligExpanderModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "ratrilig-expander") {
	addInput(createInputCentered<NTPort>(Vec(22.5f, 55.f), module, RatriligExpanderModule::IN_SKIP_CLUSTER));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 100.f), module, RatriligExpanderModule::IN_SKIP_GROUP));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 145.f), module, RatriligExpanderModule::IN_SKIP_PHRASE));

	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 235.5f), module, RatriligExpanderModule::OUT_TRIG_CLUSTER));
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 282.5f), module, RatriligExpanderModule::OUT_TRIG_GROUP));
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 327.5f), module, RatriligExpanderModule::OUT_TRIG_PHRASE));

	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 225.f), module, RatriligExpanderModule::IN_SKIP_CLUSTER));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 270.f), module, RatriligExpanderModule::IN_SKIP_GROUP));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 315.f), module, RatriligExpanderModule::IN_SKIP_PHRASE));
}


Model* modelRatriligExpander = createModel<RatriligExpanderModule, RatriligExpanderWidget>("ratrilig-expander");