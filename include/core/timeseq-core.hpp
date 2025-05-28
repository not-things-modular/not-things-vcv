#pragma once

#include "core/timeseq-json.hpp"
#include "core/timeseq-processor.hpp"


namespace timeseq {

struct PortHandler {
	virtual float getInputPortVoltage(int index, int channel) = 0;
	virtual float getOutputPortVoltage(int index, int channel) = 0;

	virtual void setOutputPortVoltage(int index, int channel, float voltage) = 0;
	virtual void setOutputPortChannels(int index, int channels) = 0;
};

struct VariableHandler {
	virtual float getVariable(std::string& name) = 0;
	virtual void setVariable(std::string& name, float value) = 0;
};

struct TriggerHandler {
	virtual std::vector<std::string>& getTriggers() = 0;
	virtual void setTrigger(std::string& name) = 0;
};

struct AssertListener {
	virtual void assertFailed(std::string name, std::string message, bool stop) = 0;
};

struct SampleRateReader {
	virtual float getSampleRate() = 0;
};

struct EventListener {
	virtual void laneLooped() = 0;
	virtual void segmentStarted() = 0;
	virtual void triggerTriggered() = 0;
	virtual void scriptReset() = 0;
};

struct TimeSeqCore : VariableHandler, TriggerHandler {
	enum Status { EMPTY, IDLE, RUNNING, PAUSED };

	TimeSeqCore(PortHandler* portHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener);
	TimeSeqCore(std::shared_ptr<JsonLoader> jsonLoader, std::shared_ptr<ProcessorLoader> processorLoader, SampleRateReader* sampleRateReader, EventListener* eventListener);
	virtual ~TimeSeqCore();

	std::vector<timeseq::ValidationError> loadScript(std::string& scriptData);
	void reloadScript();
	void clearScript();

	Status getStatus();
	bool canProcess();

	void start();
	void pause();
	void reset();

	void process();

	float getVariable(std::string& name) override;
	void setVariable(std::string& name, float value) override;

	std::vector<std::string>& getTriggers() override;
	void setTrigger(std::string& name) override;

	uint32_t getCurrentSampleRate();
	uint32_t getElapsedSamples();
	void resetElapsedSamples();

	nt_private:
		Status m_status = Status::EMPTY;
		uint32_t m_elapsedSamples = 0;
		uint32_t m_sampleRate = 48000;
		uint32_t m_samplesPerHour = 48000 * 60 * 60;

		std::shared_ptr<JsonLoader> m_jsonLoader;
		std::shared_ptr<ProcessorLoader> m_processorLoader;

		std::shared_ptr<Script> m_script;
		std::shared_ptr<Processor> m_processor;
		// The loading of a new processor happens on another thread than the processing thread.
		// If that happens while we're processing, keep the "old" processor referenced as dangling so it doesn't get immediately destroyed
		std::shared_ptr<Processor> m_danglingProcessor;

		std::unordered_map<std::string, float> m_variables;
		std::vector<std::string> m_triggers[2];
		bool m_triggerIdx = false;

		EventListener* m_eventListener;
		SampleRateReader* m_sampleRateReader;
};

}
