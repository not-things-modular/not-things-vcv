#include "core/timeseq-processor-parser.hpp"
#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"
#include <sstream>
#include <stdarg.h>

using namespace std;
using namespace timeseq;

void ProcessorScriptParseContext::stashLocation() {
	stashedLocations.push_back(location);
}

void ProcessorScriptParseContext::popLocation() {
	location = stashedLocations.back();
	stashedLocations.pop_back();
}


ProcessorScriptParser::ProcessorScriptParser(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, const SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener, const shared_ptr<RandValueGenerator> randomValueGenerator) :
	m_portHandler(portHandler), m_variableHandler(variableHandler), m_triggerHandler(triggerHandler), m_sampleRateReader(sampleRateReader), m_eventListener(eventListener), m_assertListener(assertListener), m_randomValueGenerator(randomValueGenerator) {
}

shared_ptr<Processor> ProcessorScriptParser::parseScript(shared_ptr<Script> script, vector<ValidationError>& validationErrors) {
	m_context.script = script.get();
	m_context.validationErrors = &validationErrors;

	int count = 0;
	for (const ScriptSequence& sequence : script->sequences) {
		m_context.stashLocation();
		m_context.location = { "component-pool",  "sequences", to_string(count) };
		parseSequence(&sequence);
		m_context.popLocation();
	}

	// Both clocks and timelines will be added to the timelineProcessors vector
	vector<shared_ptr<TimelineProcessor>> timelineProcessors;

	count = 0;
	m_context.location.push_back("clocks");
	for (const ScriptClock& clock : script->clocks) {
		m_context.location.push_back(to_string(count));
		timelineProcessors.push_back(parseClock(&clock));
		m_context.location.pop_back();
		count++;
	}
	m_context.location.pop_back();

	count = 0;
	m_context.location.push_back("timelines");
	for (const ScriptTimeline& timeline : script->timelines) {
		m_context.location.push_back(to_string(count));
		timelineProcessors.push_back(parseTimeline(&timeline));
		m_context.location.pop_back();
		count++;
	}
	m_context.location.pop_back();

	count = 0;
	m_context.location.push_back("input-triggers");
	vector<shared_ptr<TriggerProcessor>> triggerProcessors;
	for (const ScriptInputTrigger& trigger : script->inputTriggers) {
		m_context.location.push_back(to_string(count));
		triggerProcessors.push_back(parseInputTrigger(&trigger));
		m_context.location.pop_back();
		count++;
	}
	m_context.location.pop_back();

	count = 0;
	m_context.location.push_back("global-actions");
	vector<shared_ptr<ActionProcessor>> startActionProcessors;
	for (const ScriptAction& action : script->globalActions) {
		m_context.location.push_back(to_string(count));

		vector<string> actionLocation;
		const ScriptAction* resolvedAction = resolveScriptAction(&action, actionLocation);

		if (resolvedAction) {
			if (resolvedAction->timing == ScriptAction::ActionTiming::START) {
				startActionProcessors.push_back(parseResolvedAction(resolvedAction));
			} else {
				addValidationError(m_context.validationErrors, m_context.location, ValidationErrorCode::Script_GlobalActionTiming, "'global-actions' actions can only have a 'start' timing.");
			}
		} else {
			addValidationError(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced action with id '", action.ref.c_str(), "' in the script actions.");
		}
		m_context.location.pop_back();
		count++;
	}
	m_context.location.pop_back();

	return make_shared<Processor>(script, timelineProcessors, triggerProcessors, startActionProcessors);
}

const shared_ptr<TimelineProcessor> ProcessorScriptParser::parseClock(const ScriptClock* scriptClock) {
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> startTriggers;
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> stopTriggers;

	int count = 0;
	m_context.location.push_back("lanes");
	vector<shared_ptr<LaneProcessor>> laneProcessors;
	for (const ScriptClockLane& lane : scriptClock->lanes) {
		shared_ptr<LaneProcessor> laneProcessor;
		m_context.location.push_back(to_string(count));
		laneProcessor = parseClockLane(&lane, scriptClock->timeScale.get());
		laneProcessors.push_back(laneProcessor);
		m_context.location.pop_back();
		count++;

		if (lane.startTrigger.length() > 0) {
			if (startTriggers.find(lane.startTrigger) == startTriggers.end()) {
				startTriggers[lane.startTrigger] = vector<shared_ptr<LaneProcessor>>();
			}
			startTriggers[lane.startTrigger].push_back(laneProcessor);
		}

		if (lane.stopTrigger.length() > 0) {
			if (stopTriggers.find(lane.stopTrigger) == stopTriggers.end()) {
				stopTriggers[lane.stopTrigger] = vector<shared_ptr<LaneProcessor>>();
			}
			stopTriggers[lane.stopTrigger].push_back(laneProcessor);
		}
	}
	m_context.location.pop_back();

	return make_shared<TimelineProcessor>(false, laneProcessors, startTriggers, stopTriggers, m_triggerHandler);
}

const shared_ptr<TimelineProcessor> ProcessorScriptParser::parseTimeline(const ScriptTimeline* scriptTimeline) {
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> startTriggers;
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> stopTriggers;

	int count = 0;
	m_context.location.push_back("lanes");
	vector<shared_ptr<LaneProcessor>> laneProcessors;
	for (const ScriptLane& lane : scriptTimeline->lanes) {
		shared_ptr<LaneProcessor> laneProcessor;
		m_context.location.push_back(to_string(count));
		laneProcessor = parseLane(&lane, scriptTimeline->timeScale.get());
		laneProcessors.push_back(laneProcessor);
		m_context.location.pop_back();
		count++;

		if (lane.startTrigger.length() > 0) {
			if (startTriggers.find(lane.startTrigger) == startTriggers.end()) {
				startTriggers[lane.startTrigger] = vector<shared_ptr<LaneProcessor>>();
			}
			startTriggers[lane.startTrigger].push_back(laneProcessor);
		}

		if (lane.stopTrigger.length() > 0) {
			if (stopTriggers.find(lane.stopTrigger) == stopTriggers.end()) {
				stopTriggers[lane.stopTrigger] = vector<shared_ptr<LaneProcessor>>();
			}
			stopTriggers[lane.stopTrigger].push_back(laneProcessor);
		}
	}
	m_context.location.pop_back();

	return make_shared<TimelineProcessor>(scriptTimeline->loopLock, laneProcessors, startTriggers, stopTriggers, m_triggerHandler);
}

const shared_ptr<LaneProcessor> ProcessorScriptParser::parseClockLane(const ScriptClockLane* scriptClockLane, ScriptTimeScale* timeScale) {
	unsigned int validationCount = m_context.validationErrors->size();

	m_context.location.push_back("output");
	pair<int, int> output = parseOutput(&scriptClockLane->output);
	m_context.location.pop_back();
	int outputPort = output.first;
	int outputChannel = output.second;


	int count = 0;
	m_context.location.push_back("durations");
	for (const ScriptDuration& scriptDuration : scriptClockLane->durations) {
		m_context.location.push_back(to_string(count));
		shared_ptr<DurationProcessor> durationProcessor = parseDuration(&scriptSegment->duration, timeScale);
		m_context.location.pop_back();
		count++;


	}
	m_context.location.pop_back();

	// Only return an actual processor if there were no validation errors during parsing. Otherwise there might be partially loaded children, and we can't reliably continue with this processor.
	if (validationCount == m_context.validationErrors->size()) {
		return make_shared<LaneProcessor>(scriptLane, segmentProcessors, m_eventListener);
	} else {
		return shared_ptr<LaneProcessor>();
	}
}

const shared_ptr<LaneProcessor> ProcessorScriptParser::parseLane(const ScriptLane* scriptLane, ScriptTimeScale* timeScale) {
	unsigned int validationCount = m_context.validationErrors->size();

	m_context.location.push_back("segments");
	vector<string> stack;
	vector<shared_ptr<SegmentProcessor>> segmentProcessors = parseSegments(&scriptLane->segments, timeScale, stack);
	m_context.location.pop_back();

	// Only return an actual processor if there were no validation errors during parsing. Otherwise there might be partially loaded children, and we can't reliably continue with this processor.
	if (validationCount == m_context.validationErrors->size()) {
		return make_shared<LaneProcessor>(scriptLane, segmentProcessors, m_eventListener);
	} else {
		return shared_ptr<LaneProcessor>();
	}
}

const shared_ptr<IfProcessor> ProcessorScriptParser::parseIf(const ScriptIf* scriptIf, vector<string>& ifStack) {
	if (scriptIf->ref.length() == 0) {
		pair<shared_ptr<ValueProcessor>, shared_ptr<ValueProcessor>> values;
		vector<shared_ptr<IfProcessor>> ifs;
		bool parseValues = true;
		bool parseIfs = false;

		switch (scriptIf->ifOperator) {
			case ScriptIf::IfOperator::EQ:
				m_context.location.push_back("eq");
				break;
			case ScriptIf::IfOperator::NE:
				m_context.location.push_back("ne");
				break;
			case ScriptIf::IfOperator::LT:
				m_context.location.push_back("lt");
				break;
			case ScriptIf::IfOperator::LTE:
				m_context.location.push_back("lte");
				break;
			case ScriptIf::IfOperator::GT:
				m_context.location.push_back("gt");
				break;
			case ScriptIf::IfOperator::GTE:
				m_context.location.push_back("gte");
				break;
			case ScriptIf::IfOperator::AND:
				parseValues = false;
				parseIfs = true;
				m_context.location.push_back("and");
				break;
			case ScriptIf::IfOperator::OR:
				parseValues = false;
				parseIfs = true;
				m_context.location.push_back("or");
				break;
		}

		if (parseValues) {
			vector<string> stack;
			m_context.location.push_back("0");
			values.first = parseValue(&scriptIf->values.get()->first, stack);
			m_context.location.pop_back();
			stack.clear();
			m_context.location.push_back("1");
			values.second = parseValue(&scriptIf->values.get()->second, stack);
			m_context.location.pop_back();
		}
		if (parseIfs) {
			int count = 0;
			for (const ScriptIf& scriptIf : *scriptIf->ifs.get()) {
				m_context.location.push_back(to_string(count));
				ifs.push_back(parseIf(&scriptIf, ifStack));
				m_context.location.pop_back();
				count++;
			}
		}

		m_context.location.pop_back();

		return make_shared<IfProcessor>(scriptIf, values, ifs);
	} else {
		if (find(ifStack.begin(), ifStack.end(), scriptIf->ref) == ifStack.end()) {
			int count = 0;
			for (const ScriptIf& refIf : m_context.script->ifs) {
				if (scriptIf->ref.compare(refIf.id) == 0) {
					m_context.stashLocation();
					m_context.location = { "component-pool",  "ifs", to_string(count) };
					ifStack.push_back(scriptIf->ref);
					shared_ptr<IfProcessor> ifProcessor = parseIf(&refIf, ifStack);
					ifStack.pop_back();
					m_context.popLocation();
					return ifProcessor;
				}
				count++;
			}

			// Couldn't find the referenced if...
			addValidationError(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced if with id '", scriptIf->ref.c_str(), "' in the script ifs.");
		} else {
			addValidationError(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_CircularFound, "Encountered a circular if reference while processing the if with the id '", scriptIf->ref.c_str(), "'. Circular references can not be resolved.");
		}
	}

	return shared_ptr<IfProcessor>();
}

void ProcessorScriptParser::parseSequence(const ScriptSequence* scriptSequence) {
	vector<shared_ptr<ValueProcessor>> values;
	for (const ScriptValue& value : scriptSequence->values) {
		vector<string> stack;
		values.push_back(parseValue(&value, stack));
	}
	shared_ptr<SequenceProcessor> sequenceProcessor = make_shared<SequenceProcessor>(scriptSequence->id, values, scriptSequence->retrieveVoltageOnce);

	if (scriptSequence->shared) {
		m_context.sharedSequences.push_back(make_shared<SequencePositionProcessor>(sequenceProcessor, m_randomValueGenerator));
	} else {
		m_context.nonSharedSequences.push_back(sequenceProcessor);
	}
}

const shared_ptr<SequencePositionProcessor> ProcessorScriptParser::resolveSharedSequence(const string& id) const {
	for (const shared_ptr<SequencePositionProcessor>& sequencePositionProcessor : m_context.sharedSequences) {
		if (id == sequencePositionProcessor->getSequenceProcessor()->getId()) {
			return sequencePositionProcessor;
		}
	}

	return shared_ptr<SequencePositionProcessor>();
}

bool ProcessorScriptParser::hasNonSharedSequence(const string& id) const {
	for (const shared_ptr<SequenceProcessor>& sequenceProcessor : m_context.nonSharedSequences) {
		if (id == sequenceProcessor.get()->getId()) {
			return true;
		}
	}

	return false;
}

const shared_ptr<SequencePositionProcessor> ProcessorScriptParser::resolveNonSharedSequence(const string& id) const {
	for (const shared_ptr<SequenceProcessor>& sequenceProcessor : m_context.nonSharedSequences) {
		if (id == sequenceProcessor->getId()) {
			return make_shared<SequencePositionProcessor>(sequenceProcessor, m_randomValueGenerator);
		}
	}

	return nullptr;
}

ProcessorLoader::ProcessorLoader(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, const SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener) : m_portHandler(portHandler), m_variableHandler(variableHandler), m_triggerHandler(triggerHandler), m_sampleRateReader(sampleRateReader), m_eventListener(eventListener), m_assertListener(assertListener), m_randomValueGenerator(make_shared<RandValueGenerator>()) {}
ProcessorLoader::ProcessorLoader(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, const SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener, const shared_ptr<RandValueGenerator> randomValueGenerator) : m_portHandler(portHandler), m_variableHandler(variableHandler), m_triggerHandler(triggerHandler), m_sampleRateReader(sampleRateReader), m_eventListener(eventListener), m_assertListener(assertListener), m_randomValueGenerator(randomValueGenerator) {}

ProcessorLoader::~ProcessorLoader() {
}

const shared_ptr<Processor> ProcessorLoader::loadScript(const shared_ptr<Script>& script, vector<ValidationError>& validationErrors) {
	ProcessorScriptParser processorScriptParser(m_portHandler, m_variableHandler, m_triggerHandler, m_sampleRateReader, m_eventListener, m_assertListener, m_randomValueGenerator);
	return processorScriptParser.parseScript(script, validationErrors);
}
