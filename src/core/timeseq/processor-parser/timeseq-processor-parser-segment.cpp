#include "core/timeseq-processor-parser.hpp"
#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"

using namespace std;
using namespace timeseq;

inline uint64_t uint64_max(uint64_t a, uint64_t b) {
	return a > b ? a : b;
}

vector<shared_ptr<SegmentProcessor>> ProcessorScriptParser::parseSegments(const vector<ScriptSegment>* scriptSegments, ScriptTimeScale* timeScale, vector<string> segmentStack) {
	int count = 0;
	vector<shared_ptr<SegmentProcessor>> segmentProcessors;

	for (const ScriptSegment& segment : *scriptSegments) {
		m_context.location.push_back(to_string(count));
		vector<shared_ptr<SegmentProcessor>> segmentProcessorsSubset = parseSegment(&segment, timeScale, segmentStack);
		segmentProcessors.insert(segmentProcessors.end(), segmentProcessorsSubset.begin(), segmentProcessorsSubset.end());
		m_context.location.pop_back();
		count++;
	}

	return segmentProcessors;
}

vector<shared_ptr<SegmentProcessor>> ProcessorScriptParser::parseSegment(const ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, vector<string> segmentStack) {
	// Check if it's a ref segment object or a full one
	if (scriptSegment->ref.length() == 0) {
		if (!scriptSegment->segmentBlock) {
			// It's an inline non-segment-block segment
			return { parseResolvedSegment(scriptSegment, timeScale, segmentStack) };
		} else {
			// It's a segment-block segment
			vector<shared_ptr<SegmentProcessor>> blockSegments;
			vector<string> actionsLocation = m_context.location;
			m_context.location.push_back("segment-block");
			blockSegments = parseSegmentBlock(scriptSegment->segmentBlock.get(), timeScale, scriptSegment->actions, actionsLocation, segmentStack);
			m_context.location.pop_back();
			return blockSegments;
		}
	} else {
		if (find(segmentStack.begin(), segmentStack.end(), string("s-") + scriptSegment->ref) == segmentStack.end()) {
			int count = 0;
			for (const ScriptSegment& segment : m_context.script->segments) {
				if (scriptSegment->ref.compare(segment.id) == 0) {
					vector<shared_ptr<SegmentProcessor>> segments;
					m_context.stashLocation();
					m_context.location = { "component-pool",  "segments", to_string(count) };
					segmentStack.push_back(string("s-") + scriptSegment->ref);
					segments = parseSegment(&segment, timeScale, segmentStack);
					segmentStack.pop_back();
					m_context.popLocation();
					return segments;
				}
				count++;
			}

			// Couldn't find the referenced segment...
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced segment with id '", scriptSegment->ref.c_str(), "' in the script segments.");
		} else {
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_CircularFound, "Encountered a circular value reference while processing the segment with the id '", scriptSegment->ref.c_str(), "'. Circular references can not be resolved.");
		}
		return {};
	}
}

shared_ptr<SegmentProcessor> ProcessorScriptParser::parseResolvedSegment(const ScriptSegment* scriptSegment, ScriptTimeScale* timeScale, vector<string> segmentStack) {
	m_context.location.push_back("duration");
	shared_ptr<DurationProcessor> durationProcessor = parseDuration(&scriptSegment->duration, timeScale);
	m_context.location.pop_back();

	int count = 0;
	vector<shared_ptr<ActionProcessor>> startActions;
	vector<shared_ptr<ActionProcessor>> endActions;
	vector<shared_ptr<ActionOngoingProcessor>> ongoingActions;
	m_context.location.push_back("actions");
	for (const ScriptAction& action : scriptSegment->actions) {
		m_context.location.push_back(to_string(count));

		vector<string> actionLocation;
		const ScriptAction* resolvedAction = resolveScriptAction(&action, actionLocation);

		if (resolvedAction) {
			vector<string> location = m_context.location;
			m_context.stashLocation();
			if (resolvedAction->timing == ScriptAction::ActionTiming::START) {
				startActions.push_back(parseResolvedAction(resolvedAction));
			} else if (resolvedAction->timing == ScriptAction::ActionTiming::END) {
				endActions.push_back(parseResolvedAction(resolvedAction));
			} else if (resolvedAction->timing == ScriptAction::ActionTiming::GLIDE) {
				ongoingActions.push_back(parseResolvedGlideAction(resolvedAction));
			} else if (resolvedAction->timing == ScriptAction::ActionTiming::GATE) {
				ongoingActions.push_back(parseResolvedGateAction(resolvedAction));
			}
			m_context.popLocation();
		} else {
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced action with id '", action.ref.c_str(), "' in the script actions.");
		}

		m_context.location.pop_back();
		count++;
	}
	m_context.location.pop_back();

	return make_shared<SegmentProcessor>(scriptSegment, durationProcessor, startActions, endActions, ongoingActions, m_eventListener);
}

