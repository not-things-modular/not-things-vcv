#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <random>
#include <rack.hpp>
#include <cstdint>
#include "core/timeseq-validation.hpp"

#ifndef nt_private
	#define nt_private private
#endif


namespace timeseq {

struct ScriptIf;
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
struct ScriptSegment;
struct ScriptSegmentBlock;
struct ScriptCalc;
struct ScriptTuning;
struct Script;
struct ValueProcessor;
struct PortHandler;
struct VariableHandler;
struct TriggerHandler;
struct SampleRateReader;
struct EventListener;
struct AssertListener;


struct CalcProcessor {
	virtual double calc(double value) = 0;
};

struct CalcValueProcessor : CalcProcessor {
	enum ValueCalcOperation { ADD, SUB, DIV, MULT, MAX, MIN, REMAIN };

	CalcValueProcessor(ScriptCalc* scriptCalc, std::shared_ptr<ValueProcessor> value);

	double calc(double value) override;

	nt_private:
		ValueCalcOperation m_operation;
		std::shared_ptr<ValueProcessor> m_value;
};

struct CalcTruncProcessor : CalcProcessor {
	double calc(double value) override;
};

struct CalcFracProcessor : CalcProcessor {
	double calc(double value) override;
};

struct CalcRoundProcessor : CalcProcessor {
	CalcRoundProcessor(ScriptCalc* scriptCalc);

	double calc(double value) override;

	nt_private:
		ScriptCalc* m_scriptCalc;
};

struct CalcQuantizeProcessor : CalcProcessor {
	CalcQuantizeProcessor(ScriptTuning* scriptTuning);

	double calc(double value) override;

	nt_private:
		std::vector<std::array<float, 2>> m_quantizeValues;
};

struct CalcSignProcessor : CalcProcessor {
	CalcSignProcessor(ScriptCalc* scriptCalc);

	double calc(double value) override;

	nt_private:
		bool m_positive;
};

struct ValueProcessor {
	ValueProcessor(std::vector<std::shared_ptr<CalcProcessor>> calcProcessors, bool quantize);

	double process();
	virtual double processValue() = 0;

	nt_private:
		std::vector<std::shared_ptr<CalcProcessor>> m_calcProcessors;
		bool m_quantize;

		double quantize(double value);
};

struct StaticValueProcessor : ValueProcessor {
	StaticValueProcessor(float value, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors, bool quantize);

	double processValue() override;

	nt_private:
		float m_value;
};

struct VariableValueProcessor : ValueProcessor {
	VariableValueProcessor(std::string name, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors, bool quantize, VariableHandler* variableHandler);

	double processValue() override;

	nt_private:
		std::string m_name;
		VariableHandler* m_variableHandler;
};

struct InputValueProcessor : ValueProcessor {
	InputValueProcessor(int inputPort, int inputChannel, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors, bool quantize, PortHandler* portHandler);

	double processValue() override;

	nt_private:
		int m_inputPort;
		int m_inputChannel;
		PortHandler* m_portHandler;
};

struct OutputValueProcessor : ValueProcessor {
	OutputValueProcessor(int outputPort, int outputChannel, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors, bool quantize, PortHandler* portHandler);

	double processValue() override;

	nt_private:
		int m_outputPort;
		int m_outputChannel;
		PortHandler* m_portHandler;
};

struct RandValueGenerator {
	RandValueGenerator();
	virtual ~RandValueGenerator();

	virtual float generate(float lower, float upper);

	nt_private:
		std::minstd_rand m_generator;
};

struct RandValueProcessor : ValueProcessor {
	RandValueProcessor(std::shared_ptr<ValueProcessor> lowerValue, std::shared_ptr<ValueProcessor> upperValue, std::shared_ptr<RandValueGenerator> randValueGenerator, std::vector<std::shared_ptr<CalcProcessor>> calcProcessors, bool quantize);

	double processValue() override;

	nt_private:
		std::shared_ptr<ValueProcessor> m_lowerValue;
		std::shared_ptr<ValueProcessor> m_upperValue;

		std::shared_ptr<RandValueGenerator> m_randValueGenerator;
};

struct IfProcessor {
	IfProcessor(ScriptIf* scriptIf, std::pair<std::shared_ptr<ValueProcessor>, std::shared_ptr<ValueProcessor>> values, std::pair<std::shared_ptr<IfProcessor>, std::shared_ptr<IfProcessor>> ifs);

