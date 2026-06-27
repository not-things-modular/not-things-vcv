#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <unordered_map>
#include "timeseq-validation.hpp"

#ifndef nt_private
	#define nt_private private
#endif

namespace timeseq {

struct JsonLoader;
struct ProcessorLoader;
struct Script;
struct Processor;

struct PortHandler {
	virtual float getInputPortVoltage(int index, int channel) const = 0;
	virtual float getOutputPortVoltage(int index, int channel) const = 0;

	virtual void setOutputPortVoltage(int index, int channel, float voltage) = 0;
	virtual void setOutputPortChannels(int index, int channels) = 0;

	virtual void setOutputPortLabel(int index, const std::string& label) = 0;
};

struct VariableHandler {
	virtual float getVariable(const std::string& name) const = 0;
	virtual void setVariable(const std::string& name, float value) = 0;
};

struct TriggerHandler {
	virtual const std::vector<std::string>& getTriggers() const = 0;
	virtual void setTrigger(const std::string& name) = 0;
};

struct AssertListener {
	virtual void assertFailed(const std::string& name, const std::string& message, bool stop) = 0;
};

struct SampleRateReader {
	virtual float getSampleRate() const = 0;
};

struct EventListener {
	virtual void laneLooped() = 0;
	virtual void segmentStarted() = 0;
	virtual void triggerTriggered() = 0;
	virtual void scriptReset() = 0;
};

struct TimeSeqCore : VariableHandler, TriggerHandler {
	enum Status { EMPTY, LOADING, RUNNING, PAUSED };

	TimeSeqCore(PortHandler* portHandler, const SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener);
	TimeSeqCore(std::shared_ptr<JsonLoader> jsonLoader, std::shared_ptr<ProcessorLoader> processorLoader, const SampleRateReader* sampleRateReader, EventListener* eventListener);
	virtual ~TimeSeqCore();

	const std::vector<timeseq::ValidationError> loadScript(const std::string& scriptData);
	void reloadScript();
	void clearScript();

	Status getStatus() const;

	void start(int sampleDelay);
	void pause();
	void reset();

	void process(int rate);

	float getVariable(const std::string& name) const override;
	void setVariable(const std::string& name, float value) override;

	const std::vector<std::string>& getTriggers() const override;
	void setTrigger(const std::string& name) override;

	uint32_t getCurrentSampleRate() const;
	uint32_t getElapsedSamples() const;
	void resetElapsedSamples();

	nt_private:
		Status m_status = Status::EMPTY;
		int m_startSampleDelay = 0;
		uint32_t m_elapsedSamples = 0;
		uint32_t m_sampleRate = 48000;
		uint32_t m_samplesPerHour = 48000 * 60 * 60;

		std::shared_ptr<JsonLoader> m_jsonLoader;
		std::shared_ptr<ProcessorLoader> m_processorLoader;

		bool m_reset = false;
		std::shared_ptr<Script> m_script;
		std::shared_ptr<Processor> m_processor;
		// The loading of a new processor happens on another thread than the processing thread.
		// If that happens while we're processing, keep the "old" processor referenced as dangling so it doesn't get immediately destroyed
		std::vector<std::shared_ptr<Processor>> m_danglingProcessors;

		std::unordered_map<std::string, float> m_variables;
		std::vector<std::string> m_triggers[2];
		bool m_triggerIdx = false;

		EventListener* m_eventListener;
		const SampleRateReader* m_sampleRateReader;

		void processReset();
};

}
