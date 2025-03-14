#include "modules/timeseq.hpp"
#include "components/ntport.hpp"
#include <osdialog.h>

#define TO_CHANNEL_PORT_IDENTIFIER(channel, port) ((channel << 5) + port)
#define CHANNEL_FROM_CHANNEL_PORT_IDENTIFIER(identifier) (identifier >> 5)
#define PORT_FROM_CHANNEL_PORT_IDENTIFIER(identifier) (identifier & 0xF)


TimeSeqModule::TimeSeqModule() {
	m_timeSeqCore = new timeseq::TimeSeqCore(this, this, this);

	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configInput(IN_INPUTS + i, string::f("Input %d", i + 1));
		configOutput(OUT_OUTPUTS + i, string::f("Output %d", i + 1));
	}

	configInput(IN_RUN, "Run Input");
	configInput(IN_RESET, "Reset Input");
	configInput(IN_RATE, "Rate Input");

	configButton(PARAM_RUN, "Run");
	configButton(PARAM_RESET, "Reset");

	ParamQuantity* pq = configParam(PARAM_RATE, -10.f, 10.f, 1.f, "Rate");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;

	m_PortChannelChangeClockDivider.setDivision(48000 / 30);

	resetOutputs();
}

TimeSeqModule::~TimeSeqModule() {
	delete m_timeSeqCore;
}

json_t *TimeSeqModule::dataToJson() {
	json_t *rootJ = NTModule::dataToJson();
	if (m_script) {
		json_object_set_new(rootJ, "ntTimeSeqScript", json_string(m_script->c_str()));
	}
	return rootJ;
}

void TimeSeqModule::dataFromJson(json_t *rootJ) {
	NTModule::dataFromJson(rootJ);

	json_t *ntTimeSeqScript = json_object_get(rootJ, "ntTimeSeqScript");
	if (ntTimeSeqScript) {
		if (json_is_string(ntTimeSeqScript)) {
			loadScript(std::make_shared<std::string>(json_string_value(ntTimeSeqScript)));
		}
	}
}

