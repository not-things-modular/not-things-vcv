#include "core/timeseq-json.hpp"
#include <sstream>
#include <stdarg.h>

using namespace std;
using namespace timeseq;


#define ADD_VALIDATION_ERROR(validationErrors, location, code, message...) \
	if (validationErrors != nullptr) { \
		string errorLocation = createValidationErrorLocation(location); \
		string errorMessage = createValidationErrorMessage(code, message, ""); \
		validationErrors->emplace_back(errorLocation, errorMessage); \
	}

class ValidationErrorHandler : public json_schema::error_handler {
	public:
		ValidationErrorHandler(vector<JsonValidationError> *validationErrors) {
			m_validationErrors = validationErrors;
		}

		void error(const json::json_pointer &ptr, const json &instance, const string &message) override
		{
			if (m_validationErrors != nullptr) {
				string location = ptr.to_string();
				m_validationErrors->emplace_back(location, message);
			}
		}

	private:
		vector<JsonValidationError> *m_validationErrors;
};

string createValidationErrorMessage(ValidationErrorCode code, ...) {
	ostringstream errorMessage;

	va_list args;
	va_start(args, code);
	char* message = va_arg(args, char*);
	while (message[0] != 0) {
		errorMessage << message;
		message = va_arg(args, char*);
	}
	va_end(args);

	errorMessage << " [" << code << "]";

	return errorMessage.str();
}

string createValidationErrorLocation(vector<string> location) {
	ostringstream errorLocation;
	for (const string& entry : location) {
		errorLocation << "/";
		errorLocation << entry;
	}

	if (errorLocation.tellp() == 0) {
		errorLocation << "/";
	}

	return errorLocation.str();
}

template<size_t N>
bool hasOneOf(const json& json, const char* (&&propertyNames)[N]) {
	for (const char* propertyName : propertyNames) {
		if (json.find(propertyName) != json.end()) {
			return true;
		}
	}
	return false;
}


