#include "modules/solim.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"
#include "modules/solim-input.hpp"
#include "modules/solim-input-octaver.hpp"
#include "modules/solim-output.hpp"
#include "modules/solim-output-octaver.hpp"
#include "util/notes.hpp"

extern Model* modelSolimInput;
extern Model* modelSolimInputOctaver;
extern Model* modelSolimOutput;
extern Model* modelSolimOutputOctaver;
extern Model* modelSolimRandom;


inline int getCvFlag(std::vector<Input>& inputs, int inputIndex, std::vector<Param>& params, int paramIndex, int channel) {
	float result;
	int channels = inputs[inputIndex].getChannels();
	if (channels > channel) {
		result = inputs[inputIndex].getVoltage(channel);
	} else if (channels > 0) {
		result = inputs[inputIndex].getVoltage(channels - 1);
	} else {
		result = params[paramIndex].getValue();
	}

	return (result > 0) - (result < 0);
}


SolimModule::SolimModule() : SolimModule(new SolimCore()) {
}

SolimModule::SolimModule(SolimCore* solimCore) : m_solimCore(solimCore) {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configInput(IN_INPUTS + i, string::f("Input %d", i + 1));
		configOutput(OUT_OUTPUTS + i, string::f("Output %d", i + 1));
		configBypass(IN_INPUTS + i, OUT_OUTPUTS + i);
	}

	configParam(PARAM_LOWER_LIMIT, -5.f, 5., 0.f, "Lower Limit", " v/oct");
	configInput(IN_LOWER_LIMIT, "Lower Limit CV");
	configParam(PARAM_UPPER_LIMIT, -5.f, 5., 1.f, "Upper Limit", " v/oct");
	configInput(IN_UPPER_LIMIT, "Upper Limit CV");

	configSwitch(PARAM_SORT, -1.0, 1.0, 0.0, "Sort", {"Descending", "None", "Ascending"});
	configInput(IN_SORT, "Sort CV");

	m_clockDivider.setDivision(12);
}

SolimModule::~SolimModule() {
	delete m_solimCore;
}

json_t *SolimModule::dataToJson() {
	json_t *rootJ = NTModule::dataToJson();
	json_object_set_new(rootJ, "ntSolimProcessRate", json_integer(m_processRate));
	json_object_set_new(rootJ, "ntSolimOutputMode", json_integer(m_outputMode));
	return rootJ;
}

void SolimModule::dataFromJson(json_t *rootJ) {
	NTModule::dataFromJson(rootJ);

	json_t *ntSolimProcessRateJson = json_object_get(rootJ, "ntSolimProcessRate");
	if (ntSolimProcessRateJson) {
		json_int_t processRateNumber = json_integer_value(ntSolimProcessRateJson);
		if (processRateNumber > 0 && processRateNumber < ProcessRate::RATE_COUNT) {
			setProcessRate(static_cast<ProcessRate>(processRateNumber));
		} else {
			setProcessRate(ProcessRate::DIVIDED);
		}
	}

	json_t *ntSolimOutputModeJson = json_object_get(rootJ, "ntSolimOutputMode");
	if (ntSolimOutputModeJson) {
		json_int_t outputModeNumber = json_integer_value(ntSolimOutputModeJson);
		if (outputModeNumber > 0 && outputModeNumber < SolimOutputMode::NUM_OUTPUT_MODES) {
			setOutputMode(static_cast<SolimOutputMode>(outputModeNumber));
		} else {
			setOutputMode(SolimOutputMode::OUTPUT_MODE_MONOPHONIC);
		}
	}
}

