#include "modules/ratrilig-expander.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"

RatriligExpanderModule::RatriligExpanderModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configInput(IN_SKIP_CLUSTER, "Cluster skip change CV");
	configInput(IN_SKIP_PHRASE, "Phrase skip change CV");
	configInput(IN_SKIP_CYCLE, "Cycle skip change CV");

	configOutput(OUT_TRIG_CLUSTER, "Cluster started trigger");
	configOutput(OUT_TRIG_PHRASE, "Phrase started trigger");
	configOutput(OUT_TRIG_CYCLE, "Cycle started trigger");
}

void RatriligExpanderModule::process(const ProcessArgs& args) {
	for (int i = 0; i < outputs[OUT_TRIG_CLUSTER].getChannels(); i++) {
		outputs[OUT_TRIG_CLUSTER].setVoltage(m_clusterPulse[i].process(args.sampleTime) ? 10.f : 0.f);
	}
	for (int i = 0; i < outputs[OUT_TRIG_PHRASE].getChannels(); i++) {
		outputs[OUT_TRIG_PHRASE].setVoltage(m_phrasePulse[i].process(args.sampleTime) ? 10.f : 0.f);
	}
	for (int i = 0; i < outputs[OUT_TRIG_CYCLE].getChannels(); i++) {
		outputs[OUT_TRIG_CYCLE].setVoltage(m_cyclePulse[i].process(args.sampleTime) ? 10.f : 0.f);
	}

	lights[LIGHT_TRIG_CLUSTER].setBrightnessSmooth(0.f, args.sampleTime);
	lights[LIGHT_TRIG_PHRASE].setBrightnessSmooth(0.f, args.sampleTime);
	lights[LIGHT_TRIG_CYCLE].setBrightnessSmooth(0.f, args.sampleTime);
}

void RatriligExpanderModule::triggerCluster(int channel) {
	m_clusterPulse[channel].trigger();
	lights[LIGHT_TRIG_CLUSTER].setBrightnessSmooth(1.f, .01f);
}

void RatriligExpanderModule::triggerPhrase(int channel) {
	m_phrasePulse[channel].trigger();
	lights[LIGHT_TRIG_PHRASE].setBrightnessSmooth(1.f, .01f);
}

void RatriligExpanderModule::triggerCycle(int channel) {
	m_cyclePulse[channel].trigger();
	lights[LIGHT_TRIG_CYCLE].setBrightnessSmooth(1.f, .01f);
}

RatriligExpanderWidget::RatriligExpanderWidget(RatriligExpanderModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "ratrilig-expander") {
	addInput(createInputCentered<NTPort>(Vec(22.5f, 55.f), module, RatriligExpanderModule::IN_SKIP_CLUSTER));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 100.f), module, RatriligExpanderModule::IN_SKIP_PHRASE));
	addInput(createInputCentered<NTPort>(Vec(22.5f, 145.f), module, RatriligExpanderModule::IN_SKIP_CYCLE));

	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 235.5f), module, RatriligExpanderModule::OUT_TRIG_CLUSTER));
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 327.5f), module, RatriligExpanderModule::OUT_TRIG_PHRASE));
	addOutput(createOutputCentered<NTPort>(Vec(22.5f, 282.5f), module, RatriligExpanderModule::OUT_TRIG_CYCLE));

	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 225.f), module, RatriligExpanderModule::IN_SKIP_CLUSTER));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 315.f), module, RatriligExpanderModule::IN_SKIP_PHRASE));
	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(35.f, 270.f), module, RatriligExpanderModule::IN_SKIP_CYCLE));
}


Model* modelRatriligExpander = createModel<RatriligExpanderModule, RatriligExpanderWidget>("ratrilig-expander");