std::shared_ptr<Script> JsonScriptParser::parseScript(const json& scriptJson, vector<JsonValidationError> *validationErrors, vector<string> location) {
	Script* script = new Script();

	json::const_iterator type = scriptJson.find("type");
	if ((type == scriptJson.end()) || (!type->is_string())) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TypeMissing, "type is required and must be a string.");
	} else {
		script->type = *type;
		if (script->type != "not-things_timeseq_script") {
			std::string typeValue = (*type);
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TypeUnsupported, "type '", typeValue.c_str(), "' is not supported.");
		}
	}
	
	json::const_iterator version = scriptJson.find("version");
	if ((version == scriptJson.end()) || (!version->is_string())) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_VersionMissing, "version is required and must be a string.");
	}
	else {
		script->version = *version;
		if (script->version != "0.0.1") {
			std::string versionValue = (*version);
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_VersionUnsupported, "version '", versionValue.c_str(), "' is not supported.");
		}
	}

	json::const_iterator timelines = scriptJson.find("timelines");
	if ((timelines != scriptJson.end()) && (timelines->is_array())) {
		location.push_back("timelines");

		int count = 0;
		std::vector<json> timelineElements = (*timelines);
		for (const json& timeline : timelineElements) {
			location.push_back(std::to_string(count));
			if (timeline.is_object()) {
				script->timelines.push_back(parseTimeline(timeline, validationErrors, location));
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TimelineObject, "'timelines' elements must be objects.");
			}
			location.pop_back();
			count++;
		}

		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TimelinesMissing, "timelines is required and must be an array.");
	}

	json::const_iterator segmentBlocks = scriptJson.find("segment-blocks");
	if (segmentBlocks != scriptJson.end()) {
		if (segmentBlocks->is_array()) {
			location.push_back("segment-blocks");

			int count = 0;
			std::vector<json> segmentBlockElements = (*segmentBlocks);
			for (const json& segmentBlock : segmentBlockElements) {
				location.push_back(std::to_string(count));
				if (segmentBlock.is_array()) {
					script->segmentBlocks.push_back(parseSegmentBlock(segmentBlock, false, validationErrors, location));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_SegmentBlockObject, "'segment-blocks' elements must be objects.");
				}
				location.pop_back();
				count++;
			}

			if (count == 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_SegmentBlocksItemRequired, "At least one segment-block item is required.");
			}
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_SegmentBlocksArray, "segment-blocks must be an array.");
		}
	}

	json::const_iterator segments = scriptJson.find("segments");
	if (segments != scriptJson.end()) {
		if (segments->is_array()) {
			location.push_back("segments");

			int count = 0;
			std::vector<json> segmentElements = (*segments);
			for (const json& segment : segmentElements) {
				location.push_back(std::to_string(count));
				if (segment.is_object()) {
					script->segments.push_back(parseSegment(segment, false, validationErrors, location));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_SegmentObject, "'segments' elements must be objects.");
				}
				location.pop_back();
				count++;
			}

			if (count == 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_SegmentsItemRequired, "At least one segment item is required.");
			}
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_SegmentsArray, "segments must be an array.");
		}
	}

	json::const_iterator inputs = scriptJson.find("inputs");
	if (inputs != scriptJson.end()) {
		if (inputs->is_array()) {
			location.push_back("inputs");

			int count = 0;
			std::vector<json> inputElements = (*inputs);
			for (const json& input : inputElements) {
				location.push_back(std::to_string(count));
				if (input.is_object()) {
					script->inputs.push_back(parseInput(input, false, validationErrors, location));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_InputObject, "'inputs' elements must be objects.");
				}
				location.pop_back();
				count++;
			}

			if (count == 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_InputsItemRequired, "At least one input item is required.");
			}
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_InputsArray, "inputs must be an array.");
		}
	}

	json::const_iterator outputs = scriptJson.find("outputs");
	if (outputs != scriptJson.end()) {
		if (outputs->is_array()) {
			location.push_back("outputs");

			int count = 0;
			std::vector<json> outputElements = (*outputs);
			for (const json& output : outputElements) {
				location.push_back(std::to_string(count));
				if (output.is_object()) {
					script->outputs.push_back(parseOutput(output, false, validationErrors, location));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_OutputObject, "'outputs' elements must be objects.");
				}
				location.pop_back();
				count++;
			}

			if (count == 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_OutputsItemRequired, "At least one output item is required.");
			}
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_OutputsArray, "outputs must be an array.");
		}
	}

	json::const_iterator calcs = scriptJson.find("calcs");
	if (calcs != scriptJson.end()) {
		if (calcs->is_array()) {
			location.push_back("calcs");

			int count = 0;
			std::vector<json> calcElements = (*calcs);
			for (const json& calc : calcElements) {
				location.push_back(std::to_string(count));
				if (calc.is_object()) {
					script->calcs.push_back(parseCalc(calc, false, validationErrors, location));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_CalcObject, "'calcs' elements must be objects.");
				}
				location.pop_back();
				count++;
			}

			if (count == 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_CalcsItemRequired, "At least one calc item is required.");
			}
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_CalcsArray, "'calcs' must be an array.");
		}
	}

	json::const_iterator values = scriptJson.find("values");
	if (values != scriptJson.end()) {
		if (values->is_array()) {
			location.push_back("values");

			int count = 0;
			std::vector<json> valueElements = (*values);
			for (const json& value : valueElements) {
				location.push_back(std::to_string(count));
				if (value.is_object()) {
					script->values.push_back(parseValue(value, false, validationErrors, location));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_ValueObject, "'values' elements must be objects.");
				}
				location.pop_back();
				count++;
			}

			if (count == 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_ValuesItemRequired, "At least one value item is required.");
			}
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_ValuesArray, "'values' must be an array.");
		}
	}

	json::const_iterator actions = scriptJson.find("actions");
	if (actions != scriptJson.end()) {
		if (actions->is_array()) {
			location.push_back("actions");

			int count = 0;
			std::vector<json> actionElements = (*actions);
			for (const json& action : actionElements) {
				location.push_back(std::to_string(count));
				if (action.is_object()) {
					script->actions.push_back(parseAction(action, false, validationErrors, location));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_ActionObject, "'actions' elements must be objects.");
				}
				location.pop_back();
				count++;
			}

			if (count == 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_ActionsItemRequired, "At least one action item is required.");
			}
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_ActionsArray, "'actions' must be an array.");
		}
	}

	json::const_iterator inputTriggers = scriptJson.find("input-triggers");
	if (inputTriggers != scriptJson.end()) {
		if (inputTriggers->is_array()) {
			location.push_back("input-triggers");

			int count = 0;
			std::vector<json> inputTriggerElements = (*inputTriggers);
			for (const json& inputTrigger : inputTriggerElements) {
				location.push_back(std::to_string(count));
				if (inputTrigger.is_object()) {
					script->inputTriggers.push_back(parseInputTrigger(inputTrigger, false, validationErrors, location));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_InputTriggerObject, "'input-triggers' elements must be objects.");
				}
				location.pop_back();
				count++;
			}

			if (count == 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_InputTriggersItemRequired, "At least one input-trigger item is required.");
			}
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_InputTriggersArray, "'input-triggers' must be an array.");
		}
	}

	return shared_ptr<Script>(script);
}

