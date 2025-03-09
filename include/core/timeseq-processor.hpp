#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <random>
#include <rack.hpp>
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
struct PortHandler;
struct VariableHandler;
struct TriggerHandler;
struct SampleRateReader;

struct CalcProcessor {
	CalcProcessor(ScriptCalc *scriptCalc, std::shared_ptr<ValueProcessor> value);

	double calc(double value);

	private:
		ScriptCalc *m_scriptCalc;
		std::shared_ptr<ValueProcessor> m_value;
};

struct ValueProcessor {
	ValueProcessor(std::vector<std::shared_ptr<CalcProcessor>> calcProcessors);

	double process();
	virtual double processValue() = 0;

	private:
		std::vector<std::shared_ptr<CalcProcessor>> m_calcProcessors;
};

struct StaticValueProcessor : ValueProcessor {
	StaticValueProcessor(float value, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors);

	double processValue() override;

	private:
		float m_value;
};

struct VariableValueProcessor : ValueProcessor {
	VariableValueProcessor(std::string name, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors, VariableHandler* variableHandler);

	double processValue() override;

	private:
		std::string m_name;
		VariableHandler* m_variableHandler;
};

struct InputValueProcessor : ValueProcessor {
	InputValueProcessor(int inputPort, int inputChannel, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors, PortHandler* portHandler);

	double processValue() override;

	private:
		int m_inputPort;
		int m_inputChannel;
		PortHandler* m_portHandler;
};

struct OutputValueProcessor : ValueProcessor {
	OutputValueProcessor(int outputPort, int outputChannel, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors, PortHandler* portHandler);

	double processValue() override;

	private:
		int m_outputPort;
		int m_outputChannel;
		PortHandler* m_portHandler;
};

struct RandValueProcessor : ValueProcessor {
	RandValueProcessor(std::shared_ptr<ValueProcessor> lowerValue, std::shared_ptr<ValueProcessor> upperValue, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors);

	double processValue() override;

	private:
		std::shared_ptr<ValueProcessor> m_lowerValue;
		std::shared_ptr<ValueProcessor> m_upperValue;

		std::minstd_rand m_generator;
};

struct ActionProcessor {
	virtual void process() = 0;
};

struct ActionSetValueProcessor : ActionProcessor {
	ActionSetValueProcessor(std::shared_ptr<ValueProcessor> value, int outputPort, int outputChannel, PortHandler* portHandler);

	void process() override;

	private:
		std::shared_ptr<ValueProcessor> m_value;
		int m_outputPort;
		int m_outputChannel;
		PortHandler* m_portHandler;
};

struct ActionSetVariableProcessor : ActionProcessor {
	ActionSetVariableProcessor(std::shared_ptr<ValueProcessor> value, std::string name, VariableHandler* variableHandler);

	void process() override;

	private:
		std::shared_ptr<ValueProcessor> m_value;
		std::string m_name;
		VariableHandler* m_variableHandler;
};

struct ActionSetPolyphonyProcessor : ActionProcessor {
	ActionSetPolyphonyProcessor(int outputPort, int channelCount, PortHandler* portHandler);

	void process() override;

	private:
		int m_outputPort;
		int m_channelCount;
		PortHandler* m_portHandler;
};

struct ActionTriggerProcessor : ActionProcessor {
	ActionTriggerProcessor(std::string trigger, TriggerHandler* triggerHandler);

	void process() override;

	private:
		std::string m_trigger;
		TriggerHandler* m_triggerHandler;
};

struct ActionGlideProcessor {
	ActionGlideProcessor(std::shared_ptr<ValueProcessor> startValue, std::shared_ptr<ValueProcessor> endValue, int outputPort, int outputChannel, std::string variable, PortHandler* portHandler, VariableHandler* variableHandler);

	void start(uint64_t glideLength);
	void process(uint64_t glidePosition);

	private:
		std::shared_ptr<ValueProcessor> m_startValueProcessor;
		std::shared_ptr<ValueProcessor> m_endValueProcessor;
		
		PortHandler* m_portHandler;
		VariableHandler* m_variableHandler;

		int m_outputPort;
		int m_outputChannel;
		std::string m_variable;

		// The start and end values that were captured when the glide action was started
		double m_startValue;
		double m_endValue;
		// Pre-calculated for runtime performance: the difference between start and end, and "1.0 / duration"
		double m_valueDelta;
		double m_durationInverse;
};

struct DurationProcessor {
	enum DurationState { STATE_IDLE, STATE_START, STATE_PROGRESS, STATE_END };

	DurationProcessor(uint64_t duration, double drift);

	DurationState getState();
	uint64_t getPosition();
	uint64_t getDuration();

	double process(double drift);
	void reset();

