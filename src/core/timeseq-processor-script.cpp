#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"
#include <sstream>
#include <stdarg.h>
#include <cctype>

using namespace std;
using namespace timeseq;


inline uint64_t uint64_max(uint64_t a, uint64_t b) {
	return a > b ? a : b;
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

shared_ptr<Processor> ProcessorScriptParser::parseScript(Script* script, vector<ValidationError> *validationErrors, vector<string> location) {
	ProcessorScriptParseContext context;

	context.script = script;
	if (validationErrors != nullptr) {
		context.validationErrors = validationErrors;
	} else {
		context.validationErrors = new vector<ValidationError>();
	}

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

	count = 0;
	location.push_back("global-actions");
	vector<shared_ptr<ActionProcessor>> startActionProcessors;
	for (vector<ScriptAction>::iterator it = script->globalActions.begin(); it != script->globalActions.end(); it++) {
		location.push_back(to_string(count));

		vector<string> actionLocation;
		ScriptAction& scriptAction = *it;
		ScriptAction* resolvedAction = resolveScriptAction(&context, &scriptAction, location, actionLocation);

		if (resolvedAction) {
			if (resolvedAction->timing == ScriptAction::ActionTiming::START) {
				startActionProcessors.push_back(parseResolvedAction(&context, resolvedAction, location));
			} else {
				ADD_VALIDATION_ERROR(context.validationErrors, location, ValidationErrorCode::Script_GlobalActionTiming, "'global-actions' actions can only have a 'start' timing.");
			}
		} else {
			ADD_VALIDATION_ERROR(context.validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced action with id '", scriptAction.ref.c_str(), "' in the script actions.");
		}
		location.pop_back();
		count++;
	}
	location.pop_back();

	return shared_ptr<Processor>(new Processor(timelineProcessors, triggerProcessors, startActionProcessors));
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

	return shared_ptr<TimelineProcessor>(new TimelineProcessor(scriptTimeline, laneProcessors, startTriggers, stopTriggers, m_triggerHandler));
}

shared_ptr<TriggerProcessor> ProcessorScriptParser::parseInputTrigger(ProcessorScriptParseContext* context, ScriptInputTrigger* scriptInputTrigger, vector<string> location) {
	// Check if it's a ref input trigger object or a full one
	if (scriptInputTrigger->input.ref.length() == 0) {
		return shared_ptr<TriggerProcessor>(new TriggerProcessor(scriptInputTrigger->id, scriptInputTrigger->input.index - 1, ((bool) scriptInputTrigger->input.channel) ? *scriptInputTrigger->input.channel.get() - 1 : 0, m_portHandler, m_triggerHandler));
	} else {
		for (vector<ScriptInput>::iterator it = context->script->inputs.begin(); it != context->script->inputs.end(); it++) {
			if (scriptInputTrigger->input.ref.compare(it->id) == 0) {
				return shared_ptr<TriggerProcessor>(new TriggerProcessor(scriptInputTrigger->id, it->index - 1, ((bool) it->channel) ? *it->channel.get() - 1 : 0, m_portHandler, m_triggerHandler));
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
	unsigned int validationCount = context->validationErrors->size();

	location.push_back("segments");
	vector<shared_ptr<SegmentProcessor>> segmentProcessors = parseSegments(context, &scriptLane->segments, timeScale, location, vector<string>());
	location.pop_back();

	// Only return an actual processor if there were no validation errors during parsing. Otherwise there might be partially loaded children, and we can't reliably continue with this processor.
	if (validationCount == context->validationErrors->size()) {
		return shared_ptr<LaneProcessor>(new LaneProcessor(scriptLane, segmentProcessors, m_eventListener));
	} else {
		return shared_ptr<LaneProcessor>();
	}
}

vector<shared_ptr<SegmentProcessor>> ProcessorScriptParser::parseSegments(ProcessorScriptParseContext* context, vector<ScriptSegment>* scriptSegments, ScriptTimeScale* timeScale, vector<string> location, vector<string> segmentStack) {
	int count = 0;
	vector<shared_ptr<SegmentProcessor>> segmentProcessors;

	for (vector<ScriptSegment>::iterator it = scriptSegments->begin(); it != scriptSegments->end(); it++) {
		location.push_back(to_string(count));
		vector<shared_ptr<SegmentProcessor>> segmentProcessorsSubset = parseSegment(context, &(*it), timeScale, location, segmentStack);
		segmentProcessors.insert(segmentProcessors.end(), segmentProcessorsSubset.begin(), segmentProcessorsSubset.end());
		location.pop_back();
		count++;
	}

	return segmentProcessors;
}

vector<shared_ptr<SegmentProcessor>> ProcessorScriptParser::parseSegment(ProcessorScriptParseContext* context, ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, vector<string> location, vector<string> segmentStack) {
	// Check if it's a ref segment object or a full one
	if (scriptSegment->ref.length() == 0) {
		if (!scriptSegment->segmentBlock) {
			// It's an inline non-segment-block segment
			return { parseResolvedSegment(context, scriptSegment, timeScale, location, segmentStack) };
		} else {
			// It's a segment-block segment
			vector<shared_ptr<SegmentProcessor>> blockSegments;
			vector<string> actionsLocation = location;
			location.push_back("segment-block");
			blockSegments = parseSegmentBlock(context, scriptSegment->segmentBlock.get(), timeScale, scriptSegment->actions, location, actionsLocation, segmentStack);
			location.pop_back();
			return blockSegments;
		}
	} else {
		if (find(segmentStack.begin(), segmentStack.end(), string("s-") + scriptSegment->ref) == segmentStack.end()) {
			int count = 0;
			for (vector<ScriptSegment>::iterator it = context->script->segments.begin(); it != context->script->segments.end(); it++) {
				if (scriptSegment->ref.compare(it->id) == 0) {
					vector<shared_ptr<SegmentProcessor>> segments;
					vector<string> refLocation = { "component-pool",  "segments", to_string(count) };
					segmentStack.push_back(string("s-") + scriptSegment->ref);
					segments = parseSegment(context, &(*it), timeScale, refLocation, segmentStack);
					segmentStack.pop_back();
					return segments;
				}
				count++;
			}

			// Couldn't find the referenced segment...
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced segment with id '", scriptSegment->ref.c_str(), "' in the script segments.");
		} else {
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_CircularFound, "Encountered a circular value reference while processing the segment with the id '", scriptSegment->ref.c_str(), "'. Circular references can not be resolved.");
		}
		return {};
	}
}

shared_ptr<SegmentProcessor> ProcessorScriptParser::parseResolvedSegment(ProcessorScriptParseContext* context, ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, vector<string> location, vector<string> segmentStack) {
	location.push_back("duration");
	shared_ptr<DurationProcessor> durationProcessor = parseDuration(context, &scriptSegment->duration, timeScale, location);
	location.pop_back();

	int count = 0;
	vector<shared_ptr<ActionProcessor>> startActions;
	vector<shared_ptr<ActionProcessor>> endActions;
	vector<shared_ptr<ActionOngoingProcessor>> ongoingActions;
	location.push_back("actions");
	for (vector<ScriptAction>::iterator it = scriptSegment->actions.begin(); it != scriptSegment->actions.end(); it++) {
		location.push_back(to_string(count));

		vector<string> actionLocation;
		ScriptAction& scriptAction = *it;
		ScriptAction* resolvedAction = resolveScriptAction(context, &scriptAction, location, actionLocation);

		if (resolvedAction) {
			if (resolvedAction->timing == ScriptAction::ActionTiming::START) {
				startActions.push_back(parseResolvedAction(context, resolvedAction, actionLocation));
			} else if (resolvedAction->timing == ScriptAction::ActionTiming::END) {
				endActions.push_back(parseResolvedAction(context, resolvedAction, actionLocation));
			} else if (resolvedAction->timing == ScriptAction::ActionTiming::GLIDE) {
				ongoingActions.push_back(parseResolvedGlideAction(context, resolvedAction, actionLocation));
			} else if (resolvedAction->timing == ScriptAction::ActionTiming::GATE) {
				ongoingActions.push_back(parseResolvedGateAction(context, resolvedAction, actionLocation));
			}
		} else {
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced action with id '", scriptAction.ref.c_str(), "' in the script actions.");
		}

		location.pop_back();
		count++;
	}
	location.pop_back();

	return shared_ptr<SegmentProcessor>(new SegmentProcessor(scriptSegment, durationProcessor, startActions, endActions, ongoingActions, m_eventListener));
}

vector<shared_ptr<SegmentProcessor>> ProcessorScriptParser::parseSegmentBlock(ProcessorScriptParseContext* context, ScriptSegmentBlock* scriptSegmentBlock, ScriptTimeScale* timeScale, std::vector<ScriptAction>& actions, vector<string> location, vector<string> actionsLocation, vector<string> segmentStack) {
	// Check if it's a ref segment block object or a full one
	if (scriptSegmentBlock->ref.length() == 0) {
		location.push_back("segments");

		// Build the full list of segments for the segment block
		vector<shared_ptr<SegmentProcessor>> blockSegmentProcessors = parseSegments(context, &scriptSegmentBlock->segments, timeScale, location, segmentStack);
		location.pop_back();

		vector<shared_ptr<SegmentProcessor>> segmentProcessors;
		if (!scriptSegmentBlock->repeat) {
			segmentProcessors = blockSegmentProcessors;
		} else {
			for (int i = 0; i < *scriptSegmentBlock->repeat.get(); i++) {
				segmentProcessors.insert(segmentProcessors.end(), blockSegmentProcessors.begin(), blockSegmentProcessors.end());
			}
		}

		if ((segmentProcessors.size() > 0) && (context->validationErrors->size() == 0)) {
			// Build the lists of start and end actions defined on the segment block.
			vector<shared_ptr<ActionProcessor>> startActions;
			vector<shared_ptr<ActionProcessor>> endActions;
			int count = 0;

			actionsLocation.push_back("actions");
			for (std::vector<ScriptAction>::iterator it = actions.begin(); it != actions.end(); it++) {
				actionsLocation.push_back(to_string(count));

				vector<string> actionLocation;
				ScriptAction& scriptAction = *it;
				ScriptAction* resolvedAction = resolveScriptAction(context, &scriptAction, actionsLocation, actionLocation);

				if (resolvedAction) {
					if (resolvedAction->timing == ScriptAction::ActionTiming::START) {
						startActions.push_back(parseResolvedAction(context, resolvedAction, actionLocation));
					} else if (resolvedAction->timing == ScriptAction::ActionTiming::END) {
						endActions.push_back(parseResolvedAction(context, resolvedAction, actionLocation));
					} else {
						ADD_VALIDATION_ERROR(context->validationErrors, actionsLocation, ValidationErrorCode::Segment_SegmentBlockActionTimings, "The 'timing' of actions on a segment with a 'segment-block' reference can only be 'start' or 'end'.");
					}
				} else {
					ADD_VALIDATION_ERROR(context->validationErrors, actionsLocation, ValidationErrorCode::Ref_NotFound, "Could not find the referenced action with id '", scriptAction.ref.c_str(), "' in the script actions.");
				}

				actionsLocation.pop_back();
				count++;
			}
			actionsLocation.pop_back();

			// If there are start or end actions, they will have to be added to the first and/or the last segments of the block (but only if there are no current validation errors, since we assume a correctly loaded segment here)
			if ((context->validationErrors->size() == 0) && ((startActions.size() > 0) || (endActions.size() > 0))) {
				if ((!scriptSegmentBlock->repeat) || ((*scriptSegmentBlock->repeat.get()) < 2)) {
					// If the segment block is not repeating, we can assign the additional actions directly on the relevant segments
					segmentProcessors.front()->pushStartActions(startActions);
					segmentProcessors.back()->pushEndActions(endActions);
				} else {
					// Otherwise, we'll have to create new instances of the first and last segments, since the same instance is repeated multiple times in the list, and we only want to update the first and last ones
					*segmentProcessors.begin() = shared_ptr<SegmentProcessor>(new SegmentProcessor(*segmentProcessors.front()));
					segmentProcessors.front()->pushStartActions(startActions);
					*(segmentProcessors.end() - 1) = shared_ptr<SegmentProcessor>(new SegmentProcessor(*segmentProcessors.back()));
					segmentProcessors.back()->pushEndActions(endActions);
				}
			}
		}

		return segmentProcessors;
	} else {
		if (find(segmentStack.begin(), segmentStack.end(), string("sb-") + scriptSegmentBlock->ref) == segmentStack.end()) {
			int count = 0;
			for (vector<ScriptSegmentBlock>::iterator it = context->script->segmentBlocks.begin(); it != context->script->segmentBlocks.end(); it++) {
				if (scriptSegmentBlock->ref.compare(it->id) == 0) {
					vector<string> refLocation = { "component-pool",  "segment-blocks", to_string(count) };
					segmentStack.push_back(string("sb-") + scriptSegmentBlock->ref);
					vector<shared_ptr<SegmentProcessor>> segments = parseSegmentBlock(context, &(*it), timeScale, actions, refLocation, actionsLocation, segmentStack);
					segmentStack.pop_back();
					return segments;
				}
				count++;
			}

			// Couldn't find the referenced segment-block...
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced segment-block with id '", scriptSegmentBlock->ref.c_str(), "' in the script segment-blocks.");
		} else {
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_CircularFound, "Encountered a circular value reference while processing the segment-block with the id '", scriptSegmentBlock->ref.c_str(), "'. Circular references can not be resolved.");
		}
		return vector<shared_ptr<SegmentProcessor>>();
	}
}

shared_ptr<DurationProcessor> ProcessorScriptParser::parseDuration(ProcessorScriptParseContext* context, ScriptDuration* scriptDuration, ScriptTimeScale* timeScale, vector<string> location) {
	uint64_t duration = 0;
	double drift = 0;

	if (scriptDuration->samples) {
		float activeSampleRate = m_sampleRateReader->getSampleRate();
		if ((timeScale) && (timeScale->sampleRate) && (*timeScale->sampleRate.get() != activeSampleRate)) {
			double refactoredDuration = (double) (*scriptDuration->samples.get()) * activeSampleRate / (*timeScale->sampleRate.get());
			duration = uint64_max(floor(refactoredDuration), 1);
			drift = refactoredDuration - duration;
		} else{
			duration = *scriptDuration->samples.get();
		}
	} else if (scriptDuration->millis) {
		double refactoredDuration = (double) (*scriptDuration->millis.get()) * m_sampleRateReader->getSampleRate() / 1000;
		duration = uint64_max(floor(refactoredDuration), 1);
		drift = refactoredDuration - duration;
	} else if (scriptDuration->beats) {
		if ((timeScale) && (timeScale->bpm)) {
			int bpm = *timeScale->bpm.get();
			double beats = *scriptDuration->beats.get();
			if (scriptDuration->bars) {
				if (timeScale->bpb) {
					beats += ((*scriptDuration->bars.get()) * (*timeScale->bpb.get()));
				} else {
					ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Duration_BarsButNoBpb, "The segment duration uses bars, but no bpb (beats per bar) is specified on the timeline.");
					return shared_ptr<DurationProcessor>();
				}
			}

			double refactoredDuration = m_sampleRateReader->getSampleRate() * beats * 60 / bpm;
			duration = uint64_max(floor(refactoredDuration), 1);
			drift = refactoredDuration - duration;
		} else {
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Duration_BeatsButNoBmp, "The segment duration uses beats, but no bpm (beats per minute) is specified on the timeline.");
			return shared_ptr<DurationProcessor>();
		}
	} else if (scriptDuration->hz) {
		double refactoredDuration = (double) m_sampleRateReader->getSampleRate() / (*scriptDuration->hz.get());
		duration = uint64_max(floor(refactoredDuration), 1);
		drift = refactoredDuration - duration;
	}

	return shared_ptr<DurationProcessor>(new DurationProcessor(duration, drift));
}

shared_ptr<ActionProcessor> ProcessorScriptParser::parseResolvedAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, vector<string> location) {
	shared_ptr<ActionProcessor> actionProcessor;
	shared_ptr<IfProcessor> ifProcessor;

	if (scriptAction->condition) {
		location.push_back("if");
		ifProcessor = parseIf(context, scriptAction->condition.get(), location, vector<string>());
		location.pop_back();
	}

	if (scriptAction->setValue) {
		location.push_back("set-value");
		actionProcessor = parseSetValueAction(context, scriptAction, ifProcessor, location);
		location.pop_back();
	} else if (scriptAction->setVariable) {
		location.push_back("set-variable");
		actionProcessor = parseSetVariableAction(context, scriptAction, ifProcessor, location);
		location.pop_back();
	} else if (scriptAction->setPolyphony) {
		location.push_back("set-polyphony");
		actionProcessor = parseSetPolyphonyAction(context, scriptAction, ifProcessor, location);
		location.pop_back();
	} else if (scriptAction->assert) {
		location.push_back("assert");
		actionProcessor = parseAssertAction(context, scriptAction, ifProcessor, location);
		location.pop_back();
	} else if (scriptAction->trigger.length() > 0) {
		location.push_back("trigger");
		actionProcessor = parseTriggerAction(context, scriptAction, ifProcessor, location);
		location.pop_back();
	} else {
		return actionProcessor;
	}

	return actionProcessor;
}

