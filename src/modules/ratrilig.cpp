#include "modules/ratrilig.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"
#include "components/ratrilig-progress.hpp"


extern Model* modelRatrilig;


RatriligModule::RatriligModule() : m_ratriligCore(this) {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	m_ratriligProgress = nullptr;

	configInput(IN_GATE, "Clock");
	configInput(IN_RESET, "Clock");
	configInput(IN_CLUSTER_CHANCE, "Cluster chance CV");
	configInput(IN_GROUP_CHANCE, "Group chance CV");
	configInput(IN_PHRASE_CHANCE, "Phrase chance CV");
	configInput(IN_DENSITY, "Density CV");
	configInput(IN_CLUSTER_DENSITY, "Cluster gate density CV");
	configInput(IN_GROUP_DENSITY, "Group gate density factor CV");
	configInput(IN_PHRASE_DENSITY, "Phrase gate density factor CV");

	configOutput(OUT_GATE, "Gate");

	ParamQuantity* pq = configParam(PARAM_CLUSTER_SIZE, 1.f, 32.f, 8.f, "Gates per Cluster");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;
	pq = configParam(PARAM_GROUP_SIZE, 1.f, 32.f, 4.f, "Clusters per Group");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;
	pq = configParam(PARAM_PHRASE_SIZE, 1.f, 32.f, 4.f, "Groups per Phrase");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;

	configParam(PARAM_CLUSTER_CHANCE, 0.f, 100.f, 75.f, "Cluster enabled chance");
	configParam(PARAM_GROUP_CHANCE, 0.f, 100.f, 60.f, "Group enabled chance");
	configParam(PARAM_PHRASE_CHANCE, 0.f, 100.f, 100.f, "Phrase enabled chance");

	configParam(PARAM_DENSITY, 0.f, 100.f, 65.f, "Gate density");
	configParam(PARAM_CLUSTER_DENSITY_FACTOR, 0.f, 100.f, 10.f, "Gates in cluster density factor");
	configParam(PARAM_GROUP_DENSITY_FACTOR, 0.f, 100.f, 10.f, "Gates in group density factor");
	configParam(PARAM_PHRASE_DENSITY_FACTOR, 0.f, 100.f, 10.f, "Gates in phrase density factor");

	configParam(PARAM_CLUSTER_BIAS_AMOUNT, 0.f, 1.f, .3f, "Cluster bias");
	configParam(PARAM_CLUSTER_BIAS_DIRECTION, 0.f, 1.f, 0.f, "Cluster bias direction");
	configParam(PARAM_GROUP_BIAS_AMOUNT, 0.f, 1.f, .3f, "Group bias");
	configParam(PARAM_GROUP_BIAS_DIRECTION, 0.f, 1.f, 0.f, "Group bias direction");

	configButton(PARAM_TRIGGER, "Trigger");
	configButton(PARAM_RESET, "Reset");
}

void RatriligModule::process(const ProcessArgs& args) {
	RatriligData data;

	// Make sure the output polyphony is up to date
	updatePolyphony(false);

	// Check if the trigger button was pushed
	bool triggerPushed = m_buttonTrigger.process(params[PARAM_TRIGGER].getValue());
	bool resetPushed = m_buttonReset.process(params[PARAM_RESET].getValue());

	for (int channel = 0; channel < m_channelCount; channel++) {
		if ((m_inputReset[channel].process(inputs[IN_RESET].getVoltage(channel), 0.f, 1.f)) || (resetPushed)) {
			m_ratriligCore.reset(channel);
		}
		if ((m_inputTrigger[channel].process(inputs[IN_GATE].getVoltage(channel), 0.f, 1.f)) || (triggerPushed)) {
			data.density = params[PARAM_DENSITY].getValue() / 100.f;
			data.clusterSize = params[PARAM_CLUSTER_SIZE].getValue();
			data.clusterEnabledChance = params[PARAM_CLUSTER_CHANCE].getValue() / 100.f;
			data.clusterDensityFactor = params[PARAM_CLUSTER_DENSITY_FACTOR].getValue() / 100.f;
			data.groupSize = params[PARAM_GROUP_SIZE].getValue();
			data.groupEnabledChance = params[PARAM_GROUP_CHANCE].getValue() / 100.f;
			data.groupDensityFactor = params[PARAM_GROUP_DENSITY_FACTOR].getValue() / 100.f;
			data.phraseSize = params[PARAM_PHRASE_SIZE].getValue();
			data.phraseEnabledChance = params[PARAM_PHRASE_CHANCE].getValue() / 100.f;
			data.phraseDensityFactor = params[PARAM_PHRASE_DENSITY_FACTOR].getValue() / 100.f;
			data.clusterBiasAmount = params[PARAM_CLUSTER_BIAS_AMOUNT].getValue();
			data.clusterBiasDirection = params[PARAM_CLUSTER_BIAS_DIRECTION].getValue();
			data.groupBiasAmount = params[PARAM_GROUP_BIAS_AMOUNT].getValue();
			data.groupBiasDirection = params[PARAM_GROUP_BIAS_DIRECTION].getValue();
			m_ratriligCore.process(channel, data);
		}

		// Update the trigger output based on either the input trigger, or the trigger button pulse
		if (outputs[OUT_GATE].isConnected()) {
			if ((m_buttonTrigger.isHigh()) || (m_ratriligCore.isHigh(channel) && inputs[IN_GATE].getVoltage(channel) >= 1.f)) {
				outputs[OUT_GATE].setVoltage(10.f, channel);
			} else {
				outputs[OUT_GATE].setVoltage(0.f, channel);
			}
		}
	}
}

