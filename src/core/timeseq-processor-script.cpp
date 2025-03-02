#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include <sstream>
#include <stdarg.h>

using namespace std;
using namespace timeseq;
	

ProcessorScriptParser::ProcessorScriptParser(PortReader* portReader, PortWriter* portWriter) {
	m_portReader = portReader;
	m_portWriter = portWriter;
}

shared_ptr<Processor> ProcessorScriptParser::parseScript(Script* script, vector<ValidationError> *validationErrors, vector<string> location) {
	int count = 0;
	location.push_back("timelines");
	vector<shared_ptr<TimelineProcessor>> timelineProcessors;
	for (vector<ScriptTimeline>::iterator it = script->timelines.begin(); it != script->timelines.end(); it++) {
		location.push_back(to_string(count));
		timelineProcessors.push_back(parseTimeline(&(*it), script, validationErrors, location));
		location.pop_back();
		count++;
	}
	location.pop_back();

	count = 0;
	location.push_back("input-triggers");
	vector<shared_ptr<TriggerProcessor>> triggerProcessors;
	for (vector<ScriptInputTrigger>::iterator it = script->inputTriggers.begin(); it != script->inputTriggers.end(); it++) {
		location.push_back(to_string(count));
		triggerProcessors.push_back(parseInputTrigger(&(*it), script, validationErrors, location));
		location.pop_back();
		count++;
	}
	location.pop_back();

	return shared_ptr<Processor>(new Processor(timelineProcessors, triggerProcessors));
}

shared_ptr<TimelineProcessor> ProcessorScriptParser::parseTimeline(ScriptTimeline* scriptTimeline, Script* script, vector<ValidationError> *validationErrors, vector<string> location) {
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> startTriggers;
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> stopTriggers;

	int count = 0;
	location.push_back("lanes");
	vector<shared_ptr<LaneProcessor>> laneProcessors;
	for (vector<ScriptLane>::iterator it = scriptTimeline->lanes.begin(); it != scriptTimeline->lanes.end(); it++) {
		ScriptLane& scriptLane = *it;

		shared_ptr<LaneProcessor> laneProcessor;
		location.push_back(to_string(count));
		laneProcessor = parseLane(&scriptLane, scriptTimeline->timeScale.get(), script, validationErrors, location);
		laneProcessors.push_back(laneProcessor);
		location.pop_back();
		count++;
		
		laneProcessors.push_back(laneProcessor);
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
	location.pop_back();

	return shared_ptr<TimelineProcessor>(new TimelineProcessor(scriptTimeline, laneProcessors, startTriggers, stopTriggers));
}

shared_ptr<TriggerProcessor> ProcessorScriptParser::parseInputTrigger(ScriptInputTrigger* scriptInputTrigger, Script* script, vector<ValidationError> *validationErrors, vector<string> location) {
	// Check if it's a ref input trigger object or a full one
	if (scriptInputTrigger->input.ref.length() == 0) {
		return shared_ptr<TriggerProcessor>(new TriggerProcessor(scriptInputTrigger->id, scriptInputTrigger->input.index, ((bool) scriptInputTrigger->input.channel) ? *scriptInputTrigger->input.channel.get() : 0, m_portReader));
	} else {
		for (vector<ScriptInput>::iterator it = script->inputs.begin(); it != script->inputs.end(); it++) {
			if (scriptInputTrigger->input.ref.compare(it->id)) {
				return shared_ptr<TriggerProcessor>(new TriggerProcessor(scriptInputTrigger->id, it->index, ((bool) it->channel) ? *it->channel.get() : 0, m_portReader));
			}
		}

		// Couldn't find the referenced input...
		location.push_back("input");
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced input with id '", scriptInputTrigger->input.ref.c_str(), "' in the script inputs.");
		location.pop_back();
		return shared_ptr<TriggerProcessor>();
	}
}

shared_ptr<LaneProcessor> ProcessorScriptParser::parseLane(ScriptLane* scriptLane, ScriptTimeScale* timeScale, Script* script, vector<ValidationError> *validationErrors, vector<string> location) {
	location.push_back("segments");
	vector<shared_ptr<SegmentProcessor>> segmentProcessors = parseSegmentEntities(&scriptLane->segments, timeScale, script, validationErrors, location);
	location.pop_back();

	return shared_ptr<LaneProcessor>(new LaneProcessor(scriptLane, segmentProcessors));
}

vector<shared_ptr<SegmentProcessor>> ProcessorScriptParser::parseSegmentEntities(vector<ScriptSegmentEntity>* scriptSegmentEntities, ScriptTimeScale* timeScale, Script* script, vector<ValidationError> *validationErrors, vector<string> location) {
	int count = 0;
	vector<shared_ptr<SegmentProcessor>> segmentProcessors;

	for (vector<ScriptSegmentEntity>::iterator it = scriptSegmentEntities->begin(); it != scriptSegmentEntities->end(); it++) {
		location.push_back(to_string(count));
		ScriptSegmentEntity& entity = *it;
		if (entity.segment) {
			location.push_back("segment");
			segmentProcessors.push_back(parseSegment(entity.segment.get(), timeScale, script, validationErrors, location));
			location.pop_back();
		} else if (entity.segmentBlock) {
			location.push_back("segment-block");
			vector<shared_ptr<SegmentProcessor>> blockSegmentProcessors = parseSegmentBlock(entity.segmentBlock.get(), timeScale, script, validationErrors, location);
			segmentProcessors.insert(segmentProcessors.end(), blockSegmentProcessors.begin(), blockSegmentProcessors.end());
			location.pop_back();
		}
		location.pop_back();
		count++;
	}

	return segmentProcessors;
}

shared_ptr<SegmentProcessor> ProcessorScriptParser::parseSegment(ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, Script* script, vector<ValidationError> *validationErrors, vector<string> location) {
	return shared_ptr<SegmentProcessor>();
}

vector<shared_ptr<SegmentProcessor>> ProcessorScriptParser::parseSegmentBlock(ScriptSegmentBlock* scriptSegmentBlock, ScriptTimeScale* timeScale, Script* script, vector<ValidationError> *validationErrors, vector<string> location) {
	// Check if it's a ref input trigger object or a full one
	if (scriptSegmentBlock->ref.length() == 0) {
		location.push_back("segments");
		vector<shared_ptr<SegmentProcessor>> segmentProcessors = parseSegmentEntities(&scriptSegmentBlock->segments, timeScale, script, validationErrors, location);
		location.pop_back();
		return segmentProcessors;
	} else {
		int count = 0;
		for (vector<ScriptSegmentBlock>::iterator it = script->segmentBlocks.begin(); it != script->segmentBlocks.end(); it++) {
			if (scriptSegmentBlock->ref.compare(it->id)) {
				vector<string> refLocation = { "script",  "segment-blocks", to_string(count) };
				return parseSegmentBlock(&(*it), timeScale, script, validationErrors, refLocation);
			}
			count++;
		}

		// Couldn't find the referenced input...
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced segment-block with id '", scriptSegmentBlock->ref.c_str(), "' in the script segment-blocks.");
		return vector<shared_ptr<SegmentProcessor>>();
	}
}