void SolimModule::process(const ProcessArgs& args) {
	#ifdef __NT_DEBUG__
		if (outputs[OUT_DEBUG_PROCESS_DURATION].isConnected()) {
		 	m_avgDuration.start();
		}
	#endif

	// Determine if we actually process in this invocation based on the process rate and clock divider
	bool process = false;
	if (m_processRate == ProcessRate::DIVIDED) {
		process = m_clockDivider.process();
	} else {
		process = true;
	}

	if (process) {
		detectExpanders();
		readValues();
		// If there is a randomizer module, check if there have been randomize triggers
		if (m_solimExpanders.solimRandom != nullptr) {
			for (int i = 0; i < 8; i++) {
				RandomTrigger randomTrigger = RandomTrigger::NONE;
				if (m_lastRandom) { // Only check for random trigger changes if the module wasn't just added to the chain
					if (m_solimExpanders.solimRandom->m_resetCounters[i] != m_lastRandResetCounters[i]) {
						randomTrigger = RandomTrigger::RESET;
					} else if (m_solimExpanders.solimRandom->m_allCounters[i] != m_lastRandAllCounters[i]) {
						randomTrigger = RandomTrigger::ALL;
					} else if (m_solimExpanders.solimRandom->m_oneCounters[i] != m_lastRandOneCounters[i]) {
						randomTrigger = RandomTrigger::ONE;
					} else if (m_solimExpanders.solimRandom->m_moveCounters[i] != m_lastRandMoveCounters[i]) {
						randomTrigger = RandomTrigger::MOVE;
					}
				}
				m_lastRandom = true;
				m_lastRandResetCounters[i] = m_solimExpanders.solimRandom->m_resetCounters[i];
				m_lastRandAllCounters[i] = m_solimExpanders.solimRandom->m_allCounters[i];
				m_lastRandOneCounters[i] = m_solimExpanders.solimRandom->m_oneCounters[i];
				m_lastRandMoveCounters[i] = m_solimExpanders.solimRandom->m_moveCounters[i];
				m_randomTriggers[i] = randomTrigger;
			}
		} else {
			m_lastRandom = false;
		}
		m_solimCore->processAndActivateInactiveValues(m_solimExpanders.activeOutputs, m_lastRandom ? &m_randomTriggers : nullptr);
		writeValues();
	}

	#ifdef __NT_DEBUG__
		if (outputs[OUT_DEBUG_PROCESS_DURATION].isConnected()) {
			m_avgDuration.stop();
			outputs[OUT_DEBUG_PROCESS_DURATION].setVoltage(avgDuration.getAvgDuration());
		}
	#endif
}

void SolimModule::draw(const widget::Widget::DrawArgs& args) {
	int limit;
	float lowerLimit = m_solimCore->getActiveValues(0).lowerLimit;
	float upperLimit = m_solimCore->getActiveValues(0).upperLimit;
	if (m_upperDisplay != nullptr && m_lastUpperDisplayed != upperLimit) {
		limit = static_cast<int>(upperLimit + 5);
		if (upperLimit - (limit - 5) > 11.5f / 12) {
			// If the calculated limit is almost a full voltage off from the original value, the quantize jumped over an octave boundary, so add that octave.
			limit++;
		}
		m_upperDisplay->setScale(limit < 0 ? 0 : limit > 9 ? 9 : limit);
		m_upperDisplay->setNote(voltageToChromaticIndex(upperLimit));
		m_lastUpperDisplayed = upperLimit;
	}
	if (m_lowerDisplay != nullptr && m_lastLowerDisplayed != lowerLimit) {
		limit = static_cast<int>(lowerLimit + 5);
		if (lowerLimit - (limit - 5) > 11.5f / 12) {
			// If the calculated limit is almost a full voltage off from the original value, the quantize jumped over an octave boundary, so add that octave.
			limit++;
		}
		m_lowerDisplay->setScale(limit < 0 ? 0 : limit > 9 ? 9 : limit);
		m_lowerDisplay->setNote(voltageToChromaticIndex(lowerLimit));
		m_lastLowerDisplayed = lowerLimit;
	}
}

void SolimModule::onSampleRateChange(const SampleRateChangeEvent& sampleRateChangeEvent) {
	m_clockDivider.setDivision(sampleRateChangeEvent.sampleRate / 6000);
}

void SolimModule::onPortChange(const PortChangeEvent& event) {
	for (int i = 0; i < 8; i++) {
		m_connectedPorts[i] = outputs[i].isConnected();
	}
}

SolimModule::ProcessRate SolimModule::getProcessRate() {
	return m_processRate;
}
void SolimModule::setProcessRate(SolimModule::ProcessRate processRate) {
	m_processRate = processRate;
}

SolimOutputMode SolimModule::getOutputMode() {
	return m_outputMode;
}

void SolimModule::setOutputMode(SolimOutputMode outputMode) {
	m_outputMode = outputMode;
}

std::array<bool, 8>& SolimModule::getConnectedPorts() {
	return m_connectedPorts;
}