	bool process(std::string* message);

	nt_private:
		ScriptIf* m_scriptIf;
		std::pair<std::shared_ptr<ValueProcessor>, std::shared_ptr<ValueProcessor>> m_values;
		std::pair<std::shared_ptr<IfProcessor>, std::shared_ptr<IfProcessor>> m_ifs;
};

struct ActionProcessor {
	ActionProcessor(std::shared_ptr<IfProcessor> ifProcessor);

	void process();
	virtual void processAction() = 0;

	nt_private:
		std::shared_ptr<IfProcessor> m_ifProcessor;
};

struct ActionSetValueProcessor : ActionProcessor {
	ActionSetValueProcessor(std::shared_ptr<ValueProcessor> value, int outputPort, int outputChannel, PortHandler* portHandler, std::shared_ptr<IfProcessor> ifProcessor);

	void processAction() override;

	nt_private:
		std::shared_ptr<ValueProcessor> m_value;
		int m_outputPort;
		int m_outputChannel;
		PortHandler* m_portHandler;
};

struct ActionSetVariableProcessor : ActionProcessor {
	ActionSetVariableProcessor(std::shared_ptr<ValueProcessor> value, std::string name, VariableHandler* variableHandler, std::shared_ptr<IfProcessor> ifProcessor);

	void processAction() override;

	nt_private:
		std::shared_ptr<ValueProcessor> m_value;
		std::string m_name;
		VariableHandler* m_variableHandler;
};

struct ActionSetPolyphonyProcessor : ActionProcessor {
	ActionSetPolyphonyProcessor(int outputPort, int channelCount, PortHandler* portHandler, std::shared_ptr<IfProcessor> ifProcessor);

	void processAction() override;

	nt_private:
		int m_outputPort;
		int m_channelCount;
		PortHandler* m_portHandler;
};

struct ActionSetLabelProcessor : ActionProcessor {
	ActionSetLabelProcessor(int outputPort, std::string label, PortHandler* portHandler, std::shared_ptr<IfProcessor> ifProcessor);

	void processAction() override;

	nt_private:
		int m_outputPort;
		std::string m_label;
		PortHandler* m_portHandler;
};

struct ActionAssertProcessor : ActionProcessor {
	ActionAssertProcessor(std::string name, std::shared_ptr<IfProcessor> expect, bool stopOnFail, AssertListener* assertListener, std::shared_ptr<IfProcessor> ifProcessor);

	void processAction() override;

	nt_private:
		std::string m_name;
		std::shared_ptr<IfProcessor> m_expect;
		bool m_stopOnFail;
		AssertListener* m_assertListener;
};

struct ActionTriggerProcessor : ActionProcessor {
	ActionTriggerProcessor(std::string trigger, TriggerHandler* triggerHandler, std::shared_ptr<IfProcessor> ifProcessor);

	void processAction() override;

	nt_private:
		std::string m_trigger;
		TriggerHandler* m_triggerHandler;
};

struct ActionOngoingProcessor {
	ActionOngoingProcessor(std::shared_ptr<IfProcessor> ifProcessor);

	virtual void start(uint64_t glideLength);
	virtual void process(uint64_t glidePosition) = 0;
	virtual void end() = 0;

	protected:
		bool shouldProcess();

	nt_private:
		std::shared_ptr<IfProcessor> m_ifProcessor;

		// The result of the "if" validation as captured when the ongoing action was started
		bool m_if;
};

struct ActionGlideProcessor : ActionOngoingProcessor {
	ActionGlideProcessor(float easeFactor, bool easePow, std::shared_ptr<ValueProcessor> startValue, std::shared_ptr<ValueProcessor> endValue, std::shared_ptr<IfProcessor> ifProcessor, int outputPort, int outputChannel, std::string variable, PortHandler* portHandler, VariableHandler* variableHandler);

	void start(uint64_t glideLength) override;
	void process(uint64_t glidePosition) override;
	void end() override;

	nt_private:
		float m_easeFactor;
		bool m_easePow;
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

		// A power-based easing calculation
		double calculatePowEase(float ease, uint64_t glidePosition);
		// A sigmoid-based easing calculation
		double calculateSigEase(float ease, uint64_t glidePosition);
};

struct ActionGateProcessor : ActionOngoingProcessor {
	ActionGateProcessor(float gateHighRatio, std::shared_ptr<IfProcessor> ifProcessor, int outputPort, int outputChannel, PortHandler* portHandler);

