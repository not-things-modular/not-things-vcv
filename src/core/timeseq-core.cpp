#include "core/timeseq-core.hpp"
#include "core/timeseq-script.hpp"
#include <rack.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace timeseq;


TimeSeqCore::TimeSeqCore(PortHandler* portHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener) :
		TimeSeqCore(std::shared_ptr<JsonLoader>(new JsonLoader()), std::shared_ptr<ProcessorLoader>(new ProcessorLoader(portHandler, this, this, sampleRateReader, eventListener, assertListener)), sampleRateReader, eventListener) {
}

TimeSeqCore::TimeSeqCore(std::shared_ptr<JsonLoader> jsonLoader, std::shared_ptr<ProcessorLoader> processorLoader, SampleRateReader* sampleRateReader, EventListener* eventListener) {
	m_jsonLoader = jsonLoader;
	m_processorLoader = processorLoader;
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

		if ((validationErrors.size() == 0) && (processor)) {
			m_sampleRate = m_sampleRateReader->getSampleRate();
			m_samplesPerHour = m_sampleRate * 60 * 60;
			m_script = script;
			m_danglingProcessor = m_processor;
			m_processor = processor;

			m_status = Status::IDLE;
			
			reset();
		}
	}

	return validationErrors;
}

void TimeSeqCore::reloadScript() {
	if (m_script) {
		m_danglingProcessor = m_processor;
		m_processor = m_processorLoader->loadScript(m_script, nullptr);
		m_sampleRate = m_sampleRateReader->getSampleRate();

		m_status = Status::IDLE;
		reset();
	}
}

void TimeSeqCore::clearScript() {
	m_status = Status::EMPTY;
	m_processor.reset();
	m_script.reset();
	reset();
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
	m_eventListener->scriptReset();

	if (m_processor) {
		m_processor->reset();
	}
	m_triggers[0].clear();
	m_triggers[1].clear();
	m_variables.clear();

	resetElapsedSamples();
}

void TimeSeqCore::process() {
	std::shared_ptr<Processor> processor = m_processor;

	// If there is an old processor still dangling, it can be released now
	if (m_danglingProcessor) {
		m_danglingProcessor = nullptr;
	}

	if (processor) {
		m_triggerIdx = !m_triggerIdx; // Triggers that were set in the previous process become the active triggers now
		m_triggers[!m_triggerIdx].clear();
		processor->process();

		m_elapsedSamples++;
		if (m_elapsedSamples >= m_samplesPerHour) {
			m_elapsedSamples = 0;
		}
	}
}

float TimeSeqCore::getVariable(std::string& name) {
	std::unordered_map<std::string, float>::iterator it = m_variables.find(name);
	if (it != m_variables.end()) {
		return it->second;
	} else {
		return 0.f;
	}
}

void TimeSeqCore::setVariable(std::string& name, float value) {
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
