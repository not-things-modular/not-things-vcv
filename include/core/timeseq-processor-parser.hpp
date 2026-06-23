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


struct Script;
struct ScriptTimeline;
struct ScriptInputTrigger;
struct ScriptLane;
struct ScriptTimeScale;
struct ScriptSegment;
struct ScriptSegmentBlock;
struct ScriptDuration;
struct ScriptAction;
struct ScriptValue;
struct ScriptCalc;
struct ScriptIf;
struct ScriptSequence;
struct ScriptInput;
struct ScriptOutput;

struct SequencePositionProcessor;
struct SequenceProcessor;
struct PortHandler;
struct VariableHandler;
struct TriggerHandler;
struct SampleRateReader;
struct EventListener;
struct AssertListener;
struct RandValueGenerator;

struct Processor;
struct TimelineProcessor;
struct TriggerProcessor;
struct LaneProcessor;
struct SegmentProcessor;
struct DurationProcessor;
struct ActionProcessor;
struct ActionGlideProcessor;
struct ActionGateProcessor;
struct ValueProcessor;
struct CalcProcessor;
struct IfProcessor;



struct ProcessorScriptParseContext {
	Script* script;
	std::vector<ValidationError> *validationErrors;

	std::vector<std::shared_ptr<SequencePositionProcessor>> sharedSequences;
	std::vector<std::shared_ptr<SequenceProcessor>> nonSharedSequences;
};

struct ProcessorScriptParser {
	ProcessorScriptParser(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener, std::shared_ptr<RandValueGenerator> randomValueGenerator);

	std::shared_ptr<Processor> parseScript(std::shared_ptr<Script> script, std::vector<ValidationError> *validationErrors, std::vector<std::string>& location);
	std::shared_ptr<TimelineProcessor> parseTimeline(ProcessorScriptParseContext* context, ScriptTimeline* scriptTimeline, std::vector<std::string>& location);
	std::shared_ptr<TriggerProcessor> parseInputTrigger(ProcessorScriptParseContext* context, ScriptInputTrigger* ScriptInputTrigger, std::vector<std::string>& location);
	std::shared_ptr<LaneProcessor> parseLane(ProcessorScriptParseContext* context, ScriptLane* scriptLane, ScriptTimeScale* timeScale, std::vector<std::string>& location);
	std::vector<std::shared_ptr<SegmentProcessor>> parseSegments(ProcessorScriptParseContext* context, std::vector<ScriptSegment>* scriptSegments, ScriptTimeScale* timeScale, std::vector<std::string>& location, std::vector<std::string> segmentStack);
	std::vector<std::shared_ptr<SegmentProcessor>> parseSegment(ProcessorScriptParseContext* context, ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, std::vector<std::string>& location, std::vector<std::string> segmentStack);
	std::shared_ptr<SegmentProcessor> parseResolvedSegment(ProcessorScriptParseContext* context, ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, std::vector<std::string>& location, std::vector<std::string> segmentStack);
	std::vector<std::shared_ptr<SegmentProcessor>> parseSegmentBlock(ProcessorScriptParseContext* context, ScriptSegmentBlock* scriptSegmentBlock, ScriptTimeScale* timeScale, std::vector<ScriptAction>& actions, std::vector<std::string>& location, std::vector<std::string> actionsLocation, std::vector<std::string> segmentStack);
	std::shared_ptr<DurationProcessor> parseDuration(ProcessorScriptParseContext* context, ScriptDuration* scriptDuration, ScriptTimeScale* timeScale, std::vector<std::string>& location);
	std::shared_ptr<ActionProcessor> parseResolvedAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string>& location);
	std::shared_ptr<ActionGlideProcessor> parseResolvedGlideAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string>& location);
	std::shared_ptr<ActionGateProcessor> parseResolvedGateAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string>& location);
	std::shared_ptr<ActionProcessor> parseSetValueAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string>& location);
	std::shared_ptr<ActionProcessor> parseSetVariableAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string>& location);
	std::shared_ptr<ActionProcessor> parseSetPolyphonyAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string>& location);
	std::shared_ptr<ActionProcessor> parseSetLabelAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string>& location);
	std::shared_ptr<ActionProcessor> parseAssertAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string>& location);
	std::shared_ptr<ActionProcessor> parseTriggerAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string>& location);
	std::shared_ptr<ActionProcessor> parseMoveSequenceAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string>& location);
	std::shared_ptr<ActionProcessor> parseClearSequenceAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string>& location);
	std::shared_ptr<ActionProcessor> parseAddToSequenceAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string>& location);
	std::shared_ptr<ActionProcessor> parseRemoveFromSequenceAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string>& location);
	std::shared_ptr<ValueProcessor> parseValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::string>& location, std::vector<std::string> valueStack);
	std::shared_ptr<ValueProcessor> parseStaticValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string>& location);
	std::shared_ptr<ValueProcessor> parseVariableValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string>& location);
	std::shared_ptr<ValueProcessor> parseInputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string>& location);
	std::shared_ptr<ValueProcessor> parseOutputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string>& location);
	std::shared_ptr<ValueProcessor> parseRandValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string>& location, std::vector<std::string> valueStack);
	std::shared_ptr<ValueProcessor> parseSequenceValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string>& location, std::vector<std::string> valueStack);
	std::shared_ptr<CalcProcessor> parseCalc(ProcessorScriptParseContext* context, ScriptCalc* scriptCalc, std::vector<std::string>& location, std::vector<std::string> valueStack);
	std::shared_ptr<IfProcessor> parseIf(ProcessorScriptParseContext* context, ScriptIf* scriptIf, std::vector<std::string>& location, std::vector<std::string> ifStack);

	void parseSequence(ProcessorScriptParseContext* context, ScriptSequence* scriptSequence, std::vector<std::string>& location);

	std::pair<int, int> parseInput(ProcessorScriptParseContext* context, ScriptInput* scriptInput, std::vector<std::string>& location);
	std::pair<int, int> parseOutput(ProcessorScriptParseContext* context, ScriptOutput* scriptOutput, std::vector<std::string>& location);

	ScriptAction* resolveScriptAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::vector<std::string>& currentLocation, std::vector<std::string>& resolvedLocation);

	std::shared_ptr<SequencePositionProcessor> resolveSharedSequence(ProcessorScriptParseContext* context, std::string id);
	bool hasNonSharedSequence(ProcessorScriptParseContext* context, std::string id);
	std::shared_ptr<SequencePositionProcessor> resolveNonSharedSequence(ProcessorScriptParseContext* context, std::string id);

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
