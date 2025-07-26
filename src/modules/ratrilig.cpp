#include "modules/ratrilig.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"
#include "components/ratrilig-progress.hpp"
#include "components/ratrilig-bias.hpp"
#include "components/ratrilig-probability.hpp"


extern Model* modelRatrilig;
extern Model* modelRatriligExpander;


RatriligModule::RatriligModule() : m_ratriligCore(this) {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configInput(IN_GATE, "Clock");
	configInput(IN_RESET, "Clock");
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

	configParam(PARAM_CLUSTER_CHANCE, 0.f, 100.f, 20.f, "Cluster skip chance");
	configParam(PARAM_GROUP_CHANCE, 0.f, 100.f, 15.f, "Group skip chance");
	configParam(PARAM_PHRASE_CHANCE, 0.f, 100.f, 0.f, "Phrase skip chance");

	configParam(PARAM_DENSITY, 0.f, 100.f, 55.f, "Gate density");
	configParam(PARAM_CLUSTER_DENSITY_FACTOR, 0.f, 100.f, 15.f, "Gates in cluster density factor");
	configParam(PARAM_GROUP_DENSITY_FACTOR, 0.f, 100.f, 10.f, "Gates in group density factor");
	configParam(PARAM_PHRASE_DENSITY_FACTOR, 0.f, 100.f, 10.f, "Gates in phrase density factor");

	configParam(PARAM_CLUSTER_BIAS_AMOUNT, 0.f, 1.f, .2f, "Cluster bias");
	configParam(PARAM_CLUSTER_BIAS_DIRECTION, 0.f, 1.f, 0.f, "Cluster bias direction");
	configParam(PARAM_GROUP_BIAS_AMOUNT, 0.f, 1.f, .15f, "Group bias");
	configParam(PARAM_GROUP_BIAS_DIRECTION, 0.f, 1.f, 0.f, "Group bias direction");

	configButton(PARAM_TRIGGER, "Trigger");
	configButton(PARAM_RESET, "Reset");
}

void RatriligModule::process(const ProcessArgs& args) {
	RatriligData data;
	Module* expander = getRatriligExpander();

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
			data.density = getValue(PARAM_DENSITY, IN_DENSITY, channel) / 100.f;
			data.clusterSize = params[PARAM_CLUSTER_SIZE].getValue();
			data.clusterSkipChance = getValue(PARAM_CLUSTER_CHANCE, expander, RatriligExpanderModule::InputId::IN_SKIP_CLUSTER, channel) / 100.f;
			data.clusterDensityFactor = getValue(PARAM_CLUSTER_DENSITY_FACTOR, IN_CLUSTER_DENSITY, channel) / 100.f;
			data.groupSize = params[PARAM_GROUP_SIZE].getValue();
			data.groupSkipChance = getValue(PARAM_GROUP_CHANCE, expander, RatriligExpanderModule::InputId::IN_SKIP_GROUP, channel) / 100.f;
			data.groupDensityFactor = getValue(PARAM_GROUP_DENSITY_FACTOR, IN_GROUP_DENSITY, channel) / 100.f;
			data.phraseSize = params[PARAM_PHRASE_SIZE].getValue();
			data.phraseSkipChance = getValue(PARAM_PHRASE_CHANCE, expander, RatriligExpanderModule::InputId::IN_SKIP_PHRASE, channel) / 100.f;
			data.phraseDensityFactor = getValue(PARAM_PHRASE_DENSITY_FACTOR, IN_PHRASE_DENSITY, channel) / 100.f;
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
	int clusterSize = params[PARAM_CLUSTER_SIZE].getValue();
	int groupSize = params[PARAM_GROUP_SIZE].getValue();
	int phraseSize = params[PARAM_PHRASE_SIZE].getValue();

	if (m_ratriligProgress != nullptr) {
		m_ratriligProgress->setClusterCount(clusterSize);
		m_ratriligProgress->setGroupCount(groupSize);
		m_ratriligProgress->setPhraseCount(phraseSize);
	}
	if (m_ratriligClusterBias != nullptr) {
		m_ratriligClusterBias->setBias(params[PARAM_CLUSTER_BIAS_DIRECTION].getValue(), groupSize);
	}
	if (m_ratriligGroupBias != nullptr) {
		m_ratriligGroupBias->setBias(params[PARAM_GROUP_BIAS_DIRECTION].getValue(), phraseSize);
	}
	if (m_ratriligClusterProbability != nullptr) {
		m_ratriligClusterProbability->setDensityFactor(getValue(PARAM_CLUSTER_DENSITY_FACTOR, IN_CLUSTER_DENSITY, 0) / 100.f);
	}
	if (m_ratriligGroupProbability != nullptr) {
		m_ratriligGroupProbability->setDensityFactor(getValue(PARAM_GROUP_DENSITY_FACTOR, IN_GROUP_DENSITY, 0) / 100.f);
	}
	if (m_ratriligPhraseProbability != nullptr) {
		m_ratriligPhraseProbability->setDensityFactor(getValue(PARAM_PHRASE_DENSITY_FACTOR, IN_PHRASE_DENSITY, 0) / 100.f);
	}
}