float SolimModule::getCvOrParamVoltage(InputId inputId, ParamId paramId, int channel) {
	float result;
	int channels = inputs[inputId].getChannels();
	if (channels > channel) {
		result = inputs[inputId].getVoltage(channel);
	} else if (channels > 0) {
		result = inputs[inputId].getVoltage(channels - 1);
	} else {
		result = params[paramId].getValue();
	}

	return result > 5.0f ? 5.0f : result < -5.0f ? -5.0f : result;
}

void SolimModule::detectExpanders() {
	m_solimExpanders.solimRandom = nullptr;
	m_solimExpanders.solimInputOctaver = nullptr;
	m_solimExpanders.solimOutputOctaver = nullptr;

	m_solimExpanders.inputCount = 1;
	m_solimExpanders.inputIterators[0] = inputs.begin() + IN_INPUTS;
	Expander* expanderModule = &getLeftExpander();
	while ((expanderModule->module) && ((m_solimExpanders.inputCount < 8) || (m_solimExpanders.solimRandom == nullptr) || (m_solimExpanders.solimInputOctaver == nullptr))) {
		if ((expanderModule->module->getModel() == modelSolimInput) && (m_solimExpanders.inputCount < 8)) {
			m_solimExpanders.inputIterators[m_solimExpanders.inputCount] = expanderModule->module->inputs.begin() + SolimInputModule::InputId::IN_INPUTS;
			m_solimExpanders.inputCount++;
		} else if ((m_solimExpanders.solimRandom == nullptr) && (expanderModule->module->getModel() == modelSolimRandom)) {
			m_solimExpanders.solimRandom = reinterpret_cast<SolimRandomModule*>(expanderModule->module);
		} else if ((m_solimExpanders.solimInputOctaver == nullptr) && (expanderModule->module->getModel() == modelSolimInputOctaver)) {
			m_solimExpanders.solimInputOctaver = expanderModule->module;
		} else {
			break;
		}

		expanderModule = &expanderModule->module->getLeftExpander();
	}

	m_solimExpanders.outputCount = 1;
	m_solimExpanders.outputIterators[0] = outputs.begin() + OUT_OUTPUTS;
	m_solimExpanders.outputModes[0] = m_outputMode;
	m_solimExpanders.lightIterators[0] = lights.begin() + OUT_LIGHTS;
	m_solimExpanders.polyphonicLights[0] = &lights[LightId::OUT_POLYPHONIC_LIGHT];
	m_solimExpanders.connectedOutputPorts[0] = &getConnectedPorts();
	expanderModule = &getRightExpander();
	while ((expanderModule->module) && ((m_solimExpanders.outputCount < 8) || (m_solimExpanders.solimRandom == nullptr))) {
		if ((expanderModule->module->getModel() == modelSolimOutput) && (m_solimExpanders.outputCount < 8)) {
			m_solimExpanders.outputIterators[m_solimExpanders.outputCount] = expanderModule->module->outputs.begin() + SolimOutputModule::OutputId::OUT_OUTPUTS;
			m_solimExpanders.outputModes[m_solimExpanders.outputCount] = reinterpret_cast<SolimOutputModule*>(expanderModule->module)->getOutputMode();
			m_solimExpanders.lightIterators[m_solimExpanders.outputCount] = expanderModule->module->lights.begin() + SolimOutputModule::LightId::OUT_LIGHTS;
			m_solimExpanders.polyphonicLights[m_solimExpanders.outputCount] = &expanderModule->module->lights[SolimOutputModule::LightId::OUT_POLYPHONIC_LIGHT];
			m_solimExpanders.connectedOutputPorts[m_solimExpanders.outputCount] = &reinterpret_cast<SolimOutputModule*>(expanderModule->module)->getConnectedPorts();
			m_solimExpanders.outputCount++;
		} else if ((m_solimExpanders.solimRandom == nullptr) && (expanderModule->module->getModel() == modelSolimRandom)) {
			m_solimExpanders.solimRandom = reinterpret_cast<SolimRandomModule*>(expanderModule->module);
		} else if ((m_solimExpanders.solimOutputOctaver == nullptr) && (expanderModule->module->getModel() == modelSolimOutputOctaver)) {
			m_solimExpanders.solimOutputOctaver  = expanderModule->module;
		} else {
			break;
		}

		expanderModule = &expanderModule->module->getRightExpander();
	}

	m_solimExpanders.activeOutputs = (m_solimExpanders.inputCount < m_solimExpanders.outputCount) ? m_solimExpanders.inputCount : m_solimExpanders.outputCount;
}