ScriptTimeline JsonScriptParser::parseTimeline(const json& timelineJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptTimeline timeline;

	json::const_iterator timeScale = timelineJson.find("time-scale");
	timeline.loopLock = false;
	if (timeScale != timelineJson.end()) {
		if (timeScale->is_object()) {
			location.push_back("time-scale");
			ScriptTimeScale *scriptTimeScale = new ScriptTimeScale(parseTimeScale(*timeScale, validationErrors, location));
			timeline.timeScale.reset(scriptTimeScale);
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Timeline_TimeScaleObject, "time-scale must be an object.");
		}
	}

	json::const_iterator lanes = timelineJson.find("lanes");
	if ((lanes != timelineJson.end()) && (lanes->is_array())) {
		location.push_back("lanes");

		int count = 0;
		std::vector<json> laneElements = (*lanes);
		for (const json& lane : laneElements) {
			location.push_back(std::to_string(count));
			if (lane.is_object()) {
				timeline.lanes.push_back(parseLane(lane, validationErrors, location));
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Timeline_LaneObject, "'lanes' elements must be objects.");
			}
			location.pop_back();
			count++;
		}

		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Timeline_LanesMissing, "lanes is required and must be an array.");
	}

	json::const_iterator loopLock = timelineJson.find("loop-lock");
	if (loopLock != timelineJson.end()) {
		if (!loopLock->is_boolean()) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Timeline_LoopLockBoolean, "looplock must be a boolean.");
		} else {
			timeline.loopLock = (*loopLock);
		}
	}

	return timeline;
}

ScriptTimeScale JsonScriptParser::parseTimeScale(const json& timeScaleJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptTimeScale timeScale;

	json::const_iterator sampleRate = timeScaleJson.find("sample-rate");
	if (sampleRate != timeScaleJson.end()) {
		if (sampleRate->is_number_unsigned()) {
			timeScale.sampleRate.reset(new int(*sampleRate));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_SampleRateNumber, "sample-rate must be an unsigned number.");
		}
	}

	json::const_iterator bpm = timeScaleJson.find("bpm");
	if (bpm != timeScaleJson.end()) {
		if (bpm->is_number_unsigned()) {
			timeScale.bpm.reset(new int(*bpm));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_BpmNumber, "bpm must be an unsigned number.");
		}
	}

	json::const_iterator bpb = timeScaleJson.find("bpb");
	if (bpb != timeScaleJson.end()) {
		if (bpb->is_number_unsigned()) {
			timeScale.bpb.reset(new int(*bpb));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_BpbNumber, "bpb must be an unsigned number.");
		}
	}

	if (!(timeScale.sampleRate) && !(timeScale.bpm)) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_SampleRateOnly, "One of sample-rate or bpm is required.");
	} else if ((timeScale.sampleRate) && (timeScale.bpm)) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_SampleRateOnly, "No bpm can be set if sample-rate is set.");
	} else if ((timeScale.bpb) && !(timeScale.bpm)) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_BpbRequiresBpm, "bpm  must be set if bpb is set.");
	}

	return timeScale;
}

ScriptLane JsonScriptParser::parseLane(const json& laneJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptLane lane;

	json::const_iterator loop = laneJson.find("loop");
	lane.loop = false;
	if (loop != laneJson.end()) {
		if (loop->is_boolean()) {
			lane.loop = *loop;
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_LoopBoolean, "loop must be an boolean.");
		}
	}

	json::const_iterator repeat = laneJson.find("repeat");
	lane.repeat = 0;
	if (repeat != laneJson.end()) {
		if (repeat->is_number_unsigned()) {
			lane.repeat = *repeat;
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_RepeatNumber, "repeat must be an unsigned number.");
		}
	}

	json::const_iterator startTrigger = laneJson.find("start-trigger");
	if (startTrigger != laneJson.end()) {
		if (startTrigger->is_string()) {
			lane.startTrigger = *startTrigger;
			if (lane.startTrigger.length() == 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_StartTriggerLength, "start-trigger can not be an empty string.");
			}
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_StartTriggerString, "start-trigger must be a string.");
		}
	}

	json::const_iterator stopTrigger = laneJson.find("stop-trigger");
	if (stopTrigger != laneJson.end()) {
		if (stopTrigger->is_string()) {
			lane.stopTrigger = *stopTrigger;
			if (lane.stopTrigger.length() == 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_StopTriggerLength, "stop-trigger can not be an empty string.");
			}
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_StopTriggerString, "stop-trigger must be a string.");
		}
	}

	json::const_iterator segments = laneJson.find("segments");
	if ((segments != laneJson.end()) && (segments->is_array())) {
		location.push_back("segments");

		int count = 0;
		std::vector<json> segmentElements = (*segments);
		for (const json& segment : segmentElements) {
			location.push_back(std::to_string(count));
			if (segment.is_object()) {
				lane.segments.push_back(parseSegmentEntity(segment, validationErrors, location));
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_SegmentObject, "'segments' elements must be objects.");
			}
			location.pop_back();
			count++;
		}

		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_SegmentsMissing, "segments is required and must be an array.");
	}

	return lane;
}

