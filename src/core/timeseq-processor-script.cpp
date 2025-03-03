#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"
#include <sstream>
#include <stdarg.h>

using namespace std;
using namespace timeseq;
	

ProcessorScriptParser::ProcessorScriptParser(PortReader* portReader, SampleRateReader* sampleRateReader, PortWriter* portWriter) {
	m_portReader = portReader;
	m_sampleRateReader = sampleRateReader;
	m_portWriter = portWriter;
}

shared_ptr<Processor> ProcessorScriptParser::parseScript(Script* script, vector<ValidationError> *validationErrors, vector<string> location) {
	ProcessorScriptParseContext context;

	context.script = script;
	context.validationErrors = validationErrors;

	int count = 0;
	location.push_back("timelines");
	vector<shared_ptr<TimelineProcessor>> timelineProcessors;
	for (vector<ScriptTimeline>::iterator it = script->timelines.begin(); it != script->timelines.end(); it++) {
		location.push_back(to_string(count));
		timelineProcessors.push_back(parseTimeline(&context, &(*it), location));
		location.pop_back();
		count++;
	}
	location.pop_back();

	count = 0;
	location.push_back("input-triggers");
	vector<shared_ptr<TriggerProcessor>> triggerProcessors;
	for (vector<ScriptInputTrigger>::iterator it = script->inputTriggers.begin(); it != script->inputTriggers.end(); it++) {
		location.push_back(to_string(count));
		triggerProcessors.push_back(parseInputTrigger(&context, &(*it), location));
		location.pop_back();
		count++;
	}
	location.pop_back();

	return shared_ptr<Processor>(new Processor(timelineProcessors, triggerProcessors));
}