shared_ptr<ActionGlideProcessor> ProcessorScriptParser::parseResolvedGlideAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, vector<string> location) {
	shared_ptr<IfProcessor> ifProcessor;

	if (scriptAction->condition) {
		location.push_back("if");
		ifProcessor = parseIf(context, scriptAction->condition.get(), location, vector<string>());
		location.pop_back();
	}

	location.push_back("start-value");
	shared_ptr<ValueProcessor> startValueProcessor = parseValue(context, &(*scriptAction->startValue.get()), location, vector<string>());
	location.pop_back();
	location.push_back("end-value");
	shared_ptr<ValueProcessor> endValueProcessor = parseValue(context, &(*scriptAction->endValue.get()), location, vector<string>());
	location.pop_back();

	int outputPort = -1;
	int outputChannel = -1;
	if (scriptAction->output) {
		location.push_back("output");
		pair<int, int> output = parseOutput(context, &(*scriptAction->output), location);
		location.pop_back();
		outputPort = output.first;
		outputChannel = output.second;
	}

	float easeFactor = 0.f;
	bool easePow = false;
	if (scriptAction->easeFactor) {
		easeFactor = *scriptAction->easeFactor.get();
	}
	if (scriptAction->easeAlgorithm) {
		if (*scriptAction->easeAlgorithm == ScriptAction::EaseAlgorithm::POW) {
			easePow = true;
		}
	}

	return shared_ptr<ActionGlideProcessor>(new ActionGlideProcessor(easeFactor, easePow, startValueProcessor, endValueProcessor, ifProcessor, outputPort, outputChannel, scriptAction->variable, m_portHandler, m_variableHandler));
}