void SolimModule::readValues() {
	for (int channelIndex = 0; channelIndex < m_solimExpanders.activeOutputs; channelIndex++) {
		SolimValueSet& values = m_solimCore->getInactiveValues(channelIndex);

		values.lowerLimit = getCvOrParamVoltage(IN_LOWER_LIMIT, PARAM_LOWER_LIMIT, channelIndex);
		values.upperLimit = getCvOrParamVoltage(IN_UPPER_LIMIT, PARAM_UPPER_LIMIT, channelIndex);
		float sortf = getCvOrParamVoltage(IN_SORT, PARAM_SORT, channelIndex);
		values.sort = (sortf > 0) - (sortf < 0);

		std::vector<Input>::iterator inputIterator = m_solimExpanders.inputIterators[channelIndex];

		int channelCount = 0;
		values.inputValueCount = 0;
		float *polyVoltages = nullptr;
		for (int valueIndex = 0; valueIndex < 8; valueIndex++) {
			Input& input = *(inputIterator + valueIndex);
			if (input.isConnected()) {
				values.inputValues[values.inputValueCount].value = input.getVoltage();
				values.inputValues[values.inputValueCount].replaceOriginal = false;
				values.inputValues[values.inputValueCount].addOctave = SolimValue::AddOctave::NONE;
				values.inputValueCount++;

				// If no polyVoltages is set yet (i.e. this is the first connected input), retrieve the channel count and
				// polyphonic voltages for this input. A single connected polyphonic input will also work as multi-input
				if (!polyVoltages) {
					channelCount = input.getChannels();
					polyVoltages = input.getVoltages();
				}
			}
		}

		// If there is only one input connected and it is polyphonic, use up to 8 values from the poly input.
		if (values.inputValueCount == 1 && channelCount > 0) {
			values.inputValueCount = channelCount > 8 ? 8 : channelCount;
			for (int i = 0; i < values.inputValueCount; i++) {
				values.inputValues[i].value = polyVoltages[i];
				values.inputValues[i].replaceOriginal = false;
				values.inputValues[i].addOctave = SolimValue::AddOctave::NONE;
			}
		}

		// If there is an input octaver expander, loop through the constructed input values, and add their input octave values
		if (m_solimExpanders.solimInputOctaver != nullptr) {
			for (int i = 0; i < values.inputValueCount; i++) {
				values.inputValues[i].replaceOriginal = getCvFlag(
					m_solimExpanders.solimInputOctaver->inputs, SolimInputOctaverModule::IN_REPLACE_ORIGINAL + i,
					m_solimExpanders.solimInputOctaver->params, SolimInputOctaverModule::PARAM_REPLACE_ORIGINAL + i,
					channelIndex) > 0.f;
				values.inputValues[i].addOctave = static_cast<SolimValue::AddOctave>(getCvFlag(
					m_solimExpanders.solimInputOctaver->inputs, SolimInputOctaverModule::IN_ADD_OCTAVE + i,
					m_solimExpanders.solimInputOctaver->params, SolimInputOctaverModule::PARAM_ADD_OCTAVE + i,
					channelIndex));
				values.inputValues[i].sortRelative = static_cast<SolimValue::SortRelative>(getCvFlag(
					m_solimExpanders.solimInputOctaver->inputs, SolimInputOctaverModule::IN_SORT_POSITION + i,
					m_solimExpanders.solimInputOctaver->params, SolimInputOctaverModule::PARAM_SORT_POSITION + i,
					channelIndex));
			}
		}

		// If there is an output octaver expander, assign the output octave shifts and whether the outut is replaced/removed
		if (m_solimExpanders.solimOutputOctaver != nullptr) {
			for (int i = 0; i < 8; i++) {
				values.outputOctaves[i] = static_cast<SolimValue::AddOctave>(getCvFlag(
					m_solimExpanders.solimOutputOctaver->inputs, SolimOutputOctaverModule::IN_ADD_OCTAVE + i,
					m_solimExpanders.solimOutputOctaver->params, SolimOutputOctaverModule::PARAM_ADD_OCTAVE + i,
					channelIndex));
				values.outputReplaceOriginal[i] = getCvFlag(
					m_solimExpanders.solimOutputOctaver->inputs, SolimOutputOctaverModule::IN_REPLACE_ORIGINAL + i,
					m_solimExpanders.solimOutputOctaver->params, SolimOutputOctaverModule::PARAM_REPLACE_ORIGINAL + i,
					channelIndex) > 0.f;
			}
			if (m_solimExpanders.solimOutputOctaver->params[SolimOutputOctaverModule::PARAM_RESORT].getValue() > 0.f) {
				if (m_solimExpanders.outputModes[channelIndex] == SolimOutputMode::OUTPUT_MODE_MONOPHONIC) {
					// Detect which resort mode should be used if the output is monophonic
					values.resortMode = reinterpret_cast<SolimOutputOctaverModule *>(m_solimExpanders.solimOutputOctaver)->getSortMode() == SolimOutputOctaverModule::SortMode::SORT_ALL ? SolimValueSet::ResortMode::RESORT_ALL : SolimValueSet::ResortMode::RESORT_CONNECTED;
				} else {
					// Polyphonic output is always resorted across all channels
					values.resortMode = SolimValueSet::ResortMode::RESORT_ALL;
				}
			} else {
				values.resortMode = SolimValueSet::ResortMode::RESORT_NONE;
			}
			// If the resort mode is based on the which outputs are connected, assign the connected output ports.
			values.outputConnected = m_solimExpanders.connectedOutputPorts[channelIndex];
		} else {
			values.outputOctaves.fill(SolimValue::AddOctave::NONE);
			values.outputReplaceOriginal.fill(false);
			values.resortMode = SolimValueSet::ResortMode::RESORT_NONE;
		}
	}
}