ScriptSegmentEntity JsonScriptParser::parseSegmentEntity(const json& segmentEntityJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptSegmentEntity segmentEntity;

	json::const_iterator segment = segmentEntityJson.find("segment");
	if (segment != segmentEntityJson.end()) {
		if (segment->is_object()) {
			location.push_back("segment");
			ScriptSegment* scriptSegment = new ScriptSegment(parseSegment(*segment, true, validationErrors, location));
			segmentEntity.segment.reset(scriptSegment);
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SegmentEntity_SegmentObject, "segment must be an object.");
		}
	}

	json::const_iterator segmentBlock = segmentEntityJson.find("segment-block");
	if (segmentBlock != segmentEntityJson.end()) {
		if (segmentBlock->is_object()) {
			location.push_back("segment-block");
			ScriptSegmentBlock* scriptSegmentBlock = new ScriptSegmentBlock(parseSegmentBlock(*segmentBlock, true, validationErrors, location));
			segmentEntity.segmentBlock.reset(scriptSegmentBlock);
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SegmentEntity_SegmentBlockObject, "segment-block must be an object.");
		}
	}

	if (segmentEntity.segment && segmentEntity.segmentBlock) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SegmentEntity_EitherSegmentOrSegmentBlock, "segment and segment-block can not be used at the same time.");
	} else if (!segmentEntity.segment && !segmentEntity.segmentBlock) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SegmentEntity_EitherSegmentOrSegmentBlock, "Either segment or segment-block must be present.");
	}

	return segmentEntity;
}

ScriptSegment JsonScriptParser::parseSegment(const json& segmentJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptSegment segment;

	populateRef(segment, segmentJson, allowRefs, validationErrors, location);
	if (segment.ref.length() > 0) {
		if (hasOneOf(segmentJson, { "duration", "actions" })) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Segment_RefOrInstance, "A ref segment can not be combined other non-ref segment properties.");
		}
	} else {
		json::const_iterator duration = segmentJson.find("duration");
		if ((duration != segmentJson.end()) && (duration->is_object())) {
			location.push_back("duration");
			segment.duration = parseDuration(*duration, validationErrors, location);
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SegmentEntity_EitherSegmentOrSegmentBlock, "duration is required and must be an object.");
		}

		json::const_iterator actions = segmentJson.find("actions");
		if (actions != segmentJson.end()) {
			if (actions->is_array()) {
				location.push_back("actions");

				int count = 0;
				std::vector<json> actionElements = (*actions);
				for (const json& action : actionElements) {
					location.push_back(std::to_string(count));
					if (action.is_object()) {
						segment.actions.push_back(parseAction(action, true, validationErrors, location));
					} else {
						ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Segment_ActionObject, "'actions' elements must be objects.");
					}
					location.pop_back();
					count++;
				}

				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Segment_ActionsArray, "actions must be an array.");
			}
		}
	}

	return segment;
}

ScriptSegmentBlock JsonScriptParser::parseSegmentBlock(const json& segmentBlockJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptSegmentBlock segmentBlock;

	populateRef(segmentBlock, segmentBlockJson, allowRefs, validationErrors, location);
	if (segmentBlock.ref.length() > 0) {
		if (hasOneOf(segmentBlockJson, { "segments" })) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SegmentBlock_RefOrInstance, "A ref segment-block can not be combined other non-ref segment-block properties.");
		}
	} else {
		json::const_iterator segments = segmentBlockJson.find("segments");
		if ((segments != segmentBlockJson.end()) && (segments->is_array())) {
			location.push_back("segments");

			int count = 0;
			std::vector<json> segmentElements = (*segments);
			for (const json& segment : segmentElements) {
				location.push_back(std::to_string(count));
				if (segment.is_object()) {
					segmentBlock.segments.push_back(parseSegmentEntity(segment, validationErrors, location));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SegmentBlock_SegmentObject, "'segments' elements must be objects.");
				}
				location.pop_back();
				count++;
			}

			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SegmentBlock_SegmentsArray, "segments is required and must be an array.");
		}
	}

	return segmentBlock;
}