shared_ptr<ActionGateProcessor> ProcessorScriptParser::parseResolvedGateAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, vector<string> location) {
	shared_ptr<IfProcessor> ifProcessor;

	if (scriptAction->condition) {
		location.push_back("if");
		ifProcessor = parseIf(context, scriptAction->condition.get(), location, vector<string>());
		location.pop_back();
	}

	int outputPort = -1;
	int outputChannel = -1;
	if (scriptAction->output) {
		location.push_back("output");
		pair<int, int> output = parseOutput(context, &(*scriptAction->output), location);
		location.pop_back();
		outputPort = output.first;
		outputChannel = output.second;
	}

	float gateHighRatio = 0.5f;
	if (scriptAction->gateHighRatio) {
		gateHighRatio = *scriptAction->gateHighRatio.get();
	}

	return shared_ptr<ActionGateProcessor>(new ActionGateProcessor(gateHighRatio, ifProcessor, outputPort, outputChannel, m_portHandler));
}

shared_ptr<ActionProcessor> ProcessorScriptParser::parseSetValueAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, shared_ptr<IfProcessor> ifProcessor, vector<string> location) {
	location.push_back("value");
	shared_ptr<ValueProcessor> valueProcessor = parseValue(context, &scriptAction->setValue.get()->value, location, vector<string>());
	location.pop_back();

	location.push_back("output");
	pair<int, int> output = parseOutput(context, &scriptAction->setValue.get()->output, location);
	location.pop_back();

	return shared_ptr<ActionSetValueProcessor>(new ActionSetValueProcessor(valueProcessor, output.first, output.second, m_portHandler, ifProcessor));
}

