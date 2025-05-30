#include "modules/timeseq.hpp"
#include "components/ntport.hpp"
#include "components/timeseq-display.hpp"
#include "components/leddisplay.hpp"
#include "components/lights.hpp"
#include <osdialog.h>

#define TO_CHANNEL_PORT_IDENTIFIER(channel, port) ((channel << 5) + port)


TimeSeqModule::TimeSeqModule() {
	m_timeSeqCore = new timeseq::TimeSeqCore(this, this, this, this);

	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configInput(IN_INPUTS + i, string::f("Input %d", i + 1));
	}

	configInput(IN_RUN, "Run");
	configInput(IN_RESET, "Reset");
	configInput(IN_RATE, "Rate");

	configButton(PARAM_RUN, "Run");
	configButton(PARAM_RESET, "Reset");
	configButton(PARAM_RESET_CLOCK, "Reset Clock");

	configOutput(OUT_RUN, "Run");
	configOutput(OUT_RESET, "Reset");

	configLight(LIGHT_LANE_LOOPED, "Lane loop");
	configLight(LIGHT_SEGMENT_STARTED, "Segment start");
	configLight(LIGHT_TRIGGER_TRIGGERED, "Trigger triggered");

	ParamQuantity* pq = configParam(PARAM_RATE, -10.f, 10.f, 1.f, "Rate");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;

	m_portChannelChangeClockDivider.setDivision(48000 / 15);

	resetUi();
}

TimeSeqModule::~TimeSeqModule() {
	delete m_timeSeqCore;
}

json_t *TimeSeqModule::dataToJson() {
	json_t *rootJ = NTModule::dataToJson();
	if (m_script) {
		json_object_set_new(rootJ, "ntTimeSeqScript", json_string(m_script->c_str()));
	} else {
		json_object_set_new(rootJ, "ntTimeSeqScript", json_string(""));
	}
	json_object_set_new(rootJ, "ntTimeSeqStatus", json_integer(m_timeSeqCore->getStatus()));
	return rootJ;
}

void TimeSeqModule::dataFromJson(json_t *rootJ) {
	NTModule::dataFromJson(rootJ);

	json_t *ntTimeSeqScript = json_object_get(rootJ, "ntTimeSeqScript");
	if (ntTimeSeqScript) {
		if (json_is_string(ntTimeSeqScript)) {
			const char* script = json_string_value(ntTimeSeqScript);
			if (strlen(script) > 0) {
				std::shared_ptr<std::string> script = std::make_shared<std::string>(json_string_value(ntTimeSeqScript));
				std::string result = loadScript(script);
				if ((result.length() > 0) && (!m_script)) {
					// If the loading failed, and there is no script present yet, keep a copy of the script data so the user can copy it.
					// It's most likely a script that was saved in a patch, and is no longer valid for some reason, and we shouldn't just dump it.
					m_script = script;
				}
			} else {
				clearScript();
			}
		}
	}

	json_t *ntTimeSeqStatus = json_object_get(rootJ, "ntTimeSeqStatus");
	if (ntTimeSeqStatus) {
		json_int_t status = json_integer_value(ntTimeSeqStatus);
		if (status == timeseq::TimeSeqCore::Status::RUNNING) {
			m_timeSeqCore->start(10); // Introduce a 10 sample delay in the processing when a start is received from the data JSON to allow any other setup in the patch to complete.
		}
	}
}

