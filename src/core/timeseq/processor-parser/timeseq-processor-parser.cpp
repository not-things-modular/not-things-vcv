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


ProcessorScriptParser::ProcessorScriptParser(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener, shared_ptr<RandValueGenerator> randomValueGenerator) {
	m_portHandler = portHandler;
	m_variableHandler = variableHandler;
	m_triggerHandler = triggerHandler;
	m_sampleRateReader = sampleRateReader;
	m_eventListener = eventListener;
	m_assertListener = assertListener;
	m_randomValueGenerator = randomValueGenerator;
}

shared_ptr<Processor> ProcessorScriptParser::parseScript(shared_ptr<Script> script, vector<ValidationError>& validationErrors) {
	m_context.script = script.get();
	m_context.validationErrors = &validationErrors;

	int count = 0;
	for (vector<ScriptSequence>::iterator it = script->sequences.begin(); it != script->sequences.end(); it++) {
		m_context.stashLocation();
		m_context.location = { "component-pool",  "sequences", to_string(count) };
		parseSequence(&(*it));
		m_context.popLocation();
	}

	count = 0;
	m_context.location.push_back("timelines");
	vector<shared_ptr<TimelineProcessor>> timelineProcessors;
	for (vector<ScriptTimeline>::iterator it = script->timelines.begin(); it != script->timelines.end(); it++) {
		m_context.location.push_back(to_string(count));
		timelineProcessors.push_back(parseTimeline(&(*it)));
		m_context.location.pop_back();
		count++;
	}
	m_context.location.pop_back();

	count = 0;
	m_context.location.push_back("input-triggers");
	vector<shared_ptr<TriggerProcessor>> triggerProcessors;
	for (vector<ScriptInputTrigger>::iterator it = script->inputTriggers.begin(); it != script->inputTriggers.end(); it++) {
		m_context.location.push_back(to_string(count));
		triggerProcessors.push_back(parseInputTrigger(&(*it)));
		m_context.location.pop_back();
		count++;
	}
	m_context.location.pop_back();

	count = 0;
	m_context.location.push_back("global-actions");
	vector<shared_ptr<ActionProcessor>> startActionProcessors;
	for (vector<ScriptAction>::iterator it = script->globalActions.begin(); it != script->globalActions.end(); it++) {
		m_context.location.push_back(to_string(count));

		vector<string> actionLocation;
		ScriptAction& scriptAction = *it;
		ScriptAction* resolvedAction = resolveScriptAction(&scriptAction, actionLocation);

		if (resolvedAction) {
			if (resolvedAction->timing == ScriptAction::ActionTiming::START) {
				startActionProcessors.push_back(parseResolvedAction(resolvedAction));
			} else {
				ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Script_GlobalActionTiming, "'global-actions' actions can only have a 'start' timing.");
			}
		} else {
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced action with id '", scriptAction.ref.c_str(), "' in the script actions.");
		}
		m_context.location.pop_back();
		count++;
	}
	m_context.location.pop_back();

	return make_shared<Processor>(script, timelineProcessors, triggerProcessors, startActionProcessors);
}

shared_ptr<TimelineProcessor> ProcessorScriptParser::parseTimeline(ScriptTimeline* scriptTimeline) {
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> startTriggers;
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> stopTriggers;

	int count = 0;
	m_context.location.push_back("lanes");
	vector<shared_ptr<LaneProcessor>> laneProcessors;
	for (vector<ScriptLane>::iterator it = scriptTimeline->lanes.begin(); it != scriptTimeline->lanes.end(); it++) {
		ScriptLane& scriptLane = *it;

		shared_ptr<LaneProcessor> laneProcessor;
		m_context.location.push_back(to_string(count));
		laneProcessor = parseLane(&scriptLane, scriptTimeline->timeScale.get());
		laneProcessors.push_back(laneProcessor);
		m_context.location.pop_back();
		count++;

		if (scriptLane.startTrigger.length() > 0) {
			if (startTriggers.find(scriptLane.startTrigger) == startTriggers.end()) {
				startTriggers[scriptLane.startTrigger] = vector<shared_ptr<LaneProcessor>>();
			}
			startTriggers[scriptLane.startTrigger].push_back(laneProcessor);
		}

		if (scriptLane.stopTrigger.length() > 0) {
			if (stopTriggers.find(scriptLane.stopTrigger) == stopTriggers.end()) {
				stopTriggers[scriptLane.stopTrigger] = vector<shared_ptr<LaneProcessor>>();
			}
			stopTriggers[scriptLane.stopTrigger].push_back(laneProcessor);
		}
	}
	m_context.location.pop_back();

	return make_shared<TimelineProcessor>(scriptTimeline, laneProcessors, startTriggers, stopTriggers, m_triggerHandler);
}