ScriptDuration JsonScriptParser::parseDuration(const json& durationJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptDuration duration;
	int durationCount = 0;

	json::const_iterator samples = durationJson.find("samples");
	if (samples != durationJson.end()) {
		if ((samples->is_number_unsigned()) && ((*samples) > 0)) {
			duration.samples.reset(new int(*samples));
			durationCount++;
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_SamplesNumber, "samples must be a positive integer number.");
		}
	}

	json::const_iterator millis = durationJson.find("millis");
	if (millis != durationJson.end()) {
		if ((millis->is_number_float()) && ((*millis) > 0)) {
			duration.millis.reset(new float(*millis));
			durationCount++;
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_MillisNumber, "millis must be a positive decimal number.");
		}
	}

	json::const_iterator bars = durationJson.find("bars");
	if (bars != durationJson.end()) {
		if ((bars->is_number_unsigned()) && ((*bars) > 0)) {
			duration.bars.reset(new int(*bars));
			durationCount++;
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_BarsNumber, "bars must be a positive integer number.");
		}
	}

	json::const_iterator beats = durationJson.find("beats");
	if (beats != durationJson.end()) {
		if ((beats->is_number_float()) && ((*beats) > 0)) {
			duration.beats.reset(new float(*beats));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_BeatsNumber, "beats must be a positive decimal number.");
		}
	}

	if (durationCount == 0) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_NoSamplesOrMillisOrBars, "either samples, millis or beats must be used.");
	} else if (durationCount > 1) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_EitherSamplesOrMillisOrBars, "only one of samples, millis or beats can be used at a time.");
	} else if (duration.bars && !duration.beats) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_BarsRequiresBeats, "bars can not be used without beats.");
	}

	return duration;
}

ScriptAction JsonScriptParser::parseAction(const json& actionJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptAction action;

	populateRef(action, actionJson, allowRefs, validationErrors, location);
	if (action.ref.length() > 0) {
		if (hasOneOf(actionJson, { "timing", "set-value", "set-polyphony", "trigger", "start-value", "end-value" })) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_RefOrInstance, "A ref action can not be combined other non-ref action properties.");
		}
	} else {
		json::const_iterator timing = actionJson.find("timing");
		if ((timing != actionJson.end()) && (timing->is_string())) {
			if (*timing == "start") {
				action.timing = ScriptAction::ActionTiming::START;
			} else if (*timing == "end") {
				action.timing = ScriptAction::ActionTiming::END;
			} else if (*timing == "glide") {
				action.timing = ScriptAction::ActionTiming::GLIDE;
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_RefOrInstance, "timing is required and must be either 'start', 'end' or 'glide'.");
			}
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_RefOrInstance, "timing is required and must be either 'start', 'end' or 'glide'.");
		}

		json::const_iterator setValue = actionJson.find("set-value");
		if (setValue != actionJson.end()) {
			if (setValue->is_object()) {
				location.push_back("set-value");
				ScriptSetValue *scriptSetValue = new ScriptSetValue(parseSetValue(*setValue, validationErrors, location));
				action.setValue.reset(scriptSetValue);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_SetValueObject, "set-value must be an object.");
			}
		}

		json::const_iterator setPolyphony = actionJson.find("set-polyphony");
		if (setPolyphony != actionJson.end()) {
			if (setPolyphony->is_object()) {
				location.push_back("set-polyphony");
				ScriptSetPolyphony *scriptSetPolyphony = new ScriptSetPolyphony(parseSetPolyphony(*setPolyphony, validationErrors, location));
				action.setPolyphony.reset(scriptSetPolyphony);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_SetPolyphonyObject, "set-polyphony must be an object.");
			}
		}

		json::const_iterator trigger = actionJson.find("trigger");
		if (trigger != actionJson.end()) {
			if (trigger->is_string()) {
				action.trigger = *trigger;
				if (action.trigger.size() == 0) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_TriggerLength, "trigger can not be an empty string.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_TriggerString, "trigger must be a string.");
			}
		}

		json::const_iterator startValue = actionJson.find("start-value");
		if (startValue != actionJson.end()) {
			if (startValue->is_object()) {
				location.push_back("start-value");
				ScriptValue *scriptValue = new ScriptValue(parseValue(*startValue, true, validationErrors, location));
				action.startValue.reset(scriptValue);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_StartValueObject, "start-value must be an object.");
			}
		}

		json::const_iterator endValue = actionJson.find("end-value");
		if (endValue != actionJson.end()) {
			if (endValue->is_object()) {
				location.push_back("end-value");
				ScriptValue *scriptValue = new ScriptValue(parseValue(*endValue, true, validationErrors, location));
				action.endValue.reset(scriptValue);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_EndValueObject, "end-value must be an object.");
			}
		}

		if ((action.timing == ScriptAction::ActionTiming::GLIDE) && ((action.setValue) || (action.setPolyphony) || (action.trigger.size() > 0))) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_NonGlideProperties, "set-value, set-polyphony and trigger can not be used in combination with 'GLIDE' timing.");
		} if ((action.timing != ScriptAction::ActionTiming::GLIDE) && ((action.startValue) || (action.endValue))) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_OnlyGlideProperties, "start-value and end-value can only be used in combination with 'GLIDE' timing.");
		}
	}

	return action;
}