void TimeSeqModule::process(const ProcessArgs& args) {
	// Reset the timer if requested
	if (m_buttonTrigger[TriggerId::TRIG_RESET_CLOCK].process(params[ParamId::PARAM_RESET_CLOCK].getValue())) {
		m_timeSeqCore->resetElapsedSamples();
	}

	bool runTriggered = m_buttonTrigger[TriggerId::TRIG_RUN].process(params[ParamId::PARAM_RUN].getValue()) || m_trigTriggers[TriggerId::TRIG_RUN].process(inputs[InputId::IN_RUN].getVoltage(), 0.f, 1.f);
	bool resetTriggered = m_buttonTrigger[TriggerId::TRIG_RESET].process(params[ParamId::PARAM_RESET].getValue()) || m_trigTriggers[TriggerId::TRIG_RESET].process(inputs[InputId::IN_RESET].getVoltage(), 0.f, 1.f);

	// Change the run state if needed
	if (runTriggered) {
		m_runPulse.trigger(0.001f);
		if (m_timeSeqCore->getStatus() == timeseq::TimeSeqCore::Status::RUNNING) {
			// If the core is running, we should pause it now
			m_timeSeqCore->pause();
		} else {
			// Otherwise (if it is paused or idle), we should start it
			m_timeSeqCore->start(0);
		}
	}

	// Perform a reset
	if (resetTriggered) {
		lights[LightId::LIGHT_RESET].setBrightnessSmooth(1.f, .01f);
		m_resetPulse.trigger(0.001f);

		m_timeSeqCore->reset();
	}

	// Check the rate to see how many process calls should actually be done on the core.
	int rate = getRate();
	if (rate < -1) {
		// We're running at slower-then-real-time, so check the rateDivision if we should advance by one cycle on the core
		m_rateDivision++;
		if (m_rateDivision >= -rate) {
			m_rateDivision = 0;
			m_timeSeqCore->process(1);
		}
	} else if (rate > 1) {
		// We're going faster-then-real-time, so advance the core with multiple cycles.
		m_rateDivision = 0;
		m_timeSeqCore->process(rate);
	} else {
		// anything from -1 to 1 is considered real-time, so advance a single cycle.
		m_rateDivision = 0;
		m_timeSeqCore->process(1);
	}

	// Check if we should update the UI visualization of the changed voltages in this cycle
	if ((m_timeSeqCore->getStatus() == timeseq::TimeSeqCore::Status::RUNNING) && (m_portChannelChangeClockDivider.process())) {
		if (m_timeSeqDisplay != nullptr) {
			if (m_changedPortChannelVoltages.size() > 0) {
				// If there are changed voltages since the last time we checked, apply them now.
				m_timeSeqDisplay->processChangedVoltages(m_changedPortChannelVoltages, m_outputVoltages);
				m_changedPortChannelVoltages.clear();
			} else {
				// No port voltages have changed, so just age the existing voltages.
				m_timeSeqDisplay->ageVoltages();
			}
		}
	}

	// Update the Run and Reset outputs
	outputs[OutputId::OUT_RUN].setVoltage((m_runPulse.process(args.sampleTime) ? 10.0f : 0.0f));
	outputs[OutputId::OUT_RESET].setVoltage((m_resetPulse.process(args.sampleTime) ? 10.0f : 0.0f));
}

void TimeSeqModule::draw(const widget::Widget::DrawArgs& args) {
	// Update the event LEDs
	lights[LightId::LIGHT_LANE_LOOPED].setBrightnessSmooth(m_laneLooped, .01f);
	m_laneLooped = false;
	lights[LightId::LIGHT_SEGMENT_STARTED].setBrightnessSmooth(m_segmentStarted, .01f);
	m_segmentStarted = false;
	lights[LightId::LIGHT_TRIGGER_TRIGGERED].setBrightnessSmooth(m_triggerTriggered, .01f);
	m_triggerTriggered = false;

	// Update the status LEDs
	lights[LightId::LIGHT_RUN].setBrightnessSmooth((m_timeSeqCore->getStatus() == timeseq::TimeSeqCore::RUNNING), .01f, 20.f);
	lights[LightId::LIGHT_RESET].setBrightnessSmooth(0.f, .01f, 20.f);

	// Update the time display
	if (m_timeSeqCore->getStatus() != timeseq::TimeSeqCore::Status::EMPTY) {
		int sampleRate = getSampleRate();
		uint32_t elapsedSamples = m_timeSeqCore->getElapsedSamples();
		int seconds = elapsedSamples / sampleRate;
		int minutes = seconds / 60;
		seconds -= minutes * 60;
		m_ledDisplay->setForegroundText(string::f("%02d:%02d", minutes, seconds));
	} else {
		m_ledDisplay->setForegroundText("--:--");
	}
}

void TimeSeqModule::onPortChange(const PortChangeEvent& e) {
	// If one of the output ports gets (dis)connected, re-apply the port polyphony and voltages in case they were reset.
	updateOutputs();
}

void TimeSeqModule::onSampleRateChange(const SampleRateChangeEvent& sampleRateChangeEvent) {
	if (sampleRateChangeEvent.sampleRate != m_timeSeqCore->getCurrentSampleRate()) {
		// Reload the script to recalculate based on the new sample rate and reset the UI
		m_timeSeqCore->reloadScript();
		// Recalculate how many samples to leave between updates of the voltage display (to get 30fps)
		m_portChannelChangeClockDivider.setDivision(sampleRateChangeEvent.sampleRate / 30);
	}
}

