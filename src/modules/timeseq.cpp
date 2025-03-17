#include "modules/timeseq.hpp"
#include "components/ntport.hpp"
#include "components/timeseq-display.hpp"
#include "components/leddisplay.hpp"
#include <osdialog.h>

#define TO_CHANNEL_PORT_IDENTIFIER(channel, port) ((channel << 5) + port)


TimeSeqModule::TimeSeqModule() {
	m_timeSeqCore = new timeseq::TimeSeqCore(this, this, this);

	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configInput(IN_INPUTS + i, string::f("Input %d", i + 1));
		configOutput(OUT_OUTPUTS + i, string::f("Output %d", i + 1));
	}

	configInput(IN_RUN, "Run");
	configInput(IN_RESET, "Reset");
	configInput(IN_RATE, "Rate");

	configButton(PARAM_RUN, "Run");
	configButton(PARAM_RESET, "Reset");
	configButton(PARAM_RESET_CLOCK, "Reset Clock");

	configOutput(OUT_RUN, "Run");
	configOutput(OUT_RESET, "Reset");

	configLight(LIGHT_READY, "Ready status");
	configLight(LIGHT_LANE_LOOPED, "Lane loop");
	configLight(LIGHT_SEGMENT_STARTED, "Segment start");
	configLight(LIGHT_TRIGGER_TRIGGERED, "Trigger triggered");

	ParamQuantity* pq = configParam(PARAM_RATE, -10.f, 10.f, 1.f, "Rate");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;

	m_PortChannelChangeClockDivider.setDivision(48000 / 30);

	resetUi();
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
	// Process any triggers that may have happened
	if (m_buttonTrigger[TriggerId::TRIG_RESET_CLOCK].process(params[ParamId::PARAM_RESET_CLOCK].getValue())) {
		m_timeSeqCore->resetElapsedSamples();
	}

	if (m_timeSeqCore->getStatus() != timeseq::TimeSeqCore::Status::EMPTY) {
		bool runTriggered = m_buttonTrigger[TriggerId::TRIG_RUN].process(params[ParamId::PARAM_RUN].getValue()) || m_trigTriggers[TriggerId::TRIG_RUN].process(inputs[InputId::IN_RUN].getVoltage(), 0.f, 1.f);
		bool resetTriggered = m_buttonTrigger[TriggerId::TRIG_RESET].process(params[ParamId::PARAM_RESET].getValue()) || m_trigTriggers[TriggerId::TRIG_RESET].process(inputs[InputId::IN_RESET].getVoltage(), 0.f, 1.f);

		// Change the run state if needed
		if (runTriggered) {
			switch (m_timeSeqCore->getStatus()) {
				case timeseq::TimeSeqCore::Status::IDLE:
				case timeseq::TimeSeqCore::Status::PAUSED:
					m_timeSeqCore->start();
					m_runPulse.trigger(0.001f);
					break;
				case timeseq::TimeSeqCore::Status::RUNNING:
					m_timeSeqCore->pause();
					m_runPulse.trigger(0.001f);
					break;
				case timeseq::TimeSeqCore::Status::EMPTY:
					// Can't run if no script is loaded
					break;
			}
		}

		// Perform a reset
		if (resetTriggered) {
			lights[LightId::LIGHT_RESET].setBrightnessSmooth(1.f, .01f);
			m_resetPulse.trigger(0.001f);
			
			resetUi();
			m_timeSeqCore->reset();
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
		if (m_changedPortChannelVoltages.size() > 0) {
			m_timeSeqDisplay->processChangedVoltages(m_changedPortChannelVoltages, m_outputVoltages);
		}
		m_changedPortChannelVoltages.clear();
	}

	// Update the Run and Reset outputs
	outputs[OutputId::OUT_RUN].setVoltage((m_runPulse.process(args.sampleTime) ? 10.0f : 0.0f));
	outputs[OutputId::OUT_RESET].setVoltage((m_resetPulse.process(args.sampleTime) ? 10.0f : 0.0f));
}

void TimeSeqModule::draw(const widget::Widget::DrawArgs& args) {
	lights[LightId::LIGHT_LANE_LOOPED].setBrightnessSmooth(m_laneLooped, .01f);
	m_laneLooped = false;
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
	m_ledDisplay->setForegroundText(string::f("%02d:%02d", minutes, seconds));
}

void TimeSeqModule::onPortChange(const PortChangeEvent& e) {
	updateOutputs();
}