shared_ptr<ActionProcessor> ProcessorScriptParser::parseSetVariableAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, shared_ptr<IfProcessor> ifProcessor, vector<string> location) {
	location.push_back("value");
	shared_ptr<ValueProcessor> valueProcessor = parseValue(context, &scriptAction->setVariable.get()->value, location, vector<string>());
	location.pop_back();

	return shared_ptr<ActionSetVariableProcessor>(new ActionSetVariableProcessor(valueProcessor, scriptAction->setVariable.get()->name, m_variableHandler, ifProcessor));
}

shared_ptr<ActionProcessor> ProcessorScriptParser::parseSetPolyphonyAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, shared_ptr<IfProcessor> ifProcessor, vector<string> location) {
	ScriptSetPolyphony* scriptSetPolyphony = scriptAction->setPolyphony.get();
	return shared_ptr<ActionProcessor>(new ActionSetPolyphonyProcessor(scriptSetPolyphony->index - 1, scriptSetPolyphony->channels, m_portHandler, ifProcessor));
}

std::shared_ptr<ActionProcessor> ProcessorScriptParser::parseAssertAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, std::shared_ptr<IfProcessor> ifProcessor, std::vector<std::string> location) {
	ScriptAssert* scriptAssert = scriptAction->assert.get();

	location.push_back("expect");
	shared_ptr<IfProcessor> expect = parseIf(context, &scriptAssert->expect, location, vector<string>());
	location.pop_back();

	return shared_ptr<ActionProcessor>(new ActionAssertProcessor(scriptAssert->name, expect, scriptAssert->stopOnFail, m_assertListener, ifProcessor));
}