vector<shared_ptr<SegmentProcessor>> ProcessorScriptParser::parseSegmentBlock(const ScriptSegmentBlock* scriptSegmentBlock, ScriptTimeScale* timeScale, const vector<ScriptAction>& actions, vector<string> actionsLocation, vector<string> segmentStack) {
	// Check if it's a ref segment block object or a full one
	if (scriptSegmentBlock->ref.length() == 0) {
		m_context.location.push_back("segments");

		// Build the full list of segments for the segment block
		vector<shared_ptr<SegmentProcessor>> blockSegmentProcessors = parseSegments(&scriptSegmentBlock->segments, timeScale, segmentStack);
		m_context.location.pop_back();

		vector<shared_ptr<SegmentProcessor>> segmentProcessors;
		if (!scriptSegmentBlock->repeat) {
			segmentProcessors = blockSegmentProcessors;
		} else {
			for (int i = 0; i < *scriptSegmentBlock->repeat.get(); i++) {
				segmentProcessors.insert(segmentProcessors.end(), blockSegmentProcessors.begin(), blockSegmentProcessors.end());
			}
		}

		if ((segmentProcessors.size() > 0) && (m_context.validationErrors->size() == 0)) {
			// Build the lists of start and end actions defined on the segment block.
			vector<shared_ptr<ActionProcessor>> startActions;
			vector<shared_ptr<ActionProcessor>> endActions;
			int count = 0;

			actionsLocation.push_back("actions");
			for (const ScriptAction& action : actions) {
				actionsLocation.push_back(to_string(count));

				vector<string> actionLocation;
				const ScriptAction* resolvedAction = resolveScriptAction(&action, actionLocation);

				if (resolvedAction) {
					vector<string> location = m_context.location;
					m_context.stashLocation();
					if (resolvedAction->timing == ScriptAction::ActionTiming::START) {
						startActions.push_back(parseResolvedAction(resolvedAction));
					} else if (resolvedAction->timing == ScriptAction::ActionTiming::END) {
						endActions.push_back(parseResolvedAction(resolvedAction));
					} else {
						ADD_VALIDATION_ERROR(m_context.validationErrors, actionsLocation, ValidationErrorCode::Segment_SegmentBlockActionTimings, "The 'timing' of actions on a segment with a 'segment-block' reference can only be 'start' or 'end'.");
					}
					m_context.popLocation();
				} else {
					ADD_VALIDATION_ERROR(m_context.validationErrors, actionsLocation, ValidationErrorCode::Ref_NotFound, "Could not find the referenced action with id '", action.ref.c_str(), "' in the script actions.");
				}

				actionsLocation.pop_back();
				count++;
			}
			actionsLocation.pop_back();

			// If there are start or end actions, they will have to be added to the first and/or the last segments of the block (but only if there are no current validation errors, since we assume a correctly loaded segment here)
			if ((m_context.validationErrors->size() == 0) && ((startActions.size() > 0) || (endActions.size() > 0))) {
				if ((!scriptSegmentBlock->repeat) || ((*scriptSegmentBlock->repeat.get()) < 2)) {
					// If the segment block is not repeating, we can assign the additional actions directly on the relevant segments
					segmentProcessors.front()->pushStartActions(startActions);
					segmentProcessors.back()->pushEndActions(endActions);
				} else {
					// Otherwise, we'll have to create new instances of the first and last segments, since the same instance is repeated multiple times in the list, and we only want to update the first and last ones
					*segmentProcessors.begin() = make_shared<SegmentProcessor>(*segmentProcessors.front());
					segmentProcessors.front()->pushStartActions(startActions);
					*(segmentProcessors.end() - 1) = make_shared<SegmentProcessor>(*segmentProcessors.back());
					segmentProcessors.back()->pushEndActions(endActions);
				}
			}
		}

		return segmentProcessors;
	} else {
		if (find(segmentStack.begin(), segmentStack.end(), string("sb-") + scriptSegmentBlock->ref) == segmentStack.end()) {
			int count = 0;
			for (const ScriptSegmentBlock& segmentBlock : m_context.script->segmentBlocks) {
				if (scriptSegmentBlock->ref.compare(segmentBlock.id) == 0) {
					m_context.stashLocation();
					m_context.location = { "component-pool",  "segment-blocks", to_string(count) };
					segmentStack.push_back(string("sb-") + scriptSegmentBlock->ref);
					vector<shared_ptr<SegmentProcessor>> segments = parseSegmentBlock(&segmentBlock, timeScale, actions, actionsLocation, segmentStack);
					segmentStack.pop_back();
					m_context.popLocation();
					return segments;
				}
				count++;
			}

			// Couldn't find the referenced segment-block...
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced segment-block with id '", scriptSegmentBlock->ref.c_str(), "' in the script segment-blocks.");
		} else {
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_CircularFound, "Encountered a circular value reference while processing the segment-block with the id '", scriptSegmentBlock->ref.c_str(), "'. Circular references can not be resolved.");
		}
		return vector<shared_ptr<SegmentProcessor>>();
	}
}