void SolimModule::writeValues() {
	for (int channelIndex = 0; channelIndex < m_solimExpanders.activeOutputs; channelIndex++) {
		std::vector<Output>::iterator& outputIterator = m_solimExpanders.outputIterators[channelIndex];
		std::vector<Light>::iterator& lightIterator = m_solimExpanders.lightIterators[channelIndex];
		SolimValueSet& values = m_solimCore->getActiveValues(channelIndex);

		if (m_solimExpanders.outputModes[channelIndex] == SolimOutputMode::OUTPUT_MODE_MONOPHONIC) {
			// Make sure the first output is set to monophonic, and dim the polyphonic light
			(*(outputIterator)).setChannels(1);
			m_solimExpanders.polyphonicLights[channelIndex]->setBrightness(0.f);
			// Set the result voltages on the different outputs
			for (int i = 0; i < values.resultValueCount; i++) {
				(*(outputIterator + i)).setVoltage(values.resultValues[i]);
				(*(lightIterator + i)).setBrightness(1.f);
			}
			// Disable the remaining outputs
			for (int i = values.resultValueCount; i < 8; i++) {
				(*(outputIterator + i)).setVoltage(0.f);
				(*(lightIterator + i)).setBrightness(0.f);
			}
		} else {
			// Set the first output to polyphonic, matching the channel count to the number of outputs
			(*(outputIterator)).setChannels(values.resultValueCount);
			// Turn on the polyphonic light
			m_solimExpanders.polyphonicLights[channelIndex]->setBrightness(1.f);

			// Set the polyphonic voltages on the first output
			for (int i = 0; i < values.resultValueCount; i++) {
				(*(outputIterator)).setVoltage(values.resultValues[i], i);
			}
			// Disable the other outputs, and dim the non-polyphonic led of the first output.
			(*(lightIterator)).setBrightness(0.f);
			for (int i = 1; i < 8; i++) {
				(*(outputIterator + i)).setVoltage(0.f);
				(*(lightIterator + i)).setBrightness(0.f);
			}
		}
	}
	// Disable the output indications on the remaining outputs
	for (int channelIndex = m_solimExpanders.activeOutputs; channelIndex < m_solimExpanders.outputCount; channelIndex++) {
		std::vector<Output>::iterator& outputIterator = m_solimExpanders.outputIterators[channelIndex];
		std::vector<Light>::iterator& lightIterator = m_solimExpanders.lightIterators[channelIndex];
		for (int i = 0; i < 8; i++) {
			(*(outputIterator + i)).setVoltage(0.f);
			(*(lightIterator + i)).setBrightness(0.f);
		}
	}
}