ScriptSetValue JsonScriptParser::parseSetValue(const json& setValueJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptSetValue setValue;

	json::const_iterator output = setValueJson.find("output");
	if ((output != setValueJson.end()) && (output->is_object())) {
		location.push_back("output");
		setValue.output = parseOutput(*output, true, validationErrors, location);
		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetValue_OutputObject, "'output' is required and must be an object.");
	}

	json::const_iterator value = setValueJson.find("value");
	if ((value != setValueJson.end()) && (value->is_object())) {
		location.push_back("value");
		setValue.value = parseValue(*output, true, validationErrors, location);
		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetValue_ValueObject, "'value' is required and must be an object.");
	}

	return setValue;
}

ScriptSetPolyphony JsonScriptParser::parseSetPolyphony(const json& setPolyphonyJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptSetPolyphony setPolyphony;

	json::const_iterator index = setPolyphonyJson.find("index");
	if ((index != setPolyphonyJson.end()) && (index->is_number_unsigned())) {
		setPolyphony.index = *index;
		if ((setPolyphony.index < 1) || (setPolyphony.index > 8)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetPolyphony_IndexRange, "'index' must be a number between 1 and 8.");
		}
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetPolyphony_IndexNumber, "'index' is required and must be a number between 1 and 8.");
	}

	json::const_iterator channels = setPolyphonyJson.find("channels");
	if ((channels != setPolyphonyJson.end()) && (channels->is_number_unsigned())) {
		setPolyphony.channels = *channels;
		if ((setPolyphony.channels < 1) || (setPolyphony.channels > 16)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetPolyphony_ChannelsRange, "'channels' must be a number between 1 and 16.");
		}
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetPolyphony_ChannelsNumber, "'channels' is required and must be a number between 1 and 16.");
	}

	return setPolyphony;
}

ScriptValue JsonScriptParser::parseValue(const json& valueJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptValue value;

	populateRef(value, valueJson, allowRefs, validationErrors, location);
	if (value.ref.length() > 0) {
		if (hasOneOf(valueJson, { "voltage", "note", "input", "output", "rand", "calc" })) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_RefOrInstance, "A ref value can not be combined other non-ref value properties.");
		}
	} else {
		json::const_iterator voltage = valueJson.find("voltage");
		if (voltage != valueJson.end()) {
			if (voltage->is_number_float()) {
				value.voltage.reset(new float(*voltage));
				if ((*value.voltage < -10) || (*value.voltage > 10)) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_VoltageRange, "'voltage' must be a decimal number between -10 and 10.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_VoltageFloat, "'voltage' must be a decimal number between -10 and 10.");
			}
		}

		json::const_iterator note = valueJson.find("note");
		if (note != valueJson.end()) {
			if (note->is_string()) {
				value.note.reset(new std::string(*note));
				if ((value.note->size() < 2) || (value.note->size() > 3)) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_NoteFormat, "'note' must be a string with a note name (A-G), an octave (0-9) and optionally an accidental (+ for sharp, - for flat).");
				} else {
					char n = toupper((*value.note)[0]);
					if (n < 'A' || n > 'Z') {
						ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_NoteFormat, "'note' must start with a valid note name (A-G).");
					}
					char s = (*value.note)[1];
					if (s < '0' || s > '9') {
						ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_NoteFormat, "'note' must have a valid scale (0-9) as second character.");
					}
					if (value.note->size() == 3) {
						char a = (*value.note)[2];
						if (a != '+' && a != '-') {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_NoteFormat, "The third character of 'note' must be a valid accidental (+ for sharp, - for flat).");
						}
					}
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_NoteString, "'note' must be a string with a note name (A-G), an octave (0-9) and optionally an accidental (+ for sharp, - for flat).");
			}
		}

		json::const_iterator input = valueJson.find("input");
		if (input != valueJson.end()) {
			if (input->is_object()) {
				location.push_back("input");
				ScriptInput* scriptInput = new ScriptInput(parseInput(*input, true, validationErrors, location));
				value.input.reset(scriptInput);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_InputObject, "'input' must be an object.");
			}
		}

		json::const_iterator output = valueJson.find("output");
		if (output != valueJson.end()) {
			if (output->is_object()) {
				location.push_back("output");
				ScriptOutput* scriptOutput = new ScriptOutput(parseOutput(*output, true, validationErrors, location));
				value.output.reset(scriptOutput);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_OutputObject, "'output' must be an object.");
			}
		}
		
		json::const_iterator rand = valueJson.find("rand");
		if (rand != valueJson.end()) {
			if (rand->is_object()) {
				location.push_back("rand");
				ScriptRand* scriptRand = new ScriptRand(parseRand(*rand, validationErrors, location));
				value.rand.reset(scriptRand);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_RandObject, "'rand' must be an object.");
			}
		}

		json::const_iterator calcs = valueJson.find("calc");
		if (calcs != valueJson.end()) {
			if (calcs->is_array()) {
				location.push_back("calc");

				int count = 0;
				std::vector<json> calcElements = (*calcs);
				for (const json& calc : calcElements) {
					location.push_back(std::to_string(count));
					if (calc.is_object()) {
						value.calc.push_back(parseCalc(calc, true, validationErrors, location));
					} else {
						ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_RandObject, "'calc' elements must be objects.");
					}
					location.pop_back();
					count++;
				}
		
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_CalcArray, "'calc' must be an array.");
			}
		}
	}

	return value;
}

