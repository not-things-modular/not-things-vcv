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
	virtual float getVariable(std::string name) = 0;
	virtual void setVariable(std::string name, float value) = 0;
};

struct TriggerHandler {
	virtual void setTrigger(std::string name) = 0;
	virtual std::vector<std::string>& getTriggers() = 0;
};

struct SampleRateReader {
	virtual float getSampleRate() = 0;
};

struct EventListener {
	virtual void segmentStarted() = 0;
	virtual void triggerTriggered() = 0;
};

struct TimeSeqCore : VariableHandler, TriggerHandler {
	enum Status { EMPTY, IDLE, RUNNING, PAUSED };

	TimeSeqCore(PortHandler* portHandler, SampleRateReader* sampleRateReader, EventListener* eventListener);
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

	float getVariable(std::string name) override;
	void setVariable(std::string name, float value) override;

	void setTrigger(std::string name) override;
	std::vector<std::string>& getTriggers() override;

	private:
		Status m_status = Status::EMPTY;

		JsonLoader* m_jsonLoader;
		ProcessorLoader* m_processorLoader;

		std::shared_ptr<Script> m_script;
		std::shared_ptr<Processor> m_processor;

		std::unordered_map<std::string, float> m_variables;
		std::vector<std::string> m_triggers[2];
		bool m_triggerIdx = false;

		EventListener* m_eventListener;
};

}