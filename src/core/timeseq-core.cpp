#include "core/timeseq-core.hpp"
#include "core/timeseq-script.hpp"
#include <rack.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

extern rack::Plugin* pluginInstance;

using namespace timeseq;


TimeSeqCore::TimeSeqCore(PortHandler* portHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener) {
	m_jsonLoader = new JsonLoader();
	m_processorLoader = new ProcessorLoader(portHandler, this, this, sampleRateReader, eventListener, assertListener);
	m_eventListener = eventListener;
	m_sampleRateReader = sampleRateReader;
}

TimeSeqCore::~TimeSeqCore() {
}

std::vector<ValidationError> TimeSeqCore::loadScript(std::string& scriptData) {
	std::istringstream scriptStream(scriptData);
	std::vector<ValidationError> validationErrors;

	std::shared_ptr<Script> script = m_jsonLoader->loadScript(scriptStream, &validationErrors);
	if ((validationErrors.size() == 0) && (script)) {
		std::shared_ptr<Processor> processor = m_processorLoader->loadScript(script, &validationErrors);

		if (validationErrors.size() == 0) {
			m_sampleRate = m_sampleRateReader->getSampleRate();
			m_samplesPerHour = m_sampleRate * 60 * 60;
			m_script = script;
			m_processor = processor;

			m_processor->reset();
			m_status = Status::IDLE;

			resetElapsedSamples();
		}
	}

	return validationErrors;
}

void TimeSeqCore::reloadScript() {
	if (m_script) {
		m_processor = m_processorLoader->loadScript(m_script, nullptr);
		m_sampleRate = m_sampleRateReader->getSampleRate();

		m_processor->reset();
		m_status = Status::IDLE;
	}
}

void TimeSeqCore::clearScript() {
	m_status = Status::EMPTY;
	m_processor.reset();
	m_script.reset();
	resetElapsedSamples();
}

TimeSeqCore::Status TimeSeqCore::getStatus() {
	return m_status;
}

bool TimeSeqCore::canProcess() {
	return (bool) m_processor;
}

void TimeSeqCore::start() {
	if (m_processor) {
		m_status = Status::RUNNING;
		m_triggers[0].clear();
		m_triggers[1].clear();
		m_variables.clear();
	} else {
		m_status = Status::EMPTY;
	}
}

void TimeSeqCore::pause() {
	if (m_processor) {
		m_status = Status::PAUSED;
	} else {
		m_status = Status::EMPTY;
	}
}

void TimeSeqCore::reset() {
	if (m_processor) {
		m_processor->reset();
	}
	m_triggers[0].clear();
	m_triggers[1].clear();
	m_variables.clear();

	resetElapsedSamples();
}

void TimeSeqCore::process() {
	if (m_processor) {
		m_triggerIdx = !m_triggerIdx; // Triggers that were set in the previous process become the active triggers now
		m_triggers[!m_triggerIdx].clear();
		m_processor->process();

		m_elapsedSamples++;
		if (m_elapsedSamples >= m_samplesPerHour) {
			m_elapsedSamples = 0;
		}
	}
}

float TimeSeqCore::getVariable(std::string name) {
	std::unordered_map<std::string, float>::iterator it = m_variables.find(name);
	if (it != m_variables.end()) {
		return it->second;
	} else {
		return 0.f;
	}
}

void TimeSeqCore::setVariable(std::string name, float value) {
	if (value == 0.f) {
		std::unordered_map<std::string, float>::iterator it = m_variables.find(name);
		if (it != m_variables.end()) {
			m_variables.erase(it);
		}
	} else {
		m_variables[name] = value;
	}
}

std::vector<std::string>& TimeSeqCore::getTriggers() {
	return m_triggers[m_triggerIdx];
}

void TimeSeqCore::setTrigger(std::string name) {
	m_triggers[!m_triggerIdx].push_back(name);
	m_eventListener->triggerTriggered();
}

uint32_t TimeSeqCore::getCurrentSampleRate() {
	return m_sampleRate;
}

uint32_t TimeSeqCore::getElapsedSamples() {
	return m_elapsedSamples;
}

void TimeSeqCore::resetElapsedSamples() {
	m_elapsedSamples = 0;
}