SolimWidget::SolimWidget(SolimModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "solim") {
	float xIn = 24;
	float xOut = 126;
	float y = 41.5;
	float yDelta = 40;
	for (int i = 0; i < 8; i++) {
		addInput(createInputCentered<NTPort>(Vec(xIn, y), module, SolimModule::IN_INPUTS + i));
		addOutput(createOutputCentered<NTPort>(Vec(xOut, y), module, SolimModule::OUT_OUTPUTS + i));
		if (i == 0) {
			addChild(createLightCentered<TinyLight<BlueGreenLight>>(Vec(xOut + 12.5, y + 12.5), module, SolimModule::OUT_POLYPHONIC_LIGHT));
		} else {
			addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(xOut + 12.5, y + 12.5), module, SolimModule::OUT_LIGHTS + i));
		}
		y += yDelta;
	}

	addParam(createParamCentered<Rogan2PWhite>(Vec(75, 83), module, SolimModule::PARAM_LOWER_LIMIT));
	addInput(createInputCentered<NTPort>(Vec(75, 121.5), module, SolimModule::IN_LOWER_LIMIT));

	addParam(createParamCentered<Rogan2PWhite>(Vec(75, 203), module, SolimModule::PARAM_UPPER_LIMIT));
	addInput(createInputCentered<NTPort>(Vec(75, 241.5), module, SolimModule::IN_UPPER_LIMIT));

	addParam(createParamCentered<CKSSThree>(Vec(57.5, 296), module, SolimModule::PARAM_SORT));
	addInput(createInputCentered<NTPort>(Vec(75, 334.5), module, SolimModule::IN_SORT));

	NoteDisplay* lowerDisplay = createWidget<NoteDisplay>(Vec(56.25f, 42.75f));
	lowerDisplay->box.size = Vec(34.5f, 17.5f);
	lowerDisplay->setScale(4);
	lowerDisplay->setNote(0);
	addChild(lowerDisplay);
	if (module != nullptr) {
		module->m_lowerDisplay = lowerDisplay;
	}

	NoteDisplay* upperDisplay = createWidget<NoteDisplay>(Vec(56.25f, 162.75f));
	upperDisplay->box.size = Vec(34.5f, 17.5f);
	upperDisplay->setScale(5);
	upperDisplay->setNote(0);
	addChild(upperDisplay);
	if (module != nullptr) {
		module->m_upperDisplay = upperDisplay;
	}

	#ifdef __NT_DEBUG__
		addOutput(createOutputCentered<NTPort>(Vec(26, 365.5), module, SolimModule::OUT_DEBUG_PROCESS_DURATION));
	#endif

}

void SolimWidget::appendContextMenu(Menu* menu) {
	NTModuleWidget::appendContextMenu(menu);

	SolimModule::ProcessRate processRate = getModule() ? dynamic_cast<SolimModule *>(getModule())->getProcessRate() : SolimModule::ProcessRate::DIVIDED;
	menu->addChild(createCheckMenuItem("Process at audio rate", "", [processRate]() { return processRate == SolimModule::ProcessRate::AUDIO; }, [this]() { switchProcessRate(); }));

	SolimOutputMode outputMode = getModule() ? dynamic_cast<SolimModule *>(getModule())->getOutputMode() : SolimOutputMode::OUTPUT_MODE_MONOPHONIC;
	menu->addChild(createCheckMenuItem("Polyphonic output", "", [outputMode]() { return outputMode == SolimOutputMode::OUTPUT_MODE_POLYPHONIC; }, [this]() { switchOutputMode(); }));
}

void SolimWidget::switchProcessRate() {
	SolimModule* solimModule = dynamic_cast<SolimModule *>(getModule());
	if (solimModule != nullptr) {
		if (solimModule->getProcessRate() == SolimModule::ProcessRate::DIVIDED) {
			solimModule->setProcessRate(SolimModule::ProcessRate::AUDIO);
		} else {
			solimModule->setProcessRate(SolimModule::ProcessRate::DIVIDED);
		}
	}
}

void SolimWidget::switchOutputMode() {
	SolimModule* solimModule = dynamic_cast<SolimModule *>(getModule());
	if (solimModule != nullptr) {
		if (solimModule->getOutputMode() == SolimOutputMode::OUTPUT_MODE_POLYPHONIC) {
			solimModule->setOutputMode(SolimOutputMode::OUTPUT_MODE_MONOPHONIC);
		} else {
			solimModule->setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);
		}
	}
}



Model* modelSolim = createModel<SolimModule, SolimWidget>("solim");
