#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"
#include <sstream>
#include <stdarg.h>
#include <cctype>

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

shared_ptr<SegmentProcessor> ProcessorScriptParser::parseSegment(ProcessorScriptParseContext* context, ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, vector<string> location) {
	// Check if it's a ref segment object or a full one
	if (scriptSegment->ref.length() == 0) {
		location.push_back("duration");
		shared_ptr<DurationProcessor> durationProcessor = parseDuration(context, &scriptSegment->duration, timeScale, location);
		location.pop_back();

		int count = 0;
		vector<shared_ptr<ActionProcessor>> startActions;
		vector<shared_ptr<ActionProcessor>> endActions;
		vector<shared_ptr<ActionGlideProcessor>> glideActions;
		location.push_back("actions");
		for (vector<ScriptAction>::iterator it = scriptSegment->actions.begin(); it != scriptSegment->actions.end(); it++) {
			ScriptAction& scriptAction = *it;
			location.push_back(to_string(count));
			if (scriptAction.timing == ScriptAction::ActionTiming::START) {
				startActions.push_back(parseAction(context, &(*it), location));
			} else if (scriptAction.timing == ScriptAction::ActionTiming::END) {
				endActions.push_back(parseAction(context, &(*it), location));
			} else if (scriptAction.timing == ScriptAction::ActionTiming::GLIDE) {
				glideActions.push_back(parseGlideAction(context, &(*it), location));
			}
			location.pop_back();
			count++;
		}
		location.pop_back();

		return shared_ptr<SegmentProcessor>(new SegmentProcessor(scriptSegment, durationProcessor, startActions, endActions, glideActions));
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

shared_ptr<DurationProcessor> ProcessorScriptParser::parseDuration(ProcessorScriptParseContext* context, ScriptDuration* scriptDuration, ScriptTimeScale* timeScale, vector<string> location) {
	uint64_t duration = 0;
	double drift = 0;

	if (scriptDuration->samples) {
		float activeSampleRate = m_sampleRateReader->getSampleRate();
		if ((timeScale->sampleRate) && (*timeScale->sampleRate.get() != activeSampleRate)) {
			double refactoredDuration = (double) (*scriptDuration->samples.get()) * activeSampleRate / (*timeScale->sampleRate.get());
			duration = round(refactoredDuration);
			drift = refactoredDuration - duration;
		} else{
			duration = *scriptDuration->samples.get();
		}
	} else if (scriptDuration->millis) {
		double refactoredDuration = (double) (*scriptDuration->millis.get()) * m_sampleRateReader->getSampleRate() / 1000;
		duration = round(refactoredDuration);
		drift = refactoredDuration - duration;
	} else if (scriptDuration->beats) {
		if (timeScale->bpm) {
			int bpm = *timeScale->bpm.get();
			int beats = *scriptDuration->beats.get();
			if (scriptDuration->bars) {
				if (timeScale->bpb) {
					beats += ((*scriptDuration->bars.get()) * (*timeScale->bpb.get()));
				} else {
					ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Duration_BarsButNoBpb, "The segment duration uses bars, but no bpb (beats per bar) is specified on the timeline.");
					return shared_ptr<DurationProcessor>();
				}
			}

			double refactoredDuration = m_sampleRateReader->getSampleRate() * beats * 60 / bpm;
			duration = round(refactoredDuration);
			drift = refactoredDuration - duration;
		} else {
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Duration_BeatsButNoBmp, "The segment duration uses beats, but no bpm (beats per minute) is specified on the timeline.");
			return shared_ptr<DurationProcessor>();
		}
	}

	return shared_ptr<DurationProcessor>(new DurationProcessor(duration, drift));
}