shared_ptr<ActionProcessor> ProcessorScriptParser::parseTriggerAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, shared_ptr<IfProcessor> ifProcessor, vector<string> location) {
	return shared_ptr<ActionProcessor>(new ActionTriggerProcessor(scriptAction->trigger, m_triggerHandler, ifProcessor));
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

		if ((scriptValue->voltage) || (scriptValue->note)) {
			return parseStaticValue(context, scriptValue, calcProcessors, location);
		} else if (scriptValue->variable) {
			return parseVariableValue(context, scriptValue, calcProcessors, location);
		} else if (scriptValue->input) {
			return parseInputValue(context, scriptValue, calcProcessors, location);
		} else if (scriptValue->output) {
			return parseOutputValue(context, scriptValue, calcProcessors, location);
		} else if (scriptValue->rand) {
			return parseRandValue(context, scriptValue, calcProcessors, location, valueStack);
		}

	} else {
		if (find(valueStack.begin(), valueStack.end(), string("v-") + scriptValue->ref) == valueStack.end()) {
			int count = 0;
			for (vector<ScriptValue>::iterator it = context->script->values.begin(); it != context->script->values.end(); it++) {
				if (scriptValue->ref.compare(it->id) == 0) {
					vector<string> refLocation = { "component-pool",  "values", to_string(count) };
					valueStack.push_back(string("v-") + scriptValue->ref);
					return parseValue(context, &(*it), refLocation, valueStack);
					valueStack.pop_back();
				}
				count++;
			}

			// Couldn't find the referenced value...
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
		int noteIndex = note_to_index[(x - 'a')];
		if (note.length() > 2) {
			if (note[2] == '-') {
				noteIndex -= 1;
			} else if (note[2] == '+') {
				noteIndex += 1;
			}
		}
		value = note[1] - '0' - 4 + ((float) noteIndex / 12);
	}
	return shared_ptr<ValueProcessor>(new StaticValueProcessor(value, calcProcessors, scriptValue->quantize));
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseVariableValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors, vector<string> location) {
	return shared_ptr<ValueProcessor>(new VariableValueProcessor(*scriptValue->variable.get(), calcProcessors, scriptValue->quantize, m_variableHandler));
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseInputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors, vector<string> location) {
	location.push_back("input");
	pair<int, int> input = parseInput(context, scriptValue->input.get(), location);
	location.pop_back();

	return shared_ptr<ValueProcessor>(new InputValueProcessor(input.first, input.second, calcProcessors, scriptValue->quantize, m_portHandler));
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseOutputValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors, vector<string> location) {
	location.push_back("output");
	pair<int, int> output = parseOutput(context, scriptValue->output.get(), location);
	location.pop_back();

	return shared_ptr<ValueProcessor>(new OutputValueProcessor(output.first, output.second, calcProcessors, scriptValue->quantize, m_portHandler));
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseRandValue(ProcessorScriptParseContext* context, ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors, vector<string> location, vector<string> valueStack) {
	location.push_back("rand");

	location.push_back("lower");
	shared_ptr<ValueProcessor> lowerValueProcessor = parseValue(context, scriptValue->rand.get()->lower.get(), location, valueStack);
	location.pop_back();

	location.push_back("upper");
	shared_ptr<ValueProcessor> upperValueProcessor = parseValue(context, scriptValue->rand.get()->upper.get(), location, valueStack);
	location.pop_back();

	location.pop_back();

	return shared_ptr<ValueProcessor>(new RandValueProcessor(lowerValueProcessor, upperValueProcessor, m_randomValueGenerator, calcProcessors, scriptValue->quantize));
}

shared_ptr<CalcProcessor> ProcessorScriptParser::parseCalc(ProcessorScriptParseContext* context, ScriptCalc* scriptCalc, vector<string> location, vector<string> valueStack) {
	if (scriptCalc->ref.length() == 0) {
		switch (scriptCalc->operation) {
			case ScriptCalc::CalcOperation::ADD:
				location.push_back("add");
				break;
			case ScriptCalc::CalcOperation::SUB:
				location.push_back("sub");
				break;
			case ScriptCalc::CalcOperation::DIV:
				location.push_back("div");
				break;
			case ScriptCalc::CalcOperation::MULT:
				location.push_back("mult");
				break;
		}

		shared_ptr<ValueProcessor> valueProcessor = parseValue(context, scriptCalc->value.get(), location, valueStack);

		location.pop_back();

		return shared_ptr<CalcProcessor>(new CalcProcessor(scriptCalc, valueProcessor));
	} else {
		if (find(valueStack.begin(), valueStack.end(), string("c-") + scriptCalc->ref) == valueStack.end()) {
			int count = 0;
			for (vector<ScriptCalc>::iterator it = context->script->calcs.begin(); it != context->script->calcs.end(); it++) {
				if (scriptCalc->ref.compare(it->id) == 0) {
					vector<string> refLocation = { "component-pool",  "calcs", to_string(count) };
					valueStack.push_back(string("c-") + scriptCalc->ref);
					return parseCalc(context, &(*it), refLocation, valueStack);
					valueStack.pop_back();
				}
				count++;
			}

			// Couldn't find the referenced calc...
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced calc with id '", scriptCalc->ref.c_str(), "' in the script calcs.");
		} else {
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_CircularFound, "Encountered a circular value reference while processing the calc with the id '", scriptCalc->ref.c_str(), "'. Circular references can not be resolved.");
		}
	}

	return shared_ptr<CalcProcessor>();
}