	void start(uint64_t glideLength) override;
	void process(uint64_t glidePosition) override;
	void end() override;

	nt_private:
		PortHandler* m_portHandler;

		int m_outputPort;
		int m_outputChannel;
		float m_gateHighRatio;

		bool m_gateHigh;
		uint64_t m_gateLowPosition;
};

struct DurationProcessor {
	enum DurationState { STATE_START, STATE_PROGRESS, STATE_END };

	DurationProcessor(uint64_t duration, double drift);

	DurationState getState();
	uint64_t getPosition();
	uint64_t getDuration();

	double process(double drift);
	void reset();

	nt_private:
		DurationState m_state = STATE_START;
		uint64_t m_duration;
		double m_drift;
		uint64_t m_position;
};

struct SegmentProcessor {
	SegmentProcessor(SegmentProcessor& segmentProcessor);
	SegmentProcessor(
		ScriptSegment* scriptSegment,
		std::shared_ptr<DurationProcessor> durationProcessor,
		std::vector<std::shared_ptr<ActionProcessor>> startActions,
		std::vector<std::shared_ptr<ActionProcessor>> endActions,
		std::vector<std::shared_ptr<ActionOngoingProcessor>> ongoingActions,
		EventListener* eventListener
	);

	void pushStartActions(std::vector<std::shared_ptr<ActionProcessor>> startActions);
	void pushEndActions(std::vector<std::shared_ptr<ActionProcessor>> endActions);

	DurationProcessor::DurationState getState();

	double process(double drift);
	void reset();

	nt_private:
		ScriptSegment* m_scriptSegment;
		std::shared_ptr<DurationProcessor> m_duration;

		std::vector<std::shared_ptr<ActionProcessor>> m_startActions;
		std::vector<std::shared_ptr<ActionProcessor>> m_endActions;
		std::vector<std::shared_ptr<ActionOngoingProcessor>> m_ongoingActions;

		EventListener* m_eventListener;

		void processStartActions();
		void processEndActions();
		void processOngoingActions(bool start, bool end);
};

struct LaneProcessor {
	enum LaneState { STATE_IDLE, STATE_PROCESSING, STATE_PENDING_LOOP };

	LaneProcessor(ScriptLane* scriptLane, std::vector<std::shared_ptr<SegmentProcessor>> segments, EventListener* eventListener);

	LaneState getState();

	bool process();
	void loop();
	void reset();

	void processTriggers(std::vector<std::string>& triggers);

	nt_private:
		ScriptLane* m_scriptLane;
		std::vector<std::shared_ptr<SegmentProcessor>> m_segments;

		LaneState m_state = LaneState::STATE_IDLE;
		int m_repeatCount = 0;

		int m_activeSegment = 0;
		double m_drift = 0.;

		EventListener* m_eventListener;
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

	nt_private:
		ScriptTimeline* m_scriptTimeline;
		std::vector<std::shared_ptr<LaneProcessor>> m_lanes;

		std::unordered_map<std::string, std::vector<std::shared_ptr<LaneProcessor>>> m_startTriggers;
		std::unordered_map<std::string, std::vector<std::shared_ptr<LaneProcessor>>> m_stopTriggers;

		TriggerHandler* m_triggerHandler;
};

struct TriggerProcessor {
	TriggerProcessor(std::string id, int inputPort, int inputChannel, PortHandler* portHandler, TriggerHandler* triggerHandler);

	void process();

	nt_private:
		std::string m_id;
		int m_inputPort;
		int m_inputChannel;
		PortHandler* m_portHandler;
		TriggerHandler* m_triggerHandler;

		rack::dsp::TSchmittTrigger<float> m_trigger;
};

struct Processor {
	Processor(std::shared_ptr<Script> script, std::vector<std::shared_ptr<TimelineProcessor>> timelines, std::vector<std::shared_ptr<TriggerProcessor>> triggers, std::vector<std::shared_ptr<ActionProcessor>> startActions);

	virtual void reset();
	virtual void process();

	nt_private:
		std::vector<std::shared_ptr<TimelineProcessor>> m_timelines;
		std::vector<std::shared_ptr<TriggerProcessor>> m_triggers;
		std::vector<std::shared_ptr<ActionProcessor>> m_startActions;