shared_ptr<DurationProcessor> ProcessorScriptParser::parseDuration(const ScriptDuration* scriptDuration, ScriptTimeScale* timeScale) {
	if (scriptDuration->samples || scriptDuration->millis || scriptDuration->beats || scriptDuration->hz) {
		// Construct a constant duration processor
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
						ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_BarsButNoBpb, "The segment duration uses bars, but no bpb (beats per bar) is specified on the timeline.");
						return shared_ptr<DurationProcessor>();
					}
				}

				double refactoredDuration = m_sampleRateReader->getSampleRate() * beats * 60 / bpm;
				duration = uint64_max(floor(refactoredDuration), 1);
				drift = refactoredDuration - duration;
			} else {
				ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_BeatsButNoBmp, "The segment duration uses beats, but no bpm (beats per minute) is specified on the timeline.");
				return shared_ptr<DurationProcessor>();
			}
		} else if (scriptDuration->hz) {
			double refactoredDuration = (double) m_sampleRateReader->getSampleRate() / (*scriptDuration->hz.get());
			duration = uint64_max(floor(refactoredDuration), 1);
			drift = refactoredDuration - duration;
		}

		return make_shared<DurationConstantProcessor>(duration, drift);
	} else if (scriptDuration->hzValue) {
		// Construct a variable duration processor with a sample rate
		shared_ptr<ValueProcessor> valueProcessor = parseValue(scriptDuration->hzValue.get(), vector<string>());
		return make_shared<DurationVariableHzProcessor>(valueProcessor, m_sampleRateReader->getSampleRate());
	} else {
		// Construct a variable duration processor with a factor
		shared_ptr<ValueProcessor> valueProcessor;
		double factor = 1.f;

		if (scriptDuration->samplesValue) {
			float activeSampleRate = m_sampleRateReader->getSampleRate();
			if ((timeScale) && (timeScale->sampleRate) && (*timeScale->sampleRate.get() != activeSampleRate)) {
				factor = activeSampleRate / (*timeScale->sampleRate.get());
			}
			valueProcessor = parseValue(scriptDuration->samplesValue.get(), vector<string>());
		} else if (scriptDuration->millisValue) {
			factor = (double) m_sampleRateReader->getSampleRate() / 1000;
			valueProcessor = parseValue(scriptDuration->millisValue.get(), vector<string>());
		} else if (scriptDuration->beatsValue) {
			if ((timeScale) && (timeScale->bpm)) {
				int bpm = *timeScale->bpm.get();
				factor = (double) m_sampleRateReader->getSampleRate() * 60 / bpm;
				valueProcessor = parseValue(scriptDuration->beatsValue.get(), vector<string>());
			} else {
				ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_BeatsButNoBmp, "The segment duration uses beats, but no bpm (beats per minute) is specified on the timeline.");
				return shared_ptr<DurationProcessor>();
			}
		}

		return make_shared<DurationVariableFactorProcessor>(valueProcessor, factor);
	}
}