shared_ptr<IfProcessor> ProcessorScriptParser::parseIf(ProcessorScriptParseContext* context, ScriptIf* scriptIf, vector<string> location, vector<string> ifStack) {
	if (scriptIf->ref.length() == 0) {
		pair<shared_ptr<ValueProcessor>, shared_ptr<ValueProcessor>> values;
		pair<shared_ptr<IfProcessor>, shared_ptr<IfProcessor>> ifs;
		bool parseValues = true;
		bool parseIfs = false;

		switch (scriptIf->ifOperator) {
			case ScriptIf::IfOperator::EQ:
				location.push_back("eq");
				break;
			case ScriptIf::IfOperator::NE:
				location.push_back("ne");
				break;
			case ScriptIf::IfOperator::LT:
				location.push_back("lt");
				break;
			case ScriptIf::IfOperator::LTE:
				location.push_back("lte");
				break;
			case ScriptIf::IfOperator::GT:
				location.push_back("gt");
				break;
			case ScriptIf::IfOperator::GTE:
				location.push_back("gte");
				break;
			case ScriptIf::IfOperator::AND:
				parseValues = false;
				parseIfs = true;
				location.push_back("and");
				break;
			case ScriptIf::IfOperator::OR:
				parseValues = false;
				parseIfs = true;
				location.push_back("or");
				break;
		}

		if (parseValues) {
			location.push_back("0");
			values.first = parseValue(context, &scriptIf->values.get()->first, location, vector<string>());
			location.pop_back();
			location.push_back("1");
			values.second = parseValue(context, &scriptIf->values.get()->second, location, vector<string>());
			location.pop_back();
		}
		if (parseIfs) {
			location.push_back("0");
			ifs.first = parseIf(context, &scriptIf->ifs.get()->first, location, ifStack);
			location.pop_back();
			location.push_back("1");
			ifs.second = parseIf(context, &scriptIf->ifs.get()->second, location, ifStack);
			location.pop_back();
		}

		location.pop_back();

		return shared_ptr<IfProcessor>(new IfProcessor(scriptIf, values, ifs));
	} else {
		if (find(ifStack.begin(), ifStack.end(), scriptIf->ref) == ifStack.end()) {
			int count = 0;
			for (vector<ScriptIf>::iterator it = context->script->ifs.begin(); it != context->script->ifs.end(); it++) {
				if (scriptIf->ref.compare(it->id) == 0) {
					vector<string> refLocation = { "component-pool",  "ifs", to_string(count) };
					ifStack.push_back(scriptIf->ref);
					return parseIf(context, &(*it), refLocation, ifStack);
					ifStack.pop_back();
				}
				count++;
			}

			// Couldn't find the referenced if...
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced if with id '", scriptIf->ref.c_str(), "' in the script ifs.");
		} else {
			ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_CircularFound, "Encountered a circular if reference while processing the if with the id '", scriptIf->ref.c_str(), "'. Circular references can not be resolved.");
		}
	}

	return shared_ptr<IfProcessor>();
}