shared_ptr<LaneProcessor> ProcessorScriptParser::parseLane(ScriptLane* scriptLane, ScriptTimeScale* timeScale) {
	unsigned int validationCount = m_context.validationErrors->size();

	m_context.location.push_back("segments");
	vector<shared_ptr<SegmentProcessor>> segmentProcessors = parseSegments(&scriptLane->segments, timeScale, vector<string>());
	m_context.location.pop_back();

	// Only return an actual processor if there were no validation errors during parsing. Otherwise there might be partially loaded children, and we can't reliably continue with this processor.
	if (validationCount == m_context.validationErrors->size()) {
		return make_shared<LaneProcessor>(scriptLane, segmentProcessors, m_eventListener);
	} else {
		return shared_ptr<LaneProcessor>();
	}
}

shared_ptr<IfProcessor> ProcessorScriptParser::parseIf(ScriptIf* scriptIf, vector<string> ifStack) {
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
			m_context.location.push_back("0");
			values.first = parseValue(&scriptIf->values.get()->first, vector<string>());
			m_context.location.pop_back();
			m_context.location.push_back("1");
			values.second = parseValue(&scriptIf->values.get()->second, vector<string>());
			m_context.location.pop_back();
		}
		if (parseIfs) {
			int count = 0;
			for (ScriptIf& scriptIf : *scriptIf->ifs.get()) {
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
			for (vector<ScriptIf>::iterator it = m_context.script->ifs.begin(); it != m_context.script->ifs.end(); it++) {
				if (scriptIf->ref.compare(it->id) == 0) {
					m_context.stashLocation();
					m_context.location = { "component-pool",  "ifs", to_string(count) };
					ifStack.push_back(scriptIf->ref);
					shared_ptr<IfProcessor> ifProcessor = parseIf(&(*it), ifStack);
					ifStack.pop_back();
					m_context.popLocation();
					return ifProcessor;
				}
				count++;
			}

			// Couldn't find the referenced if...
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced if with id '", scriptIf->ref.c_str(), "' in the script ifs.");
		} else {
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_CircularFound, "Encountered a circular if reference while processing the if with the id '", scriptIf->ref.c_str(), "'. Circular references can not be resolved.");
		}
	}

	return shared_ptr<IfProcessor>();
}

void ProcessorScriptParser::parseSequence(ScriptSequence* scriptSequence) {
	vector<shared_ptr<ValueProcessor>> values;
	for (vector<ScriptValue>::iterator it = scriptSequence->values.begin(); it != scriptSequence->values.end(); it++) {
		values.push_back(parseValue(&(*it), vector<string>()));
	}
	shared_ptr<SequenceProcessor> sequenceProcessor = make_shared<SequenceProcessor>(scriptSequence->id, values, scriptSequence->retrieveVoltageOnce);

	if (scriptSequence->shared) {
		m_context.sharedSequences.push_back(make_shared<SequencePositionProcessor>(sequenceProcessor, m_randomValueGenerator));
	} else {
		m_context.nonSharedSequences.push_back(sequenceProcessor);
	}
}

shared_ptr<SequencePositionProcessor> ProcessorScriptParser::resolveSharedSequence(string id) {
	for (vector<shared_ptr<SequencePositionProcessor>>::iterator it = m_context.sharedSequences.begin(); it != m_context.sharedSequences.end(); it++) {
		if (id == it->get()->getSequenceProcessor()->getId()) {
			return *it;
		}
	}

	return nullptr;
}

bool ProcessorScriptParser::hasNonSharedSequence(string id) {
	for (vector<shared_ptr<SequenceProcessor>>::iterator it = m_context.nonSharedSequences.begin(); it != m_context.nonSharedSequences.end(); it++) {
		if (id == it->get()->getId()) {
			return true;
		}
	}

	return false;
}

shared_ptr<SequencePositionProcessor> ProcessorScriptParser::resolveNonSharedSequence(string id) {
	for (vector<shared_ptr<SequenceProcessor>>::iterator it = m_context.nonSharedSequences.begin(); it != m_context.nonSharedSequences.end(); it++) {
		if (id == it->get()->getId()) {
			return make_shared<SequencePositionProcessor>(*it, m_randomValueGenerator);
		}
	}

	return nullptr;
}

ProcessorLoader::ProcessorLoader(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener) : m_portHandler(portHandler), m_variableHandler(variableHandler), m_triggerHandler(triggerHandler), m_sampleRateReader(sampleRateReader), m_eventListener(eventListener), m_assertListener(assertListener), m_randomValueGenerator(make_shared<RandValueGenerator>()) {}
ProcessorLoader::ProcessorLoader(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener, shared_ptr<RandValueGenerator> randomValueGenerator) : m_portHandler(portHandler), m_variableHandler(variableHandler), m_triggerHandler(triggerHandler), m_sampleRateReader(sampleRateReader), m_eventListener(eventListener), m_assertListener(assertListener), m_randomValueGenerator(randomValueGenerator) {}

ProcessorLoader::~ProcessorLoader() {
}

shared_ptr<Processor> ProcessorLoader::loadScript(shared_ptr<Script> script, vector<ValidationError>& validationErrors) {
	ProcessorScriptParser processorScriptParser(m_portHandler, m_variableHandler, m_triggerHandler, m_sampleRateReader, m_eventListener, m_assertListener, m_randomValueGenerator);
	return processorScriptParser.parseScript(script, validationErrors);
}