shared_ptr<ActionProcessor> ProcessorScriptParser::parseAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, vector<string> location) {
	// Check if it's a ref segment block object or a full one
	if (scriptAction->ref.length() == 0) {
		if (scriptAction->setValue) {
			return parseSetValueAction(context, scriptAction, location);
		} else if (scriptAction->setPolyphony) {
			return parseSetPolyphonyAction(context, scriptAction, location);
		} else if (scriptAction->trigger.length() > 0) {
			return parseTriggerAction(context, scriptAction, location);
		}
		return shared_ptr<ActionProcessor>();
	} else {
		int count = 0;
		for (vector<ScriptAction>::iterator it = context->script->actions.begin(); it != context->script->actions.end(); it++) {
			if (scriptAction->ref.compare(it->id)) {
				vector<string> refLocation = { "script",  "actions", to_string(count) };
				return parseAction(context, &(*it), refLocation);
			}
			count++;
		}

		// Couldn't find the referenced action...
		ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced action with id '", scriptAction->ref.c_str(), "' in the script actions.");
		return shared_ptr<ActionProcessor>();
	}
}

shared_ptr<ActionGlideProcessor> ProcessorScriptParser::parseGlideAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, vector<string> location) {
	return shared_ptr<ActionGlideProcessor>();
}

shared_ptr<ActionProcessor> ProcessorScriptParser::parseSetValueAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, vector<string> location) {
	location.push_back("value");
	shared_ptr<ValueProcessor> valueProcessor = parseValue(context, &scriptAction->setValue.get()->value, location, vector<string>());
	location.pop_back();
	
	location.push_back("output");
	pair<int, int> output = parseOutput(context, &scriptAction->setValue.get()->output, location);
	location.pop_back();

	return shared_ptr<ActionSetValueProcessor>(new ActionSetValueProcessor(valueProcessor, output.first, output.second));
}

shared_ptr<ActionProcessor> ProcessorScriptParser::parseSetPolyphonyAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, vector<string> location) {	
	ScriptSetPolyphony* scriptSetPolyphony = scriptAction->setPolyphony.get();
	return shared_ptr<ActionProcessor>(new ActionSetPolyphonyProcessor(scriptSetPolyphony->index, scriptSetPolyphony->channels));
}

shared_ptr<ActionProcessor> ProcessorScriptParser::parseTriggerAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, vector<string> location) {
	return shared_ptr<ActionProcessor>(new ActionTriggerProcessor(scriptAction->trigger));
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, vector<string> location, vector<string> valueStack) {
	// Check if it's a ref segment block object or a full one
	if (scriptValue->ref.length() == 0) {
		int count = 0;
		location.push_back("calc");
		vector<shared_ptr<CalcProcessor>> calcProcessors;
		for (vector<ScriptCalc>::iterator it = scriptValue->calc.begin(); it != scriptValue->calc.end(); it++) {
			location.push_back(to_string(count));
			calcProcessors.push_back(parseCalc(context, &(*it), location, valueStack));
			location.pop_back();
			count++;
		}
		location.pop_back();
	} else {
		if (find(valueStack.begin(), valueStack.end(), scriptValue->ref) == valueStack.end()) {
			int count = 0;
			for (vector<ScriptValue>::iterator it = context->script->values.begin(); it != context->script->values.end(); it++) {
				if (scriptValue->ref.compare(it->id)) {
					vector<string> refLocation = { "script",  "values", to_string(count) };
					valueStack.push_back(scriptValue->ref);
					return parseValue(context, &(*it), refLocation, valueStack);
					valueStack.pop_back();
				}
				count++;
			}

			// Couldn't find the referenced action...
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced value with id '", scriptValue->ref.c_str(), "' in the script values.");
		} else {
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_CircularFound, "Encountered a circular value reference while processing the value with the id '", scriptValue->ref.c_str(), "'. Circular references can not be resolved.");
		}
	}
	
	return shared_ptr<ValueProcessor>();
}

// Take the letter of the note in lowercase, subtract 'a' from it, take the value in this array at that position
// => the result can be multiplied with 1/12 to get the matchin 1V/oct value.
const int note_to_index [] = { 9, 11, 0, 2, 4, 5, 7};