void TimeSeqModule::process(const ProcessArgs& args) {
	if (m_timeSeqCore->getStatus() != timeseq::TimeSeqCore::Status::EMPTY) {
		bool runTriggered = m_buttonTrigger[TriggerId::TRIG_RUN].process(params[ParamId::PARAM_RUN].getValue()) || m_trigTriggers[TriggerId::TRIG_RUN].process(inputs[InputId::IN_RUN].getVoltage(), 0.f, 1.f);
		bool resetTriggered = m_buttonTrigger[TriggerId::TRIG_RESET].process(params[ParamId::PARAM_RESET].getValue()) || m_trigTriggers[TriggerId::TRIG_RESET].process(inputs[InputId::IN_RESET].getVoltage(), 0.f, 1.f);

		if (runTriggered) {
			switch (m_timeSeqCore->getStatus()) {
				case timeseq::TimeSeqCore::Status::IDLE:
				case timeseq::TimeSeqCore::Status::PAUSED:
					m_timeSeqCore->start();
					break;
				case timeseq::TimeSeqCore::Status::RUNNING:
					m_timeSeqCore->pause();
					break;
				case timeseq::TimeSeqCore::Status::EMPTY:
					break;
			}
		}

		if (resetTriggered) {
			lights[LightId::LIGHT_RESET].setBrightnessSmooth(1.f, .01f);
			resetOutputs();
			m_timeSeqCore->reset();
			m_timeSeqDisplay->m_voltagePoints.clear();
		}

		if (m_timeSeqCore->getStatus() == timeseq::TimeSeqCore::Status::RUNNING) {
			int rate = params[TimeSeqModule::ParamId::PARAM_RATE].getValue();
			if (rate < -1) {
				m_rateDivision++;
				if (m_rateDivision >= -rate) {
					m_rateDivision = 0;
					m_timeSeqCore->process();
				}
			} else if (rate > 1) {
				m_rateDivision = 0;
				for (int i = 0; i < rate; i++) {
					m_timeSeqCore->process();
				}
			} else {
				m_rateDivision = 0;
				m_timeSeqCore->process();
			}
		}
	}

	if ((m_timeSeqCore->getStatus() == timeseq::TimeSeqCore::Status::RUNNING) && (m_PortChannelChangeClockDivider.process())) {
		// Remove voltage points that haven't changed recently, and update & age those that are recent enough
		for (int i = m_timeSeqDisplay->m_voltagePoints.size() - 1; i >= 0; i--) {
			if (m_timeSeqDisplay->m_voltagePoints[i].age >= TIMESEQ_DISPLAY_WINDOW_SIZE * 2) {
				m_timeSeqDisplay->m_voltagePoints.erase(m_timeSeqDisplay->m_voltagePoints.begin() + i);
			} else {
				m_timeSeqDisplay->m_voltagePoints[i].voltage = m_outputVoltages[CHANNEL_FROM_CHANNEL_PORT_IDENTIFIER(m_timeSeqDisplay->m_voltagePoints[i].id)][PORT_FROM_CHANNEL_PORT_IDENTIFIER(m_timeSeqDisplay->m_voltagePoints[i].id)];
				m_timeSeqDisplay->m_voltagePoints[i].age++;
			}
		}

		// Update/add the voltage points for the recently changed ports
		if (m_changedPortChannelVoltages.size() > 0) {
			for (std::vector<int>::iterator it = m_changedPortChannelVoltages.begin(); it != m_changedPortChannelVoltages.end(); it++) {
				bool found = false;
				// See if the port&channel combination is already in the current list of voltage points
				for (std::vector<TimeSeqVoltagePoints>::iterator vpIt = m_timeSeqDisplay->m_voltagePoints.begin(); vpIt != m_timeSeqDisplay->m_voltagePoints.end(); vpIt++) {
					if (vpIt->id == *it) {
						// The voltage point is already in there, so it was already captured. Just reset its age.
						found = true;
						vpIt->age = 0;
						break;
					}
				}
				// It's a new voltage point
				if (!found)
				{
					if (m_timeSeqDisplay->m_voltagePoints.size() < 16) {
						// We haven't reached the limit of trackable voltages yet, so just add a new one to the list.
						m_timeSeqDisplay->m_voltagePoints.emplace_back(*it);
						TimeSeqVoltagePoints& voltagePoints = m_timeSeqDisplay->m_voltagePoints.back();
						voltagePoints.voltage = m_outputVoltages[CHANNEL_FROM_CHANNEL_PORT_IDENTIFIER(voltagePoints.id)][PORT_FROM_CHANNEL_PORT_IDENTIFIER(voltagePoints.id)];
					} else {
						// We have reached the limit of trackable voltages. See if there is one that hasn't updated in the last cycle (start from the end, i.e. the most recent changing one)
						for (std::vector<TimeSeqVoltagePoints>::reverse_iterator vpIt = m_timeSeqDisplay->m_voltagePoints.rbegin(); vpIt != m_timeSeqDisplay->m_voltagePoints.rend(); vpIt++) {
							if (vpIt->age > TIMESEQ_DISPLAY_WINDOW_SIZE) {
								// Replace this tracked output with the newly changed one
								vpIt->id = *it;
								vpIt->age = 0;
								vpIt->voltage = m_outputVoltages[CHANNEL_FROM_CHANNEL_PORT_IDENTIFIER(vpIt->id)][PORT_FROM_CHANNEL_PORT_IDENTIFIER(vpIt->id)];
								break;
							}
						}
					}
				}
			}
		}
		m_changedPortChannelVoltages.clear();
	}
}

void TimeSeqModule::draw(const widget::Widget::DrawArgs& args) {
	lights[LightId::LIGHT_SEGMENT_STARTED].setBrightnessSmooth(m_segmentStarted, .01f);
	m_segmentStarted = false;
	lights[LightId::LIGHT_TRIGGER_TRIGGERED].setBrightnessSmooth(m_triggerTriggered, .01f);
	m_triggerTriggered = false;

	lights[LightId::LIGHT_READY].setBrightnessSmooth((bool) m_script, .01f);
	lights[LightId::LIGHT_NOT_READY].setBrightnessSmooth(!(bool) m_script, .01f);
	lights[LightId::LIGHT_RUN].setBrightnessSmooth((m_timeSeqCore->getStatus() == timeseq::TimeSeqCore::RUNNING), .01f);
	lights[LightId::LIGHT_RESET].setBrightnessSmooth(0.f, .01f);

	int sampleRate = getSampleRate();
	uint32_t elapsedSamples = m_timeSeqCore->getElapsedSamples();
	int seconds = elapsedSamples / sampleRate;
	int minutes = seconds / 60;
	seconds -= minutes * 60;
	m_timeSeqDisplay->m_time = string::f("%02d:%02d", minutes, seconds);
}

void TimeSeqModule::onPortChange(const PortChangeEvent& e) {
	updateOutputs();
}

void TimeSeqModule::onSampleRateChange(const SampleRateChangeEvent& sampleRateChangeEvent) {
	m_timeSeqCore->reloadScript();
}