	private:
		DurationState m_state = STATE_IDLE;
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

	DurationProcessor::DurationState getState();

	double process(double drift);
	void reset();

	private:
		ScriptSegment* m_scriptSegment;
		std::shared_ptr<DurationProcessor> m_duration;

		std::vector<std::shared_ptr<ActionProcessor>> m_startActions;
		std::vector<std::shared_ptr<ActionProcessor>> m_endActions;
		std::vector<std::shared_ptr<ActionGlideProcessor>> m_glideActions;

		void processStartActions();
		void processEndActions();
		void processGlideActions(bool start);
};

struct LaneProcessor {
	enum LaneState { STATE_IDLE, STATE_PROCESSING, STATE_PENDING_LOOP };

	LaneProcessor(ScriptLane* scriptLane, std::vector<std::shared_ptr<SegmentProcessor>> segments);

	LaneState getState();

	bool process();
	void loop();
	void reset();

	void processTriggers(std::vector<std::string>& triggers);

	private:
		ScriptLane* m_scriptLane;
		std::vector<std::shared_ptr<SegmentProcessor>> m_segments;

		LaneState m_state = LaneState::STATE_IDLE;
		int m_repeatCount = 0;

		int m_activeSegment = 0;
		double m_drift = 0.;
};

struct TimelineProcessor {
	TimelineProcessor(
		ScriptTimeline* scriptTimeline,
		std::vector<std::shared_ptr<LaneProcessor>> lanes,
		std::unordered_map<std::string, std::vector<std::shared_ptr<LaneProcessor>>> startTriggers,
		std::unordered_map<std::string, std::vector<std::shared_ptr<LaneProcessor>>> stopTriggers,
		TriggerHandler* triggerHandler
	);

	void process();
	void reset();

	private:
		ScriptTimeline* m_scriptTimeline;
		std::vector<std::shared_ptr<LaneProcessor>> m_lanes;

		std::unordered_map<std::string, std::vector<std::shared_ptr<LaneProcessor>>> m_startTriggers;
		std::unordered_map<std::string, std::vector<std::shared_ptr<LaneProcessor>>> m_stopTriggers;

		TriggerHandler* m_triggerHandler;
};

struct TriggerProcessor {
	TriggerProcessor(std::string id, int inputPort, int inputChannel, PortHandler* portHandler, TriggerHandler* triggerHandler);

	void process();

	private:
		std::string m_id;
		int m_inputPort;
		int m_inputChannel;
		PortHandler* m_portHandler;
		TriggerHandler* m_triggerHandler;

		rack::dsp::TSchmittTrigger<float> m_trigger;
};

struct Processor {
	Processor(std::vector<std::shared_ptr<TimelineProcessor>> m_timelines, std::vector<std::shared_ptr<TriggerProcessor>> m_triggers, std::vector<std::shared_ptr<ActionProcessor>> startActions, std::vector<std::shared_ptr<ActionProcessor>> endActions);

	void reset();
	void process();

	private:
		std::vector<std::shared_ptr<TimelineProcessor>> m_timelines;
		std::vector<std::shared_ptr<TriggerProcessor>> m_triggers;
		std::vector<std::shared_ptr<ActionProcessor>> m_startActions;
		std::vector<std::shared_ptr<ActionProcessor>> m_endActions;
};

struct ProcessorScriptParseContext {
	Script* script;
	std::vector<ValidationError> *validationErrors;
};

struct ProcessorScriptParser {
	ProcessorScriptParser(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader);

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
	std::shared_ptr<ActionProcessor> parseSetVariableAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseSetPolyphonyAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseTriggerAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::string> location, std::vector<std::string> valueStack);
	std::shared_ptr<ValueProcessor> parseStaticValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseVariableValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseInputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseOutputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseRandValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location, std::vector<std::string> valueStack);
	std::shared_ptr<CalcProcessor> parseCalc(ProcessorScriptParseContext* context, ScriptCalc* scriptCalc, std::vector<std::string> location, std::vector<std::string> valueStack);

	std::pair<int, int> parseInput(ProcessorScriptParseContext* context, ScriptInput* scriptInput, std::vector<std::string> location);
	std::pair<int, int> parseOutput(ProcessorScriptParseContext* context, ScriptOutput* scriptOutput, std::vector<std::string> location);

	private:
		PortHandler* m_portHandler;
		VariableHandler* m_variableHandler;
		TriggerHandler* m_triggerHandler;
		SampleRateReader* m_sampleRateReader;
};

struct ProcessorLoader {
	ProcessorLoader(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader);

	std::shared_ptr<Processor> loadScript(std::shared_ptr<Script> script, std::vector<ValidationError> *validationErrors);

	private:
		ProcessorScriptParser m_processorScriptParser;
};

}