ScriptOutput JsonScriptParser::parseOutput(const json& outputJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptOutput output;

	populateRef(output, outputJson, allowRefs, validationErrors, location);
	if (output.ref.length() > 0) {
		if (hasOneOf(outputJson, { "index", "channel" })) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Output_RefOrInstance, "A ref output can not be combined other non-ref output properties.");
		}
	} else {
		json::const_iterator index = outputJson.find("index");
		if ((index != outputJson.end()) && (index->is_number_unsigned())) {
			output.index = *index;
			if ((output.index < 1) || (output.index > 8)) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Output_IndexRange, "'index' must be a number between 1 and 8.");
			}
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Output_IndexNumber, "'index' is required and must be a number between 1 and 8.");
		}

		json::const_iterator channel = outputJson.find("channel");
		if (channel != outputJson.end()) {
			if (channel->is_number_unsigned()) {
				output.channel.reset(new int(*channel));
				if ((*output.channel < 1) || (*output.channel > 16)) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Output_ChannelRange, "'channel' must be a number between 1 and 16.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetPolyphony_ChannelsNumber, "'channel' must be a number between 1 and 16.");
			}
		}
	}

	return output;
}

ScriptInput JsonScriptParser::parseInput(const json& inputJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptInput input;

	populateRef(input, inputJson, allowRefs, validationErrors, location);
	if (input.ref.length() > 0) {
		if (hasOneOf(inputJson, { "index", "channel" })) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Input_RefOrInstance, "A ref input can not be combined other non-ref input properties.");
		}
	} else {
		json::const_iterator index = inputJson.find("index");
		if ((index != inputJson.end()) && (index->is_number_unsigned())) {
			input.index = *index;
			if ((input.index < 1) || (input.index > 8)) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Input_IndexRange, "'index' must be a number between 1 and 8.");
			}
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Input_IndexNumber, "'index' is required and must be a number between 1 and 8.");
		}

		json::const_iterator channel = inputJson.find("channel");
		if (channel != inputJson.end()) {
			if (channel->is_number_unsigned()) {
				input.channel.reset(new int(*channel));
				if ((*input.channel < 1) || (*input.channel > 16)) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Input_ChannelRange, "'channel' must be a number between 1 and 16.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetPolyphony_ChannelsNumber, "'channel' must be a number between 1 and 16.");
			}
		}
	}

	return input;
}

ScriptRand JsonScriptParser::parseRand(const json& randJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptRand rand;

	json::const_iterator lower = randJson.find("lower");
	if ((lower != randJson.end()) && (lower->is_object())) {
		location.push_back("lower");
		ScriptValue *scriptValue = new ScriptValue(parseValue(*lower, true, validationErrors, location));
		rand.lower.reset(scriptValue);
		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Rand_LowerObject, "'lower' is required and must be an object.");
	}

	json::const_iterator upper = randJson.find("upper");
	if ((upper != randJson.end()) && (upper->is_object())) {
		location.push_back("upper");
		ScriptValue *scriptValue = new ScriptValue(parseValue(*upper, true, validationErrors, location));
		rand.upper.reset(scriptValue);
		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Rand_UpperObject, "'upper' is required and must be an object.");
	}

	return rand;
}

