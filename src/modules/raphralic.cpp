#include "modules/raphralic.hpp"
#include "components/ntport.hpp"


RaphralicModule::RaphralicModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configInput(IN_CLOCK, "Constant clock");
	configInput(IN_GATE, "Source gate");
	configInput(IN_CV, "Source CV");
	configInput(IN_SKIP, "Source is skipping");

	configInput(IN_RESET_POS, "Reset position trigger");
	configInput(IN_RESET_MEM, "Reset captured phrases trigger");

	configOutput(OUT_GATE, "Gate");
	configOutput(OUT_CV, "CV");
	configOutput(OUT_REPLAY, "Replaying");

	ParamQuantity* pq = configParam(PARAM_CLUSTER_SIZE, 1.f, 32.f, 8.f, "Gates per Cluster");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;
	pq = configParam(PARAM_PHRASE_SIZE, 1.f, 32.f, 4.f, "Clusters per Phrase");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;

	pq = configParam(PARAM_CLUSTER_SLOTS, 1.f, 8.f, 4.f, "Capture cluster slot count");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;
	pq = configParam(PARAM_PHRASE_SLOTS, 1.f, 8.f, 4.f, "Capture phrase slot count");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;

	configParam(PARAM_CLUSTER_CAPTURE_CHANCE, 0.f, 100.f, 15.f, "Cluster capture chance");
	configParam(PARAM_PHRASE_CAPTURE_CHANCE, 0.f, 100.f, 15.f, "Phrase capture chance");

	configParam(PARAM_CLUSTER_REPLAY_CHANCE, 0.f, 100.f, 15.f, "Cluster capture replay chance");
	configParam(PARAM_PHRASE_REPLAY_CHANCE, 0.f, 100.f, 15.f, "Phrase capture replay chance");

	configButton(PARAM_RESET_POS, "Reset position");
	configButton(PARAM_RESET_MEM, "Reset captured phrases");
}

void RaphralicModule::process(const ProcessArgs& args) {
}

RaphralicWidget::RaphralicWidget(RaphralicModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "raphralic") {
	addInput(createInputCentered<NTPort>(Vec(32.5f, 47.f), module, RaphralicModule::IN_CLOCK));
	addInput(createInputCentered<NTPort>(Vec(72.5f, 47.f), module, RaphralicModule::IN_GATE));
	addInput(createInputCentered<NTPort>(Vec(112.5f, 47.f), module, RaphralicModule::IN_CV));
	addInput(createInputCentered<NTPort>(Vec(152.5f, 47.f), module, RaphralicModule::IN_SKIP));

	addParam(createParamCentered<VCVButton>(Vec(32.5f, 215.f), module, RaphralicModule::PARAM_RESET_POS));
	addParam(createParamCentered<VCVButton>(Vec(72.5f, 215.f), module, RaphralicModule::PARAM_RESET_MEM));

	addInput(createInputCentered<NTPort>(Vec(32.5f, 87.f), module, RaphralicModule::IN_RESET_POS));
	addInput(createInputCentered<NTPort>(Vec(72.5f, 87.f), module, RaphralicModule::IN_RESET_MEM));

	addParam(createParamCentered<Rogan1PWhite>(Vec(112.5f, 95.f), module, RaphralicModule::PARAM_CLUSTER_SIZE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(152.5f, 95.f), module, RaphralicModule::PARAM_PHRASE_SIZE));

	addParam(createParamCentered<Rogan1PWhite>(Vec(112.5f, 135.f), module, RaphralicModule::PARAM_CLUSTER_SLOTS));
	addParam(createParamCentered<Rogan1PWhite>(Vec(152.5f, 135.f), module, RaphralicModule::PARAM_PHRASE_SLOTS));

	addParam(createParamCentered<Rogan1PWhite>(Vec(32.5f, 175.f), module, RaphralicModule::PARAM_CLUSTER_CAPTURE_CHANCE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(72.5f, 175.f), module, RaphralicModule::PARAM_PHRASE_CAPTURE_CHANCE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(112.5f, 175.f), module, RaphralicModule::PARAM_CLUSTER_REPLAY_CHANCE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(152.5f, 175.f), module, RaphralicModule::PARAM_PHRASE_REPLAY_CHANCE));

	addOutput(createOutputCentered<NTPort>(Vec(97.f, 332.5f), module, RaphralicModule::OUT_GATE));
	addOutput(createOutputCentered<NTPort>(Vec(137.f, 332.5f), module, RaphralicModule::OUT_CV));
	addOutput(createOutputCentered<NTPort>(Vec(177.f, 332.5f), module, RaphralicModule::OUT_REPLAY));
}



Model* modelRaphralic = createModel<RaphralicModule, RaphralicWidget>("raphralic");