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
	ProcessorScriptParser(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, const SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener, const std::shared_ptr<RandValueGenerator> randomValueGenerator);

	std::shared_ptr<Processor> parseScript(const std::shared_ptr<Script> script, std::vector<ValidationError>& validationErrors);

	nt_private:
		const std::shared_ptr<TimelineProcessor> parseTimeline(const ScriptTimeline* scriptTimeline);
		const std::shared_ptr<TriggerProcessor> parseInputTrigger(const ScriptInputTrigger* ScriptInputTrigger);
		const std::shared_ptr<LaneProcessor> parseLane(const ScriptLane* scriptLane, ScriptTimeScale* timeScale);
		const std::vector<std::shared_ptr<SegmentProcessor>> parseSegments(const std::vector<ScriptSegment>* scriptSegments, const ScriptTimeScale* timeScale, std::vector<std::string>& segmentStack);
		const std::vector<std::shared_ptr<SegmentProcessor>> parseSegment(const ScriptSegment* scriptSegment, const ScriptTimeScale* timeScale, std::vector<std::string>& segmentStack);
		const std::shared_ptr<SegmentProcessor> parseResolvedSegment(const ScriptSegment* scriptSegment, const ScriptTimeScale* timeScale, std::vector<std::string>& segmentStack);
		const std::vector<std::shared_ptr<SegmentProcessor>> parseSegmentBlock(const ScriptSegmentBlock* scriptSegmentBlock, const ScriptTimeScale* timeScale, const std::vector<ScriptAction>& actions, std::vector<std::string>& actionsLocation, std::vector<std::string>& segmentStack);
		const std::shared_ptr<DurationProcessor> parseDuration(const ScriptDuration* scriptDuration, const ScriptTimeScale* timeScale);
		const std::shared_ptr<ActionProcessor> parseResolvedAction(const ScriptAction* scriptAction);
		const std::shared_ptr<ActionGlideProcessor> parseResolvedGlideAction(const ScriptAction* scriptAction);
		const std::shared_ptr<ActionGateProcessor> parseResolvedGateAction(const ScriptAction* scriptAction);
		const std::shared_ptr<ActionProcessor> parseSetValueAction(const ScriptAction* scriptAction, const std::shared_ptr<IfProcessor>& ifProcessor);
		const std::shared_ptr<ActionProcessor> parseSetVariableAction(const ScriptAction* scriptAction, const std::shared_ptr<IfProcessor>& ifProcessor);
		const std::shared_ptr<ActionProcessor> parseSetPolyphonyAction(const ScriptAction* scriptAction, const std::shared_ptr<IfProcessor>& ifProcessor);
		const std::shared_ptr<ActionProcessor> parseSetLabelAction(const ScriptAction* scriptAction, const std::shared_ptr<IfProcessor>& ifProcessor);
		const std::shared_ptr<ActionProcessor> parseAssertAction(const ScriptAction* scriptAction, const std::shared_ptr<IfProcessor>& ifProcessor);
		const std::shared_ptr<ActionProcessor> parseTriggerAction(const ScriptAction* scriptAction, const std::shared_ptr<IfProcessor>& ifProcessor);
		const std::shared_ptr<ActionProcessor> parseMoveSequenceAction(const ScriptAction* scriptAction, const std::shared_ptr<IfProcessor>& ifProcessor);
		const std::shared_ptr<ActionProcessor> parseClearSequenceAction(const ScriptAction* scriptAction, const std::shared_ptr<IfProcessor>& ifProcessor);
		const std::shared_ptr<ActionProcessor> parseAddToSequenceAction(const ScriptAction* scriptAction, const std::shared_ptr<IfProcessor>& ifProcessor);
		const std::shared_ptr<ActionProcessor> parseRemoveFromSequenceAction(const ScriptAction* scriptAction, const std::shared_ptr<IfProcessor>& ifProcessor);
		const std::shared_ptr<ValueProcessor> parseValue(const ScriptValue* scriptValue, std::vector<std::string>& valueStack);
		const std::shared_ptr<ValueProcessor> parseStaticValue(const ScriptValue* scriptValue, const std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors);
		const std::shared_ptr<ValueProcessor> parseVariableValue(const ScriptValue* scriptValue, const std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors);
		const std::shared_ptr<ValueProcessor> parseInputValue(const ScriptValue* scriptValue, const std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors);
		const std::shared_ptr<ValueProcessor> parseOutputValue(const ScriptValue* scriptValue, const std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors);
		const std::shared_ptr<ValueProcessor> parseRandValue(const ScriptValue* scriptValue, const std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string>& valueStack);
		const std::shared_ptr<ValueProcessor> parseSequenceValue(const ScriptValue* scriptValue, const std::vector<std::shared_ptr<CalcProcessor>>& calcProcessors, std::vector<std::string>& valueStack);
		const std::shared_ptr<CalcProcessor> parseCalc(const ScriptCalc* scriptCalc, std::vector<std::string>& valueStack);
		const std::shared_ptr<IfProcessor> parseIf(const ScriptIf* scriptIf, std::vector<std::string>& ifStack);

		void parseSequence(const ScriptSequence* scriptSequence);

		const std::pair<int, int> parseInput(const ScriptInput* scriptInput);
		const std::pair<int, int> parseOutput(const ScriptOutput* scriptOutput);

		const ScriptAction* resolveScriptAction(const ScriptAction* scriptAction, std::vector<std::string>& resolvedLocation) const;

		const std::shared_ptr<SequencePositionProcessor> resolveSharedSequence(const std::string& id) const;
		bool hasNonSharedSequence(const std::string& id) const;
		const std::shared_ptr<SequencePositionProcessor> resolveNonSharedSequence(const std::string& id) const;

		ProcessorScriptParseContext m_context;
		PortHandler* m_portHandler;
		VariableHandler* m_variableHandler;
		TriggerHandler* m_triggerHandler;
		const SampleRateReader* m_sampleRateReader;
		EventListener* m_eventListener;
		AssertListener* m_assertListener;
		const std::shared_ptr<RandValueGenerator> m_randomValueGenerator;
};

struct ProcessorLoader {
	ProcessorLoader(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, const SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener);
	ProcessorLoader(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, const SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener, const std::shared_ptr<RandValueGenerator> randomValueGenerator);
	virtual ~ProcessorLoader();

	virtual const std::shared_ptr<Processor> loadScript(const std::shared_ptr<Script>& script, std::vector<ValidationError>& validationErrors);

	private:
		PortHandler* m_portHandler;
		VariableHandler* m_variableHandler;
		TriggerHandler* m_triggerHandler;
		const SampleRateReader* m_sampleRateReader;
		EventListener* m_eventListener;
		AssertListener* m_assertListener;
		const std::shared_ptr<RandValueGenerator> m_randomValueGenerator;
};

}