void TimeSeqModule::onSampleRateChange(const SampleRateChangeEvent& sampleRateChangeEvent) {
	m_timeSeqCore->reloadScript();
	resetUi();
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

void TimeSeqModule::laneLooped() {
	m_laneLooped = true;
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
		resetUi();
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

void TimeSeqModule::resetUi() {
	resetOutputs();
	if (m_timeSeqDisplay) {
		m_timeSeqDisplay->reset();
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
	resetUi();
}

std::list<std::string>& TimeSeqModule::getLastScriptLoadErrors() {
	return m_lastScriptLoadErrors;
}


TimeSeqWidget::TimeSeqWidget(TimeSeqModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "timeseq") {
	float xIn = 24;
	float xOut = 126.f+60.f+15.f-45.f+15.f;
	float y = 41.5;
	float yDelta = 40;
	for (int i = 0; i < 8; i++) {
		addInput(createInputCentered<NTPort>(Vec(xIn, y), module, TimeSeqModule::IN_INPUTS + i));
		addOutput(createOutputCentered<NTPort>(Vec(xOut, y), module, TimeSeqModule::OUT_OUTPUTS + i));
		y += yDelta;
	}

	addInput(createInputCentered<NTPort>(Vec(68.f+7.5f, 66.5f), module, TimeSeqModule::IN_RUN));
	addParam(createLightParamCentered<LEDLightBezel<RedLight>>(Vec(68.f+7.5f, 113.f), module, TimeSeqModule::PARAM_RUN, TimeSeqModule::LIGHT_RUN));
	addOutput(createOutputCentered<NTPort>(Vec(68.f+7.5f, 146.5f), module, TimeSeqModule::OUT_RUN));
	addInput(createInputCentered<NTPort>(Vec(112.f+7.5f, 206.5f-140.f), module, TimeSeqModule::IN_RESET));
	addParam(createLightParamCentered<LEDLightBezel<RedLight>>(Vec(112.f+7.5f, 253.f-140.f), module, TimeSeqModule::PARAM_RESET, TimeSeqModule::LIGHT_RESET));
	addOutput(createOutputCentered<NTPort>(Vec(112.f+7.5f, 286.5f-140.f), module, TimeSeqModule::OUT_RESET));
	addInput(createInputCentered<NTPort>(Vec(68.f+7.5f, 66.5f+135.f), module, TimeSeqModule::IN_RATE));
	addParam(createParamCentered<RoundSmallBlackKnob>(Vec(68.f+7.5f, 112.f+135.f), module, TimeSeqModule::PARAM_RATE));

	addChild(createLightCentered<TinyLight<GreenRedLight>>(Vec(53.5f+7.5f, 90.f), module, TimeSeqModule::LightId::LIGHT_READY));
	addChild(createLightCentered<SmallLight<GreenLight>>(Vec(53.5f+7.5f, 347.5f), module, TimeSeqModule::LightId::LIGHT_LANE_LOOPED));
	addChild(createLightCentered<SmallLight<GreenLight>>(Vec(68.f+7.5f, 347.5f), module, TimeSeqModule::LightId::LIGHT_SEGMENT_STARTED));
	addChild(createLightCentered<SmallLight<GreenLight>>(Vec(82.5f+7.5f, 347.5f), module, TimeSeqModule::LightId::LIGHT_TRIGGER_TRIGGERED));

	addChild(createParamCentered<VCVButton>(Vec(92.5f + 10.f -2.f + 23.f - 44.f + 7.5f, 315.f), module, TimeSeqModule::PARAM_RESET_CLOCK));

	TimeSeqDisplay* timeSeqDisplay = createWidget<TimeSeqDisplay>(Vec(92.5f+7.5f, 174.f));
	timeSeqDisplay->setSize(Vec(39.f, 178.f));
	addChild(timeSeqDisplay);
	if (module != nullptr) {
		module->m_timeSeqDisplay = timeSeqDisplay;
	}

	LEDDisplay* ledDisplay = new LEDDisplay(nvgRGB(0xFF, 0x50, 0x50), nvgRGB(0x40, 0x40, 0x40), "88:88", 10, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, true);
	ledDisplay->setPosition(Vec(47.5f+7.5f+.25, 284.f+.25));
	ledDisplay->setSize(Vec(41.f, 18.f));
	addChild(ledDisplay);
	if (module != nullptr) {
		module->m_ledDisplay = ledDisplay;
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
			try {
				std::vector<uint8_t> data = system::readFile(path);
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
			} catch (Exception& e) {
				osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, string::f("Unexpected error: %s", e.msg.c_str()).c_str());
			}
			free(path);
		}
	}
}

void TimeSeqWidget::saveScript() {
	TimeSeqModule* timeSeqModule = dynamic_cast<TimeSeqModule *>(getModule());
	if ((timeSeqModule) && (timeSeqModule->getScript()) ) {
		std::string script = *timeSeqModule->getScript();
		if (script.size() > 0) {
			osdialog_filters* filters = osdialog_filters_parse("JSON Files (*.json):json;All Files (*.*):*");
			char* path = osdialog_file(OSDIALOG_SAVE, "", "", filters);
			osdialog_filters_free(filters);
			if (path) {
				std::string file = path;
				std::string ext = ".json";
				if ((ext.size() > file.size()) || (!std::equal(ext.rbegin(), ext.rend(), file.rbegin()))) {
					file = file + ext;
				}
				try {
					system::writeFile(file.c_str(), std::vector<uint8_t>(script.begin(), script.end()));
				} catch (Exception& e) {
					osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, string::f("Unexpected error: %s", e.msg.c_str()).c_str());
				}
				free(path);
			}
		}
	}
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