void RatriligModule::clusterStateChanged(int channel, bool enabled, float density, float bias) {
	if ((channel == 0) && (m_ratriligClusterProbability != nullptr)) {
		m_ratriligClusterProbability->setEnabled(enabled);
		m_ratriligClusterProbability->setDensity(density);
		m_ratriligClusterProbability->setBias(bias);
	}
}

void RatriligModule::groupStateChanged(int channel, bool enabled, float density, float bias) {
	if ((channel == 0) && (m_ratriligGroupProbability != nullptr)) {
		m_ratriligGroupProbability->setEnabled(enabled);
		m_ratriligGroupProbability->setDensity(density);
		m_ratriligGroupProbability->setBias(bias);
	}
}

void RatriligModule::phraseStateChanged(int channel, bool enabled, float density) {
	if ((channel == 0) && (m_ratriligPhraseProbability != nullptr)) {
		m_ratriligPhraseProbability->setEnabled(enabled);
		m_ratriligPhraseProbability->setDensity(density);
	}
}

void RatriligModule::valueChanged(int channel, int phrase, int group, int cluster, float target, float value, bool enabled) {
	if ((channel == 0) && (m_ratriligProgress != nullptr)) {
		int clusterSize = params[PARAM_CLUSTER_SIZE].getValue();
		int groupSize = params[PARAM_GROUP_SIZE].getValue();
		m_ratriligProgress->setPositionValue(phrase * groupSize * clusterSize + group * clusterSize + cluster, enabled ? target : 0.f);
	}
	if ((channel == 0) && (m_ratriligGlobalProbability != nullptr)) {
		m_ratriligGlobalProbability->setDensity(target);
		m_ratriligGlobalProbability->setDensityFactor(value);
		m_ratriligGlobalProbability->setEnabled(enabled);
	}
}

void RatriligModule::clusterStarted(int channel) {
	Module* expander = getRatriligExpander();
	if (expander != nullptr) {
		dynamic_cast<RatriligExpanderModule*>(expander)->triggerCluster();
	}
}

void RatriligModule::groupStarted(int channel) {
	Module* expander = getRatriligExpander();
	if (expander != nullptr) {
		dynamic_cast<RatriligExpanderModule*>(expander)->triggerGroup();
	}
}

void RatriligModule::phraseStarted(int channel) {
	Module* expander = getRatriligExpander();
	if (expander != nullptr) {
		dynamic_cast<RatriligExpanderModule*>(expander)->triggerPhrase();
	}
}

void RatriligModule::setRatriligProgress(RatriligProgress* ratriligProgress) {
	m_ratriligProgress = ratriligProgress;
	if (m_ratriligProgress != nullptr) {
		m_ratriligProgress->setClusterCount(params[PARAM_CLUSTER_SIZE].getValue());
		m_ratriligProgress->setGroupCount(params[PARAM_GROUP_SIZE].getValue());
		m_ratriligProgress->setPhraseCount(params[PARAM_PHRASE_SIZE].getValue());
	}
}

void RatriligModule::setRatriligClusterBias(RatriligBias* ratriligBias) {
	m_ratriligClusterBias = ratriligBias;
}

void RatriligModule::setRatriligGroupBias(RatriligBias* ratriligBias) {
	m_ratriligGroupBias = ratriligBias;
}

void RatriligModule::setRatriligClusterProbability(RatriligProbability* ratriligClusterProbability) {
	m_ratriligClusterProbability = ratriligClusterProbability;
}

void RatriligModule::setRatriligGroupProbability(RatriligProbability* ratriligGroupProbability) {
	m_ratriligGroupProbability = ratriligGroupProbability;
}

void RatriligModule::setRatriligPhraseProbability(RatriligProbability* ratriligPhraseProbability) {
	m_ratriligPhraseProbability = ratriligPhraseProbability;
}

void RatriligModule::setRatriligGlobalProbability(RatriligProbability* ratriligGlobalProbability) {
	m_ratriligGlobalProbability = ratriligGlobalProbability;
}

void RatriligModule::updatePolyphony(bool forceUpdateOutputs) {
	int channels = std::max(inputs[IN_GATE].getChannels(), 1);

	// Make sure the output polyphony is up to date
	if ((forceUpdateOutputs) || (channels != m_channelCount)) {
		m_channelCount = channels;
		outputs[OUT_GATE].setChannels(m_channelCount);
	}
}

