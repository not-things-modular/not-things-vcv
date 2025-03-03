#include "modules/timeseq.hpp"
#include "components/ntport.hpp"
#include <osdialog.h>


TimeSeqModule::TimeSeqModule() {
	m_timeSeqCore = new timeseq::TimeSeqCore(this, this, this);

	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configInput(IN_INPUTS + i, string::f("Input %d", i + 1));
		configOutput(OUT_OUTPUTS + i, string::f("Output %d", i + 1));
	}
}

TimeSeqModule::~TimeSeqModule() {
	delete m_timeSeqCore;
}

void TimeSeqModule::process(const ProcessArgs& args) {
}

float TimeSeqModule::getInputPortVoltage(int index, int channel) {
	return inputs[InputId::IN_INPUTS + index].getVoltage(channel);
}

float TimeSeqModule::getOutputPortVoltage(int index, int channel) {
	return outputs[OutputId::OUT_OUTPUTS + index].getVoltage(channel);
}

float TimeSeqModule::getSampleRate() {
	return APP->engine->getSampleRate();
}

void TimeSeqModule::setOutputPortVoltage(int index, int channel, float voltage) {
	outputs[OutputId::OUT_OUTPUTS + index].setVoltage(voltage, channel);
}

void TimeSeqModule::setOutputPortChannels(int index, int channels) {
	outputs[OutputId::OUT_OUTPUTS + index].setChannels(channels);
	// The setChannels will not set a connected port to 0. So if the request is to set it to 0,
	// assign the channels property again explicitly
	if (channels == 0) {
		outputs[OutputId::OUT_OUTPUTS + index].channels = 0;
	}
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

std::list<std::string>& TimeSeqModule::getLastScriptLoadErrors() {
	return m_lastScriptLoadErrors;
}


TimeSeqWidget::TimeSeqWidget(TimeSeqModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "timeseq") {
	float xIn = 24;
	float xOut = 126;
	float y = 41.5;
	float yDelta = 40;
	for (int i = 0; i < 8; i++) {
		addInput(createInputCentered<NTPort>(Vec(xIn, y), module, TimeSeqModule::IN_INPUTS + i));
		addOutput(createOutputCentered<NTPort>(Vec(xOut, y), module, TimeSeqModule::OUT_OUTPUTS + i));
		y += yDelta;
	}
}

void TimeSeqWidget::appendContextMenu(Menu* menu) {
	NTModuleWidget::appendContextMenu(menu);

	menu->addChild(new MenuSeparator);
	menu->addChild(createMenuItem("Load script...", "", [this]() { this->loadScript(); }));
	menu->addChild(createMenuItem("Save script...", "", [this]() { this->saveScript(); }));
}

void TimeSeqWidget::loadScript() {
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

void TimeSeqWidget::saveScript() {
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


Model* modelTimeSeq = createModel<TimeSeqModule, TimeSeqWidget>("timeseq");