pair<int, int> ProcessorScriptParser::parseInput(ProcessorScriptParseContext* context, ScriptInput* scriptInput, vector<string> location) {
	// Check if it's a ref input or a full one
	if (scriptInput->ref.length() == 0) {
		return pair<int, int>(scriptInput->index - 1, scriptInput->channel ? *scriptInput->channel.get() - 1 : 0);
	} else {
		int count = 0;
		for (vector<ScriptInput>::iterator it = context->script->inputs.begin(); it != context->script->inputs.end(); it++) {
			if (scriptInput->ref.compare(it->id) == 0) {
				vector<string> refLocation = { "component-pool",  "inputs", to_string(count) };
				return parseInput(context, &(*it), refLocation);
			}
			count++;
		}

		// Couldn't find the referenced input...
		ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced input with id '", scriptInput->ref.c_str(), "' in the script inputs.");
		return pair<int, int>(-1, -1);
	}

}

pair<int, int> ProcessorScriptParser::parseOutput(ProcessorScriptParseContext* context, ScriptOutput* scriptOutput, vector<string> location) {
	// Check if it's a ref output or a full one
	if (scriptOutput->ref.length() == 0) {
		return pair<int, int>(scriptOutput->index - 1, scriptOutput->channel ? *scriptOutput->channel.get() - 1 : 0);
	} else {
		int count = 0;
		for (vector<ScriptOutput>::iterator it = context->script->outputs.begin(); it != context->script->outputs.end(); it++) {
			if (scriptOutput->ref.compare(it->id) == 0) {
				vector<string> refLocation = { "component-pool",  "outputs", to_string(count) };
				return parseOutput(context, &(*it), refLocation);
			}
			count++;
		}

		// Couldn't find the referenced output...
		ADD_VALIDATION_ERROR(context->validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced output with id '", scriptOutput->ref.c_str(), "' in the script outputs.");
		return pair<int, int>(-1, -1);
	}
}

ScriptAction* ProcessorScriptParser::resolveScriptAction(ProcessorScriptParseContext* context, ScriptAction* scriptAction, vector<string>& currentLocation, vector<string>& resolvedLocation) {
	if (scriptAction->ref.length() == 0) {
		resolvedLocation = currentLocation;
		return scriptAction;
	} else {
		int count = 0;
		for (vector<ScriptAction>::iterator it = context->script->actions.begin(); it != context->script->actions.end(); it++) {
			if (scriptAction->ref.compare(it->id) == 0) {
				resolvedLocation = { "component-pool", "actions", to_string(count) };
				return &(*it);
			}
			count++;
		}

		return nullptr;
	}

}

ProcessorLoader::ProcessorLoader(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener) : m_processorScriptParser(portHandler, variableHandler, triggerHandler, sampleRateReader, eventListener, assertListener, shared_ptr<RandValueGenerator>(new RandValueGenerator())) {}
ProcessorLoader::ProcessorLoader(PortHandler* portHandler, VariableHandler* variableHandler, TriggerHandler* triggerHandler, SampleRateReader* sampleRateReader, EventListener* eventListener, AssertListener* assertListener, shared_ptr<RandValueGenerator> randomValueGenerator) : m_processorScriptParser(portHandler, variableHandler, triggerHandler, sampleRateReader, eventListener, assertListener, randomValueGenerator) {}

ProcessorLoader::~ProcessorLoader() {
}

shared_ptr<Processor> ProcessorLoader::loadScript(shared_ptr<Script> script, vector<ValidationError> *validationErrors) {
	return m_processorScriptParser.parseScript(script.get(), validationErrors, vector<string>());
}
