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

	std::vector<std::string> location;
	std::vector<std::vector<std::string>> stashedLocations;

	void stashLocation();
	void popLocation();
};

struct ProcessorScriptParser {
	ProcessorScriptParser(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener, std::shared_ptr<RandValueGenerator> randomValueGenerator);

	std::shared_ptr<Processor> parseScript(std::shared_ptr<Script> script, std::vector<ValidationError> *validationErrors);

	nt_private:
		std::shared_ptr<TimelineProcessor> parseTimeline(ScriptTimeline* scriptTimeline);
		std::shared_ptr<TriggerProcessor> parseInputTrigger(ScriptInputTrigger* ScriptInputTrigger);
		std::shared_ptr<LaneProcessor> parseLane(ScriptLane* scriptLane, ScriptTimeScale* timeScale);
		std::vector<std::shared_ptr<SegmentProcessor>> parseSegments(std::vector<ScriptSegment>* scriptSegments, ScriptTimeScale* timeScale, std::vector<std::string> segmentStack);
		std::vector<std::shared_ptr<SegmentProcessor>> parseSegment(ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, std::vector<std::string> segmentStack);
		std::shared_ptr<SegmentProcessor> parseResolvedSegment(ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, std::vector<std::string> segmentStack);
		std::vector<std::shared_ptr<SegmentProcessor>> parseSegmentBlock(ScriptSegmentBlock* scriptSegmentBlock, ScriptTimeScale* timeScale, std::vector<ScriptAction>& actions, std::vector<std::string> actionsLocation, std::vector<std::string> segmentStack);
		std::shared_ptr<DurationProcessor> parseDuration(ScriptDuration* scriptDuration, ScriptTimeScale* timeScale);
		std::shared_ptr<ActionProcessor> parseResolvedAction(ScriptAction* scriptAction);
		std::shared_ptr<ActionGlideProcessor> parseResolvedGlideAction(ScriptAction* scriptAction);
		std::shared_ptr<ActionGateProcessor> parseResolvedGateAction(ScriptAction* scriptAction);
		std::shared_ptr<ActionProcessor> parseSetValueAction(ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor);
		std::shared_ptr<ActionProcessor> parseSetVariableAction(ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor);
		std::shared_ptr<ActionProcessor> parseSetPolyphonyAction(ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor);
		std::shared_ptr<ActionProcessor> parseSetLabelAction(ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor);
		std::shared_ptr<ActionProcessor> parseAssertAction(ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor);
		std::shared_ptr<ActionProcessor> parseTriggerAction(ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor);
		std::shared_ptr<ActionProcessor> parseMoveSequenceAction(ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor);
		std::shared_ptr<ActionProcessor> parseClearSequenceAction(ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor);
		std::shared_ptr<ActionProcessor> parseAddToSequenceAction(ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor);
		std::shared_ptr<ActionProcessor> parseRemoveFromSequenceAction(ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor);
		std::shared_ptr<ValueProcessor> parseValue(ScriptValue* scriptValue, std::vector<std::string> valueStack);
		std::shared_ptr<ValueProcessor> parseStaticValue(ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors);
		std::shared_ptr<ValueProcessor> parseVariableValue(ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors);
		std::shared_ptr<ValueProcessor> parseInputValue(ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors);
		std::shared_ptr<ValueProcessor> parseOutputValue(ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors);
		std::shared_ptr<ValueProcessor> parseRandValue(ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> valueStack);
		std::shared_ptr<ValueProcessor> parseSequenceValue(ScriptValue* scriptValue, std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string> valueStack);
		std::shared_ptr<CalcProcessor> parseCalc(ScriptCalc* scriptCalc, std::vector<std::string> valueStack);
		std::shared_ptr<IfProcessor> parseIf(ScriptIf* scriptIf, std::vector<std::string> ifStack);

		void parseSequence(ScriptSequence* scriptSequence);

		std::pair<int, int> parseInput(ScriptInput* scriptInput);
		std::pair<int, int> parseOutput(ScriptOutput* scriptOutput);

		ScriptAction* resolveScriptAction(ScriptAction* scriptAction, std::vector<std::string>& resolvedLocation);

		std::shared_ptr<SequencePositionProcessor> resolveSharedSequence(std::string id);
		bool hasNonSharedSequence(std::string id);
		std::shared_ptr<SequencePositionProcessor> resolveNonSharedSequence(std::string id);

		ProcessorScriptParseContext m_context;
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

	private:
		PortHandler* m_portHandler;
		VariableHandler* m_variableHandler;
		TriggerHandler* m_triggerHandler;
		SampleRateReader* m_sampleRateReader;
		EventListener* m_eventListener;
		AssertListener* m_assertListener;
		std::shared_ptr<RandValueGenerator> m_randomValueGenerator;
};

}