ScriptCalc JsonScriptParser::parseCalc(const json& calcJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptCalc calc;

	populateRef(calc, calcJson, allowRefs, validationErrors, location);
	if (calc.ref.length() > 0) {
		if (hasOneOf(calcJson, { "add", "sub", "div", "mult" })) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Input_RefOrInstance, "A ref calc can not be combined other non-ref input properties.");
		}
	} else {
		int count = 0;

		json::const_iterator add = calcJson.find("add");
		if (add != calcJson.end()) {
			if (add->is_object()) {
				calc.operation = ScriptCalc::CalcOperation::ADD;
				location.push_back("add");
				ScriptValue *scriptValue = new ScriptValue(parseValue(*add, true, validationErrors, location));
				calc.value.reset(scriptValue);
				location.pop_back();
				count++;
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_AddObject, "'add' must be an object.");
			}
		}

		json::const_iterator sub = calcJson.find("sub");
		if (sub != calcJson.end()) {
			if (sub->is_object()) {
				calc.operation = ScriptCalc::CalcOperation::SUB;
				location.push_back("sub");
				ScriptValue *scriptValue = new ScriptValue(parseValue(*sub, true, validationErrors, location));
				calc.value.reset(scriptValue);
				location.pop_back();
				count++;
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_SubObject, "'sub' must be an object.");
			}
		}

		json::const_iterator div = calcJson.find("div");
		if (div != calcJson.end()) {
			if (div->is_object()) {
				calc.operation = ScriptCalc::CalcOperation::DIV;
				location.push_back("div");
				ScriptValue *scriptValue = new ScriptValue(parseValue(*div, true, validationErrors, location));
				calc.value.reset(scriptValue);
				location.pop_back();
				count++;
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_DivObject, "'div' must be an object.");
			}
		}

		json::const_iterator mult = calcJson.find("mult");
		if (mult != calcJson.end()) {
			if (mult->is_object()) {
				calc.operation = ScriptCalc::CalcOperation::MULT;
				location.push_back("mult");
				ScriptValue *scriptValue = new ScriptValue(parseValue(*mult, true, validationErrors, location));
				calc.value.reset(scriptValue);
				location.pop_back();
				count++;
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_MultObject, "'mult' must be an object.");
			}
		}

		if (count == 0) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_NoOperation, "Either 'add', 'sub', 'div' or 'mult' must be set.");
		} else if (count > 1) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_MultpleOperations, "At most one of 'add', 'sub', 'div' or 'mult' may be set.");
		}
	}

	return calc;
}

ScriptInputTrigger JsonScriptParser::parseInputTrigger(const json& inputTriggerJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptInputTrigger inputTrigger;

	json::const_iterator id = inputTriggerJson.find("id");
	if ((id != inputTriggerJson.end()) && (id->is_string())) {
		inputTrigger.id = *id;
		if (inputTrigger.id.length() == 0) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::InputTrigger_IdLength, "'id' can not be an empty string.");
		}
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::InputTrigger_IdString, "'id' is required and must be a string.");
	}

	json::const_iterator input = inputTriggerJson.find("input");
	if ((input != inputTriggerJson.end()) && (input->is_object())) {
		location.push_back("input");
		inputTrigger.input = parseInput(*input, true, validationErrors, location);
		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::InputTrigger_InputObject, "'input' is required and must be an object.");
	}

	return inputTrigger;
}

void JsonScriptParser::populateRef(ScriptRefObject &refObject, const json& refJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location) {
	json::const_iterator ref = refJson.find("ref");
	json::const_iterator id = refJson.find("id");

	if (allowRefs) {
		if (ref != refJson.end()) {
			if (ref->is_string()) {
				string refValue = *ref;
				if (refValue.length() > 0) {
					refObject.ref = refValue;
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Ref_Length, "ref can not be an empty string.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Ref_String, "ref must be a string.");
			}
		}
		if (id != refJson.end()) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_NotAllowed, "id is not allowed here.");
		}
	} else {
		if ((id != refJson.end()) && (id->is_string())) {
			string idValue = *id;
			if (idValue.length() > 0) {
				refObject.id = idValue;
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_Length, "id can not be an empty string.");
			}
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_String, "id is required and must be a string.");
		}
		if (ref != refJson.end()) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Ref_NotAllowed, "ref is not allowed here.");
		}
	}
}