		// The script objects are used throughout the processor hierarchy,
		// keep a reference so it doesn't get deleted on script switch
		std::shared_ptr<Script> m_script;
};

struct ProcessorScriptParseContext {
	Script* script;
	std::vector<ValidationError> *validationErrors;
};

struct ProcessorScriptParser {
	ProcessorScriptParser(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener, std::shared_ptr<RandValueGenerator> randomValueGenerator);

	std::shared_ptr<Processor> parseScript(std::shared_ptr<Script> script, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	std::shared_ptr<TimelineProcessor> parseTimeline(ProcessorScriptParseContext* context, ScriptTimeline* scriptTimeline, std::vector<std::string> location);
	std::shared_ptr<TriggerProcessor> parseInputTrigger(ProcessorScriptParseContext* context, ScriptInputTrigger* ScriptInputTrigger, std::vector<std::string> location);
	std::shared_ptr<LaneProcessor> parseLane(ProcessorScriptParseContext* context, ScriptLane* scriptLane, ScriptTimeScale* timeScale, std::vector<std::string> location);
	std::vector<std::shared_ptr<SegmentProcessor>> parseSegments(ProcessorScriptParseContext* context, std::vector<ScriptSegment>* scriptSegments, ScriptTimeScale* timeScale, std::vector<std::string> location, std::vector<std::string> segmentStack);
	std::vector<std::shared_ptr<SegmentProcessor>> parseSegment(ProcessorScriptParseContext* context, ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, std::vector<std::string> location, std::vector<std::string> segmentStack);
	std::shared_ptr<SegmentProcessor> parseResolvedSegment(ProcessorScriptParseContext* context, ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, std::vector<std::string> location, std::vector<std::string> segmentStack);
	std::vector<std::shared_ptr<SegmentProcessor>> parseSegmentBlock(ProcessorScriptParseContext* context, ScriptSegmentBlock* scriptSegmentBlock, ScriptTimeScale* timeScale, std::vector<ScriptAction>& actions, std::vector<std::string> location, std::vector<std::string> actionsLocation, std::vector<std::string> segmentStack);
	std::shared_ptr<DurationProcessor> parseDuration(ProcessorScriptParseContext* context, ScriptDuration* scriptDuration, ScriptTimeScale* timeScale, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseResolvedAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string> location);
	std::shared_ptr<ActionGlideProcessor> parseResolvedGlideAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string> location);
	std::shared_ptr<ActionGateProcessor> parseResolvedGateAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseSetValueAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseSetVariableAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseSetPolyphonyAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseSetLabelAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseAssertAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string> location);
	std::shared_ptr<ActionProcessor> parseTriggerAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::string> location, std::vector<std::string> valueStack);
	std::shared_ptr<ValueProcessor> parseStaticValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseVariableValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseInputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseOutputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location);
	std::shared_ptr<ValueProcessor> parseRandValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> location, std::vector<std::string> valueStack);
	std::shared_ptr<CalcProcessor> parseCalc(ProcessorScriptParseContext* context, ScriptCalc* scriptCalc, std::vector<std::string> location, std::vector<std::string> valueStack);
	std::shared_ptr<IfProcessor> parseIf(ProcessorScriptParseContext* context, ScriptIf* scriptIf, std::vector<std::string> location, std::vector<std::string> ifStack);

	std::pair<int, int> parseInput(ProcessorScriptParseContext* context, ScriptInput* scriptInput, std::vector<std::string> location);
	std::pair<int, int> parseOutput(ProcessorScriptParseContext* context, ScriptOutput* scriptOutput, std::vector<std::string> location);

	ScriptAction* resolveScriptAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string>& currentLocation, std::vector<std::string>& resolvedLocation);

	nt_private:
		PortHandler* m_portHandler;
		VariableHandler* m_variableHandler;
		TriggerHandler* m_triggerHandler;
		SampleRateReader* m_sampleRateReader;
		EventListener* m_eventListener;
		AssertListener* m_assertListener;
		std::shared_ptr<RandValueGenerator> m_randomValueGenerator;
};

struct ProcessorLoader {
	ProcessorLoader(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener);
	ProcessorLoader(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener, std::shared_ptr<RandValueGenerator> randomValueGenerator);
	virtual ~ProcessorLoader();

	virtual std::shared_ptr<Processor> loadScript(std::shared_ptr<Script> script, std::vector<ValidationError> *validationErrors);

	nt_private:
		ProcessorScriptParser m_processorScriptParser;
};

}