shared_ptr<TimelineProcessor> ProcessorScriptParser::parseTimeline(ProcessorScriptParseContext* context, ScriptTimeline* scriptTimeline, vector<string> location) {
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> startTriggers;
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> stopTriggers;

	int count = 0;
	location.push_back("lanes");
	vector<shared_ptr<LaneProcessor>> laneProcessors;
	for (vector<ScriptLane>::iterator it = scriptTimeline->lanes.begin(); it != scriptTimeline->lanes.end(); it++) {
		ScriptLane& scriptLane = *it;

		shared_ptr<LaneProcessor> laneProcessor;
		location.push_back(to_string(count));
		laneProcessor = parseLane(context, &scriptLane, scriptTimeline->timeScale.get(), location);
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

shared_ptr<TriggerProcessor> ProcessorScriptParser::parseInputTrigger(ProcessorScriptParseContext* context, ScriptInputTrigger* scriptInputTrigger, vector<string> location) {
	// Check if it's a ref input trigger object or a full one
	if (scriptInputTrigger->input.ref.length() == 0) {
		return shared_ptr<TriggerProcessor>(new TriggerProcessor(scriptInputTrigger->id, scriptInputTrigger->input.index, ((bool) scriptInputTrigger->input.channel) ? *scriptInputTrigger->input.channel.get() : 0, m_portReader));
	} else {
		for (vector<ScriptInput>::iterator it = context->script->inputs.begin(); it != context->script->inputs.end(); it++) {
			if (scriptInputTrigger->input.ref.compare(it->id)) {
				return shared_ptr<TriggerProcessor>(new TriggerProcessor(scriptInputTrigger->id, it->index, ((bool) it->channel) ? *it->channel.get() : 0, m_portReader));
			}
		}

		// Couldn't find the referenced input...
		location.push_back("input");
		ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced input with id '", scriptInputTrigger->input.ref.c_str(), "' in the script inputs.");
		location.pop_back();
		return shared_ptr<TriggerProcessor>();
	}
}

shared_ptr<LaneProcessor> ProcessorScriptParser::parseLane(ProcessorScriptParseContext* context, ScriptLane* scriptLane, ScriptTimeScale* timeScale, vector<string> location) {
	location.push_back("segments");
	vector<shared_ptr<SegmentProcessor>> segmentProcessors = parseSegmentEntities(context, &scriptLane->segments, timeScale, location);
	location.pop_back();

	return shared_ptr<LaneProcessor>(new LaneProcessor(scriptLane, segmentProcessors));
}

vector<shared_ptr<SegmentProcessor>> ProcessorScriptParser::parseSegmentEntities(ProcessorScriptParseContext* context, vector<ScriptSegmentEntity>* scriptSegmentEntities, ScriptTimeScale* timeScale, vector<string> location) {
	int count = 0;
	vector<shared_ptr<SegmentProcessor>> segmentProcessors;

	for (vector<ScriptSegmentEntity>::iterator it = scriptSegmentEntities->begin(); it != scriptSegmentEntities->end(); it++) {
		location.push_back(to_string(count));
		ScriptSegmentEntity& entity = *it;
		if (entity.segment) {
			location.push_back("segment");
			segmentProcessors.push_back(parseSegment(context, entity.segment.get(), timeScale, location));
			location.pop_back();
		} else if (entity.segmentBlock) {
			location.push_back("segment-block");
			vector<shared_ptr<SegmentProcessor>> blockSegmentProcessors = parseSegmentBlock(context, entity.segmentBlock.get(), timeScale, location);
			segmentProcessors.insert(segmentProcessors.end(), blockSegmentProcessors.begin(), blockSegmentProcessors.end());
			location.pop_back();
		}
		location.pop_back();
		count++;
	}

	return segmentProcessors;
}

shared_ptr<SegmentProcessor> ProcessorScriptParser::parseSegment(ProcessorScriptParseContext* context, ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, vector<string> location) {
	// Check if it's a ref segment object or a full one
	if (scriptSegment->ref.length() == 0) {
		location.push_back("duration");
		uint64_t duration = -1;
		if (scriptSegment->duration.samples) {
			duration = *scriptSegment->duration.samples.get();
		} else if (scriptSegment->duration.millis) {
			duration = m_sampleRateReader->getSampleRate() / 1000 * (*scriptSegment->duration.millis.get());
		} else if (scriptSegment->duration.beats) {
			if (timeScale->bpm) {
				int bpm = *timeScale->bpm.get();
				int beats = *scriptSegment->duration.beats.get();
				if (scriptSegment->duration.bars) {
					if (timeScale->bpb) {
						beats += ((*scriptSegment->duration.bars.get()) * (*timeScale->bpb.get()));
					} else {
						ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Duration_BarsButNoBpb, "The segment duration uses bars, but no bpb (beats per bar) is specified on the timeline.");
						beats = -1;
					}
				}

				if (beats != -1) {
					duration = m_sampleRateReader->getSampleRate() * beats / bpm * 60;
				}
			} else {
				ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Duration_BeatsButNoBmp, "The segment duration uses beats, but no bpm (beats per minute) is specified on the timeline.");
			}
		}
		location.pop_back();
		// vector<shared_ptr<SegmentProcessor>> segmentProcessors = parseSegmentEntities(context, &scriptSegmentBlock->segments, timeScale, location);
		// location.pop_back();
		// return segmentProcessors;
	} else {
		int count = 0;
		for (vector<ScriptSegment>::iterator it = context->script->segments.begin(); it != context->script->segments.end(); it++) {
			if (scriptSegment->ref.compare(it->id)) {
				vector<string> refLocation = { "script",  "segments", to_string(count) };
				return parseSegment(context, &(*it), timeScale, refLocation);
			}
			count++;
		}

		// Couldn't find the referenced input...
		ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced segment with id '", scriptSegment->ref.c_str(), "' in the script segments.");
		return shared_ptr<SegmentProcessor>();
	}
}

vector<shared_ptr<SegmentProcessor>> ProcessorScriptParser::parseSegmentBlock(ProcessorScriptParseContext* context, ScriptSegmentBlock* scriptSegmentBlock, ScriptTimeScale* timeScale, vector<string> location) {
	// Check if it's a ref segment block object or a full one
	if (scriptSegmentBlock->ref.length() == 0) {
		location.push_back("segments");
		vector<shared_ptr<SegmentProcessor>> segmentProcessors = parseSegmentEntities(context, &scriptSegmentBlock->segments, timeScale, location);
		location.pop_back();
		return segmentProcessors;
	} else {
		int count = 0;
		for (vector<ScriptSegmentBlock>::iterator it = context->script->segmentBlocks.begin(); it != context->script->segmentBlocks.end(); it++) {
			if (scriptSegmentBlock->ref.compare(it->id)) {
				vector<string> refLocation = { "script",  "segment-blocks", to_string(count) };
				return parseSegmentBlock(context, &(*it), timeScale, refLocation);
			}
			count++;
		}

		// Couldn't find the referenced input...
		ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced segment-block with id '", scriptSegmentBlock->ref.c_str(), "' in the script segment-blocks.");
		return vector<shared_ptr<SegmentProcessor>>();
	}
}