float RatriligModule::getValue(ParamId paramId, InputId inputId, int channel) {
	float value = params[paramId].getValue();
	if (inputs[inputId].isPolyphonic()) {
		value += inputs[inputId].getVoltage(channel) * 20.f;
	} else if (inputs[inputId].isMonophonic()) {
		value += inputs[inputId].getVoltage(0) * 20.f;
	}

	return (value > 100.f) ? 100.f : (value < 0.f) ? 0.f : value;
}

float RatriligModule::getValue(ParamId paramId, Module* expander, RatriligExpanderModule::InputId inputId, int channel) {
	float value = params[paramId].getValue();
	if (expander != nullptr) {
		if (expander->inputs[inputId].isPolyphonic()) {
			value += expander->inputs[inputId].getVoltage(channel) * 20.f;
		} else if (expander->inputs[inputId].isMonophonic()) {
			value += expander->inputs[inputId].getVoltage(0) * 20.f;
		}
	}

	return (value > 100.f) ? 100.f : (value < 0.f) ? 0.f : value;
}

Module* RatriligModule::getRatriligExpander() {
	Expander& expander = getRightExpander();
	if ((expander.module != nullptr) && (expander.module->getModel() == modelRatriligExpander)) {
		return expander.module;
	}
	expander = getLeftExpander();
	if ((expander.module != nullptr) && (expander.module->getModel() == modelRatriligExpander)) {
		return expander.module;
	}
	return nullptr;
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
	addParam(createParamCentered<Rogan1PWhite>(Vec(102.5f, 140.f), module, RatriligModule::PARAM_CLUSTER_DENSITY_FACTOR));
	addInput(createInputCentered<NTPort>(Vec(102.5f, 180.f), module, RatriligModule::IN_CLUSTER_DENSITY));

	addParam(createParamCentered<Rogan1PWhite>(Vec(142.5f, 55.f), module, RatriligModule::PARAM_GROUP_SIZE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(142.5f, 97.5f), module, RatriligModule::PARAM_GROUP_CHANCE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(142.5f, 140.f), module, RatriligModule::PARAM_GROUP_DENSITY_FACTOR));
	addInput(createInputCentered<NTPort>(Vec(142.5f, 180.f), module, RatriligModule::IN_GROUP_DENSITY));

	addParam(createParamCentered<Rogan1PWhite>(Vec(182.5f, 55.f), module, RatriligModule::PARAM_PHRASE_SIZE));
	addParam(createParamCentered<Rogan1PWhite>(Vec(182.5f, 97.5f), module, RatriligModule::PARAM_PHRASE_CHANCE));
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

	RatriligBias* ratriligClusterBias = createWidget<RatriligBias>(Vec(84.f, 290.f));
	ratriligClusterBias->setSize(Vec(50.f, 8.f));
	addChild(ratriligClusterBias);
	if (module != nullptr) {
		module->setRatriligClusterBias(ratriligClusterBias);
	}

	RatriligBias* ratriligGroupBias = createWidget<RatriligBias>(Vec(146.f, 290.f));
	ratriligGroupBias->setSize(Vec(50.f, 8.f));
	addChild(ratriligGroupBias);
	if (module != nullptr) {
		module->setRatriligGroupBias(ratriligGroupBias);
	}

	RatriligProbability* ratriligClusterProbability = createWidget<RatriligProbability>(Vec(86.5f, 205.f));
	ratriligClusterProbability->setBidirectional(true);
	ratriligClusterProbability->setSize(Vec(32.f, 32.f));
	addChild(ratriligClusterProbability);
	if (module != nullptr) {
		module->setRatriligClusterProbability(ratriligClusterProbability);
	}

	RatriligProbability* ratriligGroupProbability = createWidget<RatriligProbability>(Vec(126.5f, 205.f));
	ratriligGroupProbability->setBidirectional(true);
	ratriligGroupProbability->setSize(Vec(32.f, 32.f));
	addChild(ratriligGroupProbability);
	if (module != nullptr) {
		module->setRatriligGroupProbability(ratriligGroupProbability);
	}

	RatriligProbability* ratriligPhraseProbability = createWidget<RatriligProbability>(Vec(166.5f, 205.f));
	ratriligPhraseProbability->setBidirectional(true);
	ratriligPhraseProbability->setSize(Vec(32.f, 32.f));
	addChild(ratriligPhraseProbability);
	if (module != nullptr) {
		module->setRatriligPhraseProbability(ratriligPhraseProbability);
	}

	RatriligProbability* ratriligGlobalProbability = createWidget<RatriligProbability>(Vec(119.f, 316.5f));
	ratriligGlobalProbability->setBidirectional(false);
	ratriligGlobalProbability->setSize(Vec(32.f, 32.f));
	addChild(ratriligGlobalProbability);
	if (module != nullptr) {
		module->setRatriligGlobalProbability(ratriligGlobalProbability);
	}
}



Model* modelRatrilig = createModel<RatriligModule, RatriligWidget>("ratrilig");