void TimeSeqModule::onRemove(const RemoveEvent& e) {
	m_timeSeqDisplay = nullptr;
	m_ledDisplay = nullptr;
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

	for (int j = 0; j < m_outputChannels[index]; j++) {
		outputs[OutputId::OUT_OUTPUTS + index].setVoltage(m_outputVoltages[index][j], j);
	}
}

void TimeSeqModule::setOutputPortLabel(int index, std::string& label) {
	configOutput(OUT_OUTPUTS + index, label);
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

void TimeSeqModule::scriptReset() {
	resetUi();
}

void TimeSeqModule::assertFailed(std::string& name, std::string& message, bool stop) {
	// Update the display (if needed)
	if (m_timeSeqDisplay != nullptr) {
		m_timeSeqDisplay->setAssert(true);
	}

	// We'll only keep the first 25 assert failures in memory.
	if (m_failedAsserts.size() < 25) {
		m_failedAsserts.push_back(string::f("Assert '%s' failed due to expectation '%s'.", name.c_str(), message.c_str()));
	}

	// If it's an assert that also stops the running state, do so now.
	if ((stop) && (m_timeSeqCore->getStatus() == timeseq::TimeSeqCore::Status::RUNNING)) {
		m_timeSeqCore->pause();
	}
}

std::shared_ptr<std::string> TimeSeqModule::getScript() {
	return m_script;
}

std::string TimeSeqModule::loadScript(std::shared_ptr<std::string> script) {
	std::vector<timeseq::ValidationError> errors = m_timeSeqCore->loadScript(*script);

	m_lastScriptLoadErrors.clear();
	if (errors.size() == 0) {
		setDisplayScriptError(false);
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

		if (m_timeSeqCore->getStatus() != timeseq::TimeSeqCore::Status::EMPTY) {
			setDisplayScriptError(false);
		} else {
			setDisplayScriptError(true);
		}
		return errorMessage.str();
	}
}

void TimeSeqModule::resetUi() {
	resetOutputs();
	m_failedAsserts.clear();
	if (m_timeSeqDisplay) {
		m_timeSeqDisplay->reset();
		m_timeSeqDisplay->setAssert(false);
	}
}

void TimeSeqModule::resetOutputs() {
	m_outputVoltages.fill({ 1.f });
	for (std::array<std::array<float, 16>, 8>::iterator it = m_outputVoltages.begin(); it != m_outputVoltages.end(); it++) {
		it->fill(0.f);
	}
	m_outputChannels.fill(1);
	updateOutputs();
	for (int i = 0; i < 8; i++) {
		configOutput(OUT_OUTPUTS + i, string::f("Output %d", i + 1));
	}
}

void TimeSeqModule::updateOutputs() {
	for (int i = 0; i < 8; i++) {
		outputs[OutputId::OUT_OUTPUTS + i].setChannels(m_outputChannels[i]);
		for (int j = 0; j < m_outputChannels[i]; j++) {
			outputs[OutputId::OUT_OUTPUTS + i].setVoltage(m_outputVoltages[i][j], j);
		}
	}
}

void TimeSeqModule::setDisplayScriptError(bool error) {
	m_scriptError = error;
	if (m_timeSeqDisplay) {
		m_timeSeqDisplay->setError(m_scriptError);
	}
}

void TimeSeqModule::clearScript() {
	setDisplayScriptError(false);
	m_timeSeqCore->clearScript();
	m_script.reset();
}

std::list<std::string>& TimeSeqModule::getLastScriptLoadErrors() {
	return m_lastScriptLoadErrors;
}

std::vector<std::string>& TimeSeqModule::getFailedAsserts() {
	return m_failedAsserts;
}

void TimeSeqModule::setTimeSeqDisplay(TimeSeqDisplay* timeSeqDisplay) {
	m_timeSeqDisplay = timeSeqDisplay;
	if (m_timeSeqDisplay != nullptr) {
		m_timeSeqDisplay->setTimeSeqCore(m_timeSeqCore);
		m_timeSeqDisplay->setError(m_scriptError);
		m_timeSeqDisplay->setAssert(m_failedAsserts.size() > 0);
	}
}

void TimeSeqModule::setLEDDisplay(LEDDisplay* ledDisplay) {
	m_ledDisplay = ledDisplay;
}

int TimeSeqModule::getRate() {
	if (inputs[TimeSeqModule::InputId::IN_RATE].isConnected()) {
		float rate = inputs[TimeSeqModule::InputId::IN_RATE].getVoltage();
		if (rate < -10) {
			return -10;
		} else if (rate > 10) {
			return 10;
		} else {
			return std::floor(rate);
		}
	} else {
		return params[TimeSeqModule::ParamId::PARAM_RATE].getValue();
	}
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
	addParam(createLightParamCentered<LEDLightBezel<DimmedLight<RedLight>>>(Vec(112.f+7.5f, 253.f-140.f), module, TimeSeqModule::PARAM_RESET, TimeSeqModule::LIGHT_RESET));
	addOutput(createOutputCentered<NTPort>(Vec(112.f+7.5f, 286.5f-140.f), module, TimeSeqModule::OUT_RESET));
	addInput(createInputCentered<NTPort>(Vec(68.f+7.5f, 66.5f+135.f), module, TimeSeqModule::IN_RATE));
	addParam(createParamCentered<RoundSmallBlackKnob>(Vec(68.f+7.5f, 112.f+135.f), module, TimeSeqModule::PARAM_RATE));

	addChild(createLightCentered<SmallLight<DimmedLight<GreenLight>>>(Vec(53.5f+7.5f, 347.5f), module, TimeSeqModule::LightId::LIGHT_LANE_LOOPED));
	addChild(createLightCentered<SmallLight<GreenLight>>(Vec(68.f+7.5f, 347.5f), module, TimeSeqModule::LightId::LIGHT_SEGMENT_STARTED));
	addChild(createLightCentered<SmallLight<DimmedLight<GreenLight>>>(Vec(82.5f+7.5f, 347.5f), module, TimeSeqModule::LightId::LIGHT_TRIGGER_TRIGGERED));

	addChild(createParamCentered<VCVButton>(Vec(92.5f + 10.f -2.f + 23.f - 44.f + 7.5f, 315.f), module, TimeSeqModule::PARAM_RESET_CLOCK));

	TimeSeqDisplay* timeSeqDisplay = createWidget<TimeSeqDisplay>(Vec(92.5f+7.5f, 174.f));
	timeSeqDisplay->setSize(Vec(39.f, 178.f));
	addChild(timeSeqDisplay);
	if (module != nullptr) {
		module->setTimeSeqDisplay(timeSeqDisplay);
	}

	LEDDisplay* ledDisplay = new LEDDisplay(nvgRGB(0xFF, 0x50, 0x50), nvgRGB(0x40, 0x40, 0x40), "88:88", 10, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, true);
	ledDisplay->setPosition(Vec(47.5f+7.5f+.25, 284.f+.25));
	ledDisplay->setSize(Vec(41.f, 18.f));
	ledDisplay->setForegroundText("16:20");
	addChild(ledDisplay);
	if (module != nullptr) {
		module->setLEDDisplay(ledDisplay);
	}
}

void TimeSeqWidget::appendContextMenu(Menu* menu) {
	NTModuleWidget::appendContextMenu(menu);

	bool disabled = !hasScript();
	bool hasClipboard = glfwGetClipboardString(APP->window->win) == nullptr;
	bool hasAsserts = hasFailedAsserts();
	menu->addChild(new MenuSeparator);
	menu->addChild(createSubmenuItem("Script", "",
		[this, disabled, hasClipboard](Menu* menu) {
			menu->addChild(createMenuItem("Load script...", "", [this]() { this->loadScript(); }));
			menu->addChild(createMenuItem("Save script...", "", [this]() { this->saveScript(); }, disabled));
			menu->addChild(new MenuSeparator);
			menu->addChild(createMenuItem("Copy script", "", [this]() { this->copyScript(); }, disabled));
			menu->addChild(createMenuItem("Paste script", "", [this]() { this->pasteScript(); }, hasClipboard));
			menu->addChild(new MenuSeparator);
			menu->addChild(createMenuItem("Clear script", "", [this]() { this->clearScript(); }, disabled));
		}
	));
	menu->addChild(new MenuSeparator);
	menu->addChild(createMenuItem("Copy failed assertions", "", [this]() { this->copyAssertions(); }, !hasAsserts));
}

void TimeSeqWidget::onRemove(const RemoveEvent& e) {
	engine::Module* module = getModule();
	if (module != nullptr) {
		TimeSeqModule* timeSeqModule = dynamic_cast<TimeSeqModule *>(module);
		timeSeqModule->setTimeSeqDisplay(nullptr);
		timeSeqModule->setLEDDisplay(nullptr);
	}
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
					history::ModuleChange *h = new history::ModuleChange;
					h->name = "load TimeSeq script";
					h->moduleId = module->id;
					h->oldModuleJ = json_incref(toJson());
					h->newModuleJ = nullptr;

					std::string error = timeSeqModule->loadScript(std::make_shared<std::string>(json));
					if (error.length() > 0) {
						delete h;
						if (osdialog_message(OSDIALOG_ERROR, OSDIALOG_YES_NO, error.c_str()) == 1) {
							copyLastLoadErrors();
						}
					} else {
						h->newModuleJ = json_incref(toJson());
						APP->history->push(h);
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

void TimeSeqWidget::pasteScript() {
	const char* clipboardString = glfwGetClipboardString(APP->window->win);
	if (clipboardString) {
		// If a script is already loaded, first confirm if it should be replaced.
		// if ((!hasScript()) || (osdialog_message(OSDIALOG_ERROR, OSDIALOG_YES_NO, "A script is already loaded. Are you sure you want to replace it?") == 1)) {
			std::string json = clipboardString;
			TimeSeqModule* timeSeqModule = dynamic_cast<TimeSeqModule *>(getModule());
			if (timeSeqModule != nullptr) {
				history::ModuleChange *h = new history::ModuleChange;
				h->name = "paste TimeSeq script";
				h->moduleId = module->id;
				h->oldModuleJ = json_incref(toJson());
				h->newModuleJ = nullptr;

				std::string error = timeSeqModule->loadScript(std::make_shared<std::string>(json));
				if (error.length() > 0) {
					delete h;
					if (osdialog_message(OSDIALOG_ERROR, OSDIALOG_YES_NO, error.c_str()) == 1) {
						copyLastLoadErrors();
					}
				} else {
					h->newModuleJ = json_incref(toJson());
					APP->history->push(h);
				}
			}
		// }
	}
}

void TimeSeqWidget::copyScript() {
	TimeSeqModule* timeSeqModule = dynamic_cast<TimeSeqModule *>(getModule());
	if ((timeSeqModule) && (timeSeqModule->getScript()) ) {
		std::string script = *timeSeqModule->getScript();
		if (script.size() > 0) {
			glfwSetClipboardString(APP->window->win, script.c_str());
		}
	}
}

void TimeSeqWidget::clearScript() {
	if (osdialog_message(OSDIALOG_WARNING, OSDIALOG_YES_NO, "Are you sure you want to clear the currently loaded script?") == 1) {
		history::ModuleChange *h = new history::ModuleChange;
		h->name = "clear TimeSeq script";
		h->moduleId = module->id;
		h->oldModuleJ = json_incref(toJson());

		dynamic_cast<TimeSeqModule *>(getModule())->clearScript();

		h->newModuleJ = json_incref(toJson());
		APP->history->push(h);
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

void TimeSeqWidget::copyAssertions() {
	TimeSeqModule* timeSeqModule = dynamic_cast<TimeSeqModule *>(getModule());
	if (timeSeqModule != nullptr) {
		std::vector<std::string>& failedAsserts = timeSeqModule->getFailedAsserts();
		if (failedAsserts.size() > 0) {
			std::ostringstream assertsMessage;
			for (const std::string& assert : failedAsserts) {
				if (assertsMessage.tellp() != 0) {
					assertsMessage << "\n";
				}
				assertsMessage << assert;
			}
			glfwSetClipboardString(APP->window->win, assertsMessage.str().c_str());
		}
	}
}

bool TimeSeqWidget::hasScript() {
	return getModule() ? (bool) dynamic_cast<TimeSeqModule *>(getModule())->getScript() : false;
}

bool TimeSeqWidget::hasFailedAsserts() {
	return getModule() ? dynamic_cast<TimeSeqModule *>(getModule())->getFailedAsserts().size() > 0 : false;
}


Model* modelTimeSeq = createModel<TimeSeqModule, TimeSeqWidget>("timeseq");