float TimeSeqModule::getInputPortVoltage(int index, int channel) {
	return inputs[InputId::IN_INPUTS + index].getVoltage(channel);
}

float TimeSeqModule::getOutputPortVoltage(int index, int channel) {
	return m_outputVoltages[index][channel];
}

float TimeSeqModule::getSampleRate() {
	return APP->engine->getSampleRate();
}

void TimeSeqModule::setOutputPortVoltage(int index, int channel, float voltage) {
	m_outputVoltages[index][channel] = voltage;
	outputs[OutputId::OUT_OUTPUTS + index].setVoltage(voltage, channel);

	int id = TO_CHANNEL_PORT_IDENTIFIER(index, channel);
	if (std::find(m_changedPortChannelVoltages.begin(), m_changedPortChannelVoltages.end(), id) == m_changedPortChannelVoltages.end()) {
		m_changedPortChannelVoltages.push_back(id);
	}
}

void TimeSeqModule::setOutputPortChannels(int index, int channels) {
	m_outputChannels[index] = channels;
	outputs[OutputId::OUT_OUTPUTS + index].setChannels(channels);
}

void TimeSeqModule::segmentStarted() {
	m_segmentStarted = true;
}

void TimeSeqModule::triggerTriggered() {
	m_triggerTriggered = true;
}

std::shared_ptr<std::string> TimeSeqModule::getScript() {
	return m_script;
}

std::string TimeSeqModule::loadScript(std::shared_ptr<std::string> script) {
	std::vector<timeseq::ValidationError> errors = m_timeSeqCore->loadScript(*script);

	m_lastScriptLoadErrors.clear();
	if (errors.size() == 0) {
		m_script = script;
		return std::string();
	} else {
		std::ostringstream errorMessage;

		for (const timeseq::ValidationError& error : errors) {
			m_lastScriptLoadErrors.emplace_back(error.location + " : " + error.message);
			if (m_lastScriptLoadErrors.size() == 1) {
				errorMessage << errors.size() << " error(s) were encountered while loading the script. The first error is:\n\n";
				errorMessage << error.location << " : " << error.message;
				errorMessage << "\n\nDo you want to copy all error details to the clipboard?.";
			}
		}

		return errorMessage.str();
	}
}

void TimeSeqModule::resetOutputs() {
	m_outputVoltages.fill({ 1.f });
	for (std::array<std::array<float, 16>, 8>::iterator it = m_outputVoltages.begin(); it != m_outputVoltages.end(); it++) {
		it->fill(0.f);
	}
	m_outputChannels.fill(1);
	updateOutputs();
}

void TimeSeqModule::updateOutputs() {
	for (int i = 0; i < 8; i++) {
		outputs[OutputId::OUT_OUTPUTS + i].setChannels(m_outputChannels[i]);
		for (int j = 0; j < m_outputChannels[i]; j++) {
			outputs[OutputId::OUT_OUTPUTS + i].setVoltage(m_outputVoltages[i][j], j);
		}
	}
}

void TimeSeqModule::clearScript() {
	m_timeSeqCore->clearScript();
	m_script.reset();
}

std::list<std::string>& TimeSeqModule::getLastScriptLoadErrors() {
	return m_lastScriptLoadErrors;
}


