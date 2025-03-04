#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include "core/timeseq-validation.hpp"


namespace timeseq {

struct ScriptInput;
struct ScriptOutput;
struct ScriptValue;
struct ScriptAction;
struct ScriptGlideAction;
struct ScriptDuration;
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
struct SampleRateReader;

struct CalcProcessor {
	CalcProcessor(ScriptCalc *scriptCalc, std::shared_ptr<ValueProcessor> value);

	float calc(float value);

	private:
		ScriptCalc *m_scriptCalc;
		std::shared_ptr<ValueProcessor> m_value;
};

struct ValueProcessor {
	ValueProcessor(std::vector<std::shared_ptr<CalcProcessor>> calcProcessors);

	float process();
	virtual float processValue() = 0;

	private:
		std::vector<std::shared_ptr<CalcProcessor>> m_calcProcessors;
};

struct StaticValueProcessor : ValueProcessor {
	StaticValueProcessor(float value, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors);

	float processValue() override;

	private:
		float m_value;
};

struct InputValueProcessor : ValueProcessor {
	InputValueProcessor(int inputPort, int inputChannel, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors);

	float processValue() override;

	private:
		int m_inputPort;
		int m_inputChannel;
};

struct OutputValueProcessor : ValueProcessor {
	OutputValueProcessor(int outputPort, int outputChannel, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors);

	float processValue() override;

	private:
		int m_outputPort;
		int m_outputChannel;
};

struct RandValueProcessor : ValueProcessor {
	RandValueProcessor(std::shared_ptr<ValueProcessor> lowerValue, std::shared_ptr<ValueProcessor> upperValue, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors);

	float processValue() override;

	private:
		std::shared_ptr<ValueProcessor> m_lowerValue;
		std::shared_ptr<ValueProcessor> m_upperValue;
};

struct ActionProcessor {
	virtual void process() = 0;
};

struct ActionSetValueProcessor : ActionProcessor {
	ActionSetValueProcessor(std::shared_ptr<ValueProcessor> value, int outputPort, int outputChannel);

	void process() override;

	private:
		std::shared_ptr<ValueProcessor> m_value;
		int m_outputPort;
		int m_outputChannel;

};

struct ActionSetPolyphonyProcessor : ActionProcessor {
	ActionSetPolyphonyProcessor(int outputPort, int channelCount);

	void process() override;

	private:
		int m_outputPort;
		int m_channelCount;

};

struct ActionTriggerProcessor : ActionProcessor {
	ActionTriggerProcessor(std::string trigger);

	void process() override;

	private:
		std::string m_trigger;

};

struct ActionGlideProcessor {
	ActionGlideProcessor(std::shared_ptr<ValueProcessor> startValue, std::shared_ptr<ValueProcessor> endValue);

	void start();
	void process(long glidePosition, long glideLength);

	private:
		std::shared_ptr<ValueProcessor> m_startValueProcessor;
		std::shared_ptr<ValueProcessor> m_endValueProcessor;

		float m_startValue;
		float m_endValue;
};

struct DurationProcessor {
	enum State { STATE_IDLE, STATE_START, STATE_PROGRESS, STATE_STOP };

	DurationProcessor(uint64_t duration, double drift);

	State process();

	private:
		uint64_t m_duration;
		double m_drift;
		uint64_t m_position;
};

struct SegmentProcessor {
	SegmentProcessor(
		ScriptSegment* scriptSegment,
		std::shared_ptr<DurationProcessor> durationProcessor,
		std::vector<std::shared_ptr<ActionProcessor>> startActions,
		std::vector<std::shared_ptr<ActionProcessor>> endActions,
		std::vector<std::shared_ptr<ActionGlideProcessor>> glideActions
	);

	void process();

	private:
		ScriptSegment* m_scriptSegment;
		std::shared_ptr<DurationProcessor> m_duration;

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

struct ProcessorScriptParseContext {
	Script* script;
	std::vector<ValidationError> *validationErrors;
};

struct ProcessorScriptParser {
	ProcessorScriptParser(PortReader* portReader, SampleRateReader* sampleRateReader, PortWriter* portWriter);

	std::shared_ptr<Processor> parseScript(Script* script, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	std::shared_ptr<TimelineProcessor> parseTimeline(ProcessorScriptParseContext* context, ScriptTimeline* scriptTimeline, std::vector<std::string> location);
	std::shared_ptr<TriggerProcessor> parseInputTrigger(ProcessorScriptParseContext* context, ScriptInputTrigger* ScriptInputTrigger, std::vector<std::string> location);
	std::shared_ptr<LaneProcessor> parseLane(ProcessorScriptParseContext* context, ScriptLane* scriptLane, ScriptTimeScale* timeScale, std::vector<std::string> location);
	std::vector<std::shared_ptr<SegmentProcessor>> parseSegmentEntities(ProcessorScriptParseContext* context, std::vector<ScriptSegmentEntity>* scriptSegmentEntities, ScriptTimeScale* timeScale, std::vector<std::string> location);
	std::vector<std::shared_ptr<SegmentProcessor>> parseSegmentBlock(ProcessorScriptParseContext* context, ScriptSegmentBlock* scriptSegmentBlock, ScriptTimeScale* timeScale, std::vector<std::string> location);
	std::shared_ptr<SegmentProcessor> parseSegment(ProcessorScriptParseContext* context, ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, std::vector<std::string> location);
	std::shared_ptr<DurationProcessor> parseDuration(ProcessorScriptParseContext* context, ScriptDuration* scriptDuration, ScriptTimeScale* timeScale, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string> location);
	std::shared_ptr<ActionGlideProcessor> parseGlideAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseSetValueAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseSetPolyphonyAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseTriggerAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::string> location, std::vector<std::string> valueStack);
	std::shared_ptr<ValueProcessor> parseStaticValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseInputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseOutputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseRandValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location, std::vector<std::string> valueStack);
	std::shared_ptr<CalcProcessor> parseCalc(ProcessorScriptParseContext* context, ScriptCalc* scriptCalc, std::vector<std::string> location, std::vector<std::string> valueStack);

	std::pair<int, int> parseInput(ProcessorScriptParseContext* context, ScriptInput* scriptInput, std::vector<std::string> location);
	std::pair<int, int> parseOutput(ProcessorScriptParseContext* context, ScriptOutput* scriptOutput, std::vector<std::string> location);

	private:
		PortReader* m_portReader;
		PortWriter* m_portWriter;
		SampleRateReader* m_sampleRateReader;
};

struct ProcessorLoader {
	ProcessorLoader(PortReader* portReader, SampleRateReader* sampleRateReader, PortWriter* portWriter);

	std::shared_ptr<Processor> loadScript(std::shared_ptr<Script> script, std::vector<ValidationError> *validationErrors);

	private:
		ProcessorScriptParser m_processorScriptParser;
};

}