void RatriligModule::draw(const widget::Widget::DrawArgs& args) {
	if (m_ratriligProgress != nullptr) {
		m_ratriligProgress->setClusterCount(params[PARAM_CLUSTER_SIZE].getValue());
		m_ratriligProgress->setGroupCount(params[PARAM_GROUP_SIZE].getValue());
		m_ratriligProgress->setPhraseCount(params[PARAM_PHRASE_SIZE].getValue());
	}
}

void RatriligModule::valueChanged(int channel, int phrase, int group, int cluster, float value, bool enabled) {
	if ((m_ratriligProgress != nullptr) && (channel == 0)) {
		int clusterSize = params[PARAM_CLUSTER_SIZE].getValue();
		int groupSize = params[PARAM_GROUP_SIZE].getValue();
		m_ratriligProgress->setPositionValue(phrase * groupSize * clusterSize + group * clusterSize + cluster, enabled ? value : 0.f);
	}
}

void RatriligModule::clusterStarted(int channel) {

}

void RatriligModule::groupStarted(int channel) {

}

void RatriligModule::phraseStarted(int channel) {

}

void RatriligModule::setRatriligProgress(RatriligProgress* ratriligProgress) {
	m_ratriligProgress = ratriligProgress;
	if (m_ratriligProgress != nullptr) {
		m_ratriligProgress->setClusterCount(params[PARAM_CLUSTER_SIZE].getValue());
		m_ratriligProgress->setGroupCount(params[PARAM_GROUP_SIZE].getValue());
		m_ratriligProgress->setPhraseCount(params[PARAM_PHRASE_SIZE].getValue());
	}
}

void RatriligModule::updatePolyphony(bool forceUpdateOutputs) {
	int channels = std::max(inputs[IN_GATE].getChannels(), 1);

	// Make sure the output polyphony is up to date
	if ((forceUpdateOutputs) || (channels != m_channelCount)) {
		m_channelCount = channels;
		outputs[OUT_GATE].setChannels(m_channelCount);
	}
}

RatriligWidget::RatriligWidget(RatriligModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "ratrilig") {
	addInput(createInputCentered<NTPort>(Vec(32.5f, 47.f), module, RatriligModule::IN_GATE));
	addParam(createParamCentered<VCVButton>(Vec(32.5f, 87.f), module, RatriligModule::PARAM_TRIGGER));

	addInput(createInputCentered<NTPort>(Vec(32.5f, 127.f), module, RatriligModule::IN_RESET));
	addParam(createParamCentered<VCVButton>(Vec(32.5f, 167.f), module, RatriligModule::PARAM_RESET));

	addParam(createParamCentered<Rogan1PWhite>(Vec(32.5f, 227.f + 5.f), module, RatriligModule::PARAM_DENSITY));
	addInput(createInputCentered<NTPort>(Vec(32.5f, 267.f + 5.f), module, RatriligModule::IN_DENSITY));

	addParam(createParamCentered<Rogan1PWhite>(Vec(102.5f, 55.f), module, RatriligModule::PARAM_CLUSTER_SIZE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(102.5f, 97.5f), module, RatriligModule::PARAM_CLUSTER_CHANCE));
	// addInput(createInputCentered<NTPort>(Vec(102.5f, 140.f), module, RatriligModule::IN_CLUSTER_CHANCE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(102.5f, 140.f), module, RatriligModule::PARAM_CLUSTER_DENSITY_FACTOR));
	addInput(createInputCentered<NTPort>(Vec(102.5f, 180.f), module, RatriligModule::IN_CLUSTER_DENSITY));

	addParam(createParamCentered<Rogan1PWhite>(Vec(142.5f, 55.f), module, RatriligModule::PARAM_GROUP_SIZE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(142.5f, 97.5f), module, RatriligModule::PARAM_GROUP_CHANCE));
	// addInput(createInputCentered<NTPort>(Vec(142.5f, 140.f), module, RatriligModule::IN_GROUP_CHANCE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(142.5f, 140.f), module, RatriligModule::PARAM_GROUP_DENSITY_FACTOR));
	addInput(createInputCentered<NTPort>(Vec(142.5f, 180.f), module, RatriligModule::IN_GROUP_DENSITY));

	addParam(createParamCentered<Rogan1PWhite>(Vec(182.5f, 55.f), module, RatriligModule::PARAM_PHRASE_SIZE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(182.5f, 97.5f), module, RatriligModule::PARAM_PHRASE_CHANCE));
	// addInput(createInputCentered<NTPort>(Vec(182.5f, 140.f), module, RatriligModule::IN_PHRASE_CHANCE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(182.5f, 140.f), module, RatriligModule::PARAM_PHRASE_DENSITY_FACTOR));
	addInput(createInputCentered<NTPort>(Vec(182.5f, 180.f), module, RatriligModule::IN_PHRASE_DENSITY));

	addParam(createParamCentered<Trimpot>(Vec(95.5f, 267.f), module, RatriligModule::PARAM_CLUSTER_BIAS_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(122.5f, 267.f), module, RatriligModule::PARAM_CLUSTER_BIAS_DIRECTION));
	addParam(createParamCentered<Trimpot>(Vec(157.5f, 267.f), module, RatriligModule::PARAM_GROUP_BIAS_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(184.5f, 267.f), module, RatriligModule::PARAM_GROUP_BIAS_DIRECTION));

	addOutput(createOutputCentered<NTPort>(Vec(177.f, 332.5f), module, RatriligModule::OUT_GATE));

	RatriligProgress* ratriligProgress = createWidget<RatriligProgress>(Vec(14.5f, 316.5f));
	ratriligProgress->setSize(Vec(96.5f, 32.f));
	addChild(ratriligProgress);
	if (module != nullptr) {
		module->setRatriligProgress(ratriligProgress);
	}
}



Model* modelRatrilig = createModel<RatriligModule, RatriligWidget>("ratrilig");