TimeSeqWidget::TimeSeqWidget(TimeSeqModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "timeseq") {
	float xIn = 24;
	float xOut = 126.f+60.f+15.f;
	float y = 41.5;
	float yDelta = 40;
	for (int i = 0; i < 8; i++) {
		addInput(createInputCentered<NTPort>(Vec(xIn, y), module, TimeSeqModule::IN_INPUTS + i));
		addOutput(createOutputCentered<NTPort>(Vec(xOut, y), module, TimeSeqModule::OUT_OUTPUTS + i));
		y += yDelta;
	}

	addParam(createLightParamCentered<LEDLightBezel<RedLight>>(Vec(64.5f+7.5f, 88.f+11.5f), module, TimeSeqModule::PARAM_RUN, TimeSeqModule::LIGHT_RUN));
	addInput(createInputCentered<NTPort>(Vec(64.5f+7.5f, 41.5f+11.5f), module, TimeSeqModule::IN_RUN));
	addOutput(createOutputCentered<NTPort>(Vec(64.5f+7.5f, 121.5f+11.5f), module, TimeSeqModule::OUT_RUN));
	addParam(createLightParamCentered<LEDLightBezel<RedLight>>(Vec(105.f+7.5f, 88.f+11.5f), module, TimeSeqModule::PARAM_RESET, TimeSeqModule::LIGHT_RESET));
	addInput(createInputCentered<NTPort>(Vec(105.f+7.5f, 41.5f+11.5f), module, TimeSeqModule::IN_RESET));
	addOutput(createOutputCentered<NTPort>(Vec(105.f+7.5f, 121.5f+11.5f), module, TimeSeqModule::OUT_RESET));
	addParam(createParamCentered<RoundSmallBlackKnob>(Vec(145.5f+7.5f, 88.f+11.5f), module, TimeSeqModule::PARAM_RATE));
	addInput(createInputCentered<NTPort>(Vec(145.5f+7.5f, 41.5f+11.5f), module, TimeSeqModule::IN_RATE));

	addChild(createLightCentered<TinyLight<GreenRedLight>>(Vec(49.5f+7.5f, 64.5f+11.5f), module, TimeSeqModule::LightId::LIGHT_READY));
	addChild(createLightCentered<TinyLight<GreenLight>>(Vec(xIn + 34.f, 322.5f), module, TimeSeqModule::LightId::LIGHT_SEGMENT_STARTED));
	addChild(createLightCentered<TinyLight<GreenLight>>(Vec(xIn + (34.f * 2), 322.5f), module, TimeSeqModule::LightId::LIGHT_TRIGGER_TRIGGERED));

	TimeSeqDisplay* timeSeqDisplay = createWidget<TimeSeqDisplay>(Vec(53.5f, 156.f));
	timeSeqDisplay->box.size = Vec(118.f, 195.f);
	addChild(timeSeqDisplay);
	if (module != nullptr) {
		module->m_timeSeqDisplay = timeSeqDisplay;
	}

}

void TimeSeqWidget::appendContextMenu(Menu* menu) {
	NTModuleWidget::appendContextMenu(menu);

	bool disabled = !hasScript();
	menu->addChild(new MenuSeparator);
	menu->addChild(createMenuItem("Load script...", "", [this]() { this->loadScript(); }));
	menu->addChild(createMenuItem("Save script...", "", [this]() { this->saveScript(); }, disabled));
	menu->addChild(createMenuItem("Clear script...", "", [this]() { this->clearScript(); }, disabled));
}

void TimeSeqWidget::loadScript() {
	// If a script is already loaded, first confirm if it should be replaced.
	if ((!hasScript()) || (osdialog_message(OSDIALOG_ERROR, OSDIALOG_YES_NO, "A script is already loaded. Are you sure you want to load a new script?") == 1)) {
		osdialog_filters* filters = osdialog_filters_parse("JSON Files (*.json):json;All Files (*.*):*");
		char* path = osdialog_file(OSDIALOG_OPEN, "", "", filters);
		osdialog_filters_free(filters);
		if (path) {
			std::vector<uint8_t> data = system::readFile(path);
			free(path);
			std::string json = std::string(data.begin(), data.end());

			TimeSeqModule* timeSeqModule = dynamic_cast<TimeSeqModule *>(getModule());
			if (timeSeqModule != nullptr) {
				std::string error = timeSeqModule->loadScript(std::make_shared<std::string>(json));
				if (error.length() > 0) {
					if (osdialog_message(OSDIALOG_ERROR, OSDIALOG_YES_NO, error.c_str()) == 1) {
						copyLastLoadErrors();
					}
				}
			}
		}
	}
}

void TimeSeqWidget::saveScript() {
}

void TimeSeqWidget::clearScript() {
	if (osdialog_message(OSDIALOG_WARNING, OSDIALOG_YES_NO, "Are you sure you want to clear the currently loaded script?") == 1) {
		dynamic_cast<TimeSeqModule *>(getModule())->clearScript();
	}
}

void TimeSeqWidget::copyLastLoadErrors() {
	TimeSeqModule* timeSeqModule = dynamic_cast<TimeSeqModule *>(getModule());
	if (timeSeqModule != nullptr) {
		std::list<std::string>& loadErrors = timeSeqModule->getLastScriptLoadErrors();
		if (loadErrors.size() > 0) {
			std::ostringstream errorMessage;
			for (const std::string& error : loadErrors) {
				if (errorMessage.tellp() != 0) {
					errorMessage << "\n";
				}
				errorMessage << error;
			}
			glfwSetClipboardString(APP->window->win, errorMessage.str().c_str());
		}
	}
}

bool TimeSeqWidget::hasScript() {
	return getModule() ? (bool) dynamic_cast<TimeSeqModule *>(getModule())->getScript() : false;
}


Model* modelTimeSeq = createModel<TimeSeqModule, TimeSeqWidget>("timeseq");