shared_ptr<ValueProcessor> ProcessorScriptParser::parseStaticValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors, vector<string> location) {
	float value = 0.f;
	if (scriptValue->voltage) {
		value = *scriptValue->voltage.get();
	} else if (scriptValue->note) {
		string note = *scriptValue->note.get();
		char x = tolower(note[0]);
		value = note[1] - 4 + (note_to_index[(x - 'a')] / 12);
		if (note.length() > 2) {
			if (note[2] == '-') {
				value -= (1 / 24);
			} else if (note[2] == '+') {
				value += (1 / 24);
			}
		}
	}
	return shared_ptr<ValueProcessor>(new StaticValueProcessor(value, calcProcessors));
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseInputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors, vector<string> location) {
	location.push_back("input");
	pair<int, int> input = parseInput(context, scriptValue->input.get(), location);
	location.pop_back();

	return shared_ptr<ValueProcessor>(new InputValueProcessor(input.first, input.second, calcProcessors));
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseOutputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors, vector<string> location) {
	location.push_back("output");
	pair<int, int> output = parseOutput(context, scriptValue->output.get(), location);
	location.pop_back();

	return shared_ptr<ValueProcessor>(new OutputValueProcessor(output.first, output.second, calcProcessors));
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseRandValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors, vector<string> location, vector<string> valueStack) {
	location.push_back("lower");
	shared_ptr<ValueProcessor> lowerValueProcessor = parseValue(context, scriptValue->rand.get()->lower.get(), location, valueStack);
	location.pop_back();

	location.push_back("upper");
	shared_ptr<ValueProcessor> upperValueProcessor = parseValue(context, scriptValue->rand.get()->upper.get(), location, valueStack);
	location.pop_back();

	return shared_ptr<ValueProcessor>(new RandValueProcessor(lowerValueProcessor, upperValueProcessor, calcProcessors));
}

shared_ptr<CalcProcessor> ProcessorScriptParser::parseCalc(ProcessorScriptParseContext* context, ScriptCalc* scriptCalc, vector<string> location, vector<string> valueStack) {
	if (scriptCalc->operation == ScriptCalc::CalcOperation::ADD) {
		location.push_back("add");
	} else if (scriptCalc->operation == ScriptCalc::CalcOperation::SUB) {
		location.push_back("sub");
	} else if (scriptCalc->operation == ScriptCalc::CalcOperation::DIV) {
		location.push_back("div");
	} else if (scriptCalc->operation == ScriptCalc::CalcOperation::MULT) {
		location.push_back("mult");
	} else {
		location.push_back(" ");
	}

	shared_ptr<ValueProcessor> valueProcessor = parseValue(context, scriptCalc->value.get(), location, valueStack);

	location.pop_back();

	return shared_ptr<CalcProcessor>(new CalcProcessor(scriptCalc, valueProcessor));
}

pair<int, int> ProcessorScriptParser::parseInput(ProcessorScriptParseContext* context, ScriptInput* scriptInput, vector<string> location) {
	// Check if it's a ref input or a full one
	if (scriptInput->ref.length() == 0) {
		return pair<int, int>(scriptInput->index, scriptInput->channel ? *scriptInput->channel.get() : 1);
	} else {
		int count = 0;
		for (vector<ScriptInput>::iterator it = context->script->inputs.begin(); it != context->script->inputs.end(); it++) {
			if (scriptInput->ref.compare(it->id)) {
				vector<string> refLocation = { "script",  "inputs", to_string(count) };
				return parseInput(context, &(*it), refLocation);
			}
			count++;
		}

		// Couldn't find the referenced action...
		ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced input with id '", scriptInput->ref.c_str(), "' in the script inputs.");
		return pair<int, int>(-1, -1);
	}

}

pair<int, int> ProcessorScriptParser::parseOutput(ProcessorScriptParseContext* context, ScriptOutput* scriptOutput, vector<string> location) {
	// Check if it's a ref output or a full one
	if (scriptOutput->ref.length() == 0) {
		return pair<int, int>(scriptOutput->index, scriptOutput->channel ? *scriptOutput->channel.get() : 1);
	} else {
		int count = 0;
		for (vector<ScriptOutput>::iterator it = context->script->outputs.begin(); it != context->script->outputs.end(); it++) {
			if (scriptOutput->ref.compare(it->id)) {
				vector<string> refLocation = { "script",  "outputs", to_string(count) };
				return parseOutput(context, &(*it), refLocation);
			}
			count++;
		}

		// Couldn't find the referenced action...
		ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced output with id '", scriptOutput->ref.c_str(), "' in the script outputs.");
		return pair<int, int>(-1, -1);
	}

}