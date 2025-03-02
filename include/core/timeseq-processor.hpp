#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "core/timeseq-validation.hpp"


namespace timeseq {

struct ScriptTimeScale;
struct ScriptInputTrigger;
struct ScriptTimeline;
struct ScriptLane;
struct ScriptSegmentEntity;
struct ScriptSegment;
struct ScriptSegmentBlock;
struct ScriptCalc;
struct Script;
struct ValueProcessor;
struct PortReader;
struct PortWriter;

struct CalcProcessor {
	float calc(float value);

	private:
		ScriptCalc *m_scriptCalc;
		std::unique_ptr<ValueProcessor> m_value;
};

struct ValueProcessor {
	float process();
	virtual float processValue();

	private:
		std::vector<std::shared_ptr<CalcProcessor>> m_calcs;
};

struct StaticValueProcessor : ValueProcessor {
	float processValue() override;

	private:
		float m_value;
};

struct InputValueProcessor : ValueProcessor {
	float processValue() override;

	private:
		int m_inputPort;
		int m_inputChannel;
};

struct OutputValueProcessor : ValueProcessor {
	float processValue() override;

	private:
		int m_outputPort;
		int m_outputChannel;
};

struct RandValueProcessor : ValueProcessor {
	float processValue() override;

	private:
		ValueProcessor m_lowerValue;
		ValueProcessor m_upperValue;
};

struct ActionProcessor {
	virtual void process();
};

struct ActionSetValueProcessor : ActionProcessor {
	void process() override;

	private:
		ValueProcessor m_valueProcessor;
		int m_outputPort;
		int m_outputChannel;

};

struct ActionSetPolyphonyProcessor : ActionProcessor {
	void process() override;

	private:
		int m_outputPort;
		int m_channelCount;

};

struct ActionTriggerProcessor : ActionProcessor {
	void process() override;

	private:
		std::string trigger;

};

struct ActionGlideProcessor {
	void start();
	void process(long glidePosition, long glideLength);

	private:
		ValueProcessor m_startValueProcessor;
		ValueProcessor m_endValueProcessor;
		float m_startValue;
		float m_endValue;
};

struct DurationProcessor {
	enum State { STATE_IDLE, STATE_START, STATE_PROGRESS, STATE_STOP };

	State process();

	private:
		long m_duration;
		long m_position;
};

struct SegmentProcessor {
	void process();

	private:
		ScriptSegment* m_scriptSegment;
		DurationProcessor m_durationProcessor;

		std::vector<std::shared_ptr<ActionProcessor>> m_startActions;
		std::vector<std::shared_ptr<ActionProcessor>> m_endActions;
		std::vector<std::shared_ptr<ActionGlideProcessor>> m_glideActions;
};

struct LaneProcessor {
	LaneProcessor(ScriptLane* scriptLane, std::vector<std::shared_ptr<SegmentProcessor>> segments);

	void process();

	private:
		ScriptLane* m_scriptLane;
		std::vector<std::shared_ptr<SegmentProcessor>> m_segments;

		bool m_active = false;
		int m_repeatCount = 0;
};
	
struct TimelineProcessor {
	TimelineProcessor(
		ScriptTimeline* scriptTimeline,
		std::vector<std::shared_ptr<LaneProcessor>> laneProcessors,
		std::unordered_map<std::string, std::vector<std::shared_ptr<LaneProcessor>>> startTriggers,
		std::unordered_map<std::string, std::vector<std::shared_ptr<LaneProcessor>>> stopTriggers
	);

	void process();
	
	std::vector<LaneProcessor>& getLaneProcessors();

	private:
		ScriptTimeline* m_scriptTimeline;
		std::vector<std::shared_ptr<LaneProcessor>> m_laneProcessors;

		std::unordered_map<std::string, std::vector<std::shared_ptr<LaneProcessor>>> m_startTriggers;
		std::unordered_map<std::string, std::vector<std::shared_ptr<LaneProcessor>>> m_stopTriggers;
};

struct TriggerProcessor {
	TriggerProcessor(std::string id, int inputPort, int inputChannel, PortReader* portReader);

	void process();

	private:
		PortReader* m_portReader;
		std::string m_id;
		int m_inputPort;
		int m_inputChannel;
};

struct Processor {
	Processor(std::vector<std::shared_ptr<TimelineProcessor>> m_timelines, std::vector<std::shared_ptr<TriggerProcessor>> m_triggers);

	void process();

	private:
		std::vector<std::shared_ptr<TimelineProcessor>> m_timelines;
		std::vector<std::shared_ptr<TriggerProcessor>> m_triggers;
};

struct ProcessorScriptParser {
	ProcessorScriptParser(PortReader* portReader, PortWriter* portWriter);

	std::shared_ptr<Processor> parseScript(Script* script, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	std::shared_ptr<TimelineProcessor> parseTimeline(ScriptTimeline* scriptTimeline, Script* script, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	std::shared_ptr<TriggerProcessor> parseInputTrigger(ScriptInputTrigger* ScriptInputTrigger, Script* script, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	std::shared_ptr<LaneProcessor> parseLane(ScriptLane* scriptLane, ScriptTimeScale* timeScale, Script* script, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	std::vector<std::shared_ptr<SegmentProcessor>> parseSegmentEntities(std::vector<ScriptSegmentEntity>* scriptSegmentEntities, ScriptTimeScale* timeScale, Script* script, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	std::shared_ptr<SegmentProcessor> parseSegment(ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, Script* script, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	std::vector<std::shared_ptr<SegmentProcessor>> parseSegmentBlock(ScriptSegmentBlock* scriptSegmentBlock, ScriptTimeScale* timeScale, Script* script, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);

	private:
		PortReader* m_portReader;
		PortWriter* m_portWriter;
};

struct ProcessorLoader {
	ProcessorLoader(PortReader* portReader, PortWriter* portWriter);
	
	std::shared_ptr<Processor> loadScript(std::shared_ptr<Script> script, std::vector<ValidationError> *validationErrors);

	private:
		ProcessorScriptParser m_processorScriptParser;
};

}
