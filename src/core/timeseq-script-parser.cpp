#include "core/timeseq-script-parser-internal.hpp"

void verifyVersion(int expectedVersion, JsonScriptParseContext& context, const char* feature) {
	if (context.version < expectedVersion) {
		map<int, string> versionMap = {
				{ VERSION_1_0_0, "1.0.0" },
				{ VERSION_1_1_0, "1.1.0" },
				{ VERSION_1_2_0, "1.2.0" }
		};

		ADD_VALIDATION_ERROR(&context.validationErrors, context.location, ValidationErrorCode::Feature_Not_In_Version, feature, " requires version ", versionMap[expectedVersion].c_str(), " but the script has its version set to ", versionMap[context.version].c_str(), ".");
	}
}

bool verifyAllowedProperties(const json& json, vector<string> propertyNames, bool allowRef, JsonScriptParseContext& context) {
	uint64_t count = 0;
	string unexpectedKeys;

	if (json.is_object()) {
		for (json::const_iterator i = json.begin(); i != json.end(); i++) {
			if ((i.key().substr(0, 2) != "x-") && (find(propertyNames.begin(), propertyNames.end(), i.key()) == propertyNames.end())) {
				if (allowRef && (i.key() == "ref" || i.key() == "id")) {
					continue;
				}
				count++;
				if (unexpectedKeys.length() > 0) {
					unexpectedKeys += ", ";
				}
				unexpectedKeys += "'" + i.key() + "'";
			}
		}
	}

	if (count == 1) {
		ADD_VALIDATION_ERROR(&context.validationErrors, context.location, ValidationErrorCode::Unknown_Property, "Unknown property encountered: ", unexpectedKeys.c_str(), ".");
	} else if (count > 1) {
		ADD_VALIDATION_ERROR(&context.validationErrors, context.location, ValidationErrorCode::Unknown_Property, "Unknown properties encountered: ", unexpectedKeys.c_str(), ".");
	}

	return count > 0;
}

ScriptSequenceMoveDirection parseScriptSequenceMoveDirection(const json& moveDirectionJson, const char* property, ValidationErrorCode enumErrorCode, ValidationErrorCode stringErrorCode, JsonScriptParseContext& context) {
	ScriptSequenceMoveDirection moveDirection = ScriptSequenceMoveDirection::NONE;

	if (moveDirectionJson.is_string()) {
		string moveDirectionString = moveDirectionJson.get<string>();
		if (moveDirectionString == "forward") {
			moveDirection = ScriptSequenceMoveDirection::FORWARD;
		} else if (moveDirectionString == "backward") {
			moveDirection = ScriptSequenceMoveDirection::BACKWARD;
		} else if (moveDirectionString == "random") {
			moveDirection = ScriptSequenceMoveDirection::RANDOM;
		} else if (moveDirectionString == "none") {
			moveDirection = ScriptSequenceMoveDirection::NONE;
		} else {
			ADD_VALIDATION_ERROR(&context.validationErrors, context.location, enumErrorCode, "'", property, "' must be either 'forward', 'backward', 'random' or 'none'.");
		}
	} else {
		ADD_VALIDATION_ERROR(&context.validationErrors, context.location, stringErrorCode, "'", property, "' must be a string with either 'forward', 'backward', 'random' or 'none'.");
	}

	return moveDirection;
}

template<class ScriptType>
void parseChildArray(JsonScriptParseContext& context, const json& parent, std::string jsonTag, int version, vector<ScriptType>& scriptArray, std::function<ScriptType(const json&)> parseFunc, ValidationErrorCode objectErrorCode, ValidationErrorCode arrayErrorCode) {
	json::const_iterator items = parent.find(jsonTag);
	if (items != parent.end()) {
		if (version > 0) {
			verifyVersion(version, context, (std::string("'") + jsonTag + "'").c_str());
		}
		if (items->is_array()) {
			context.location.push_back(jsonTag);

			int count = 0;
			vector<string> ids;
			vector<json> elements = (*items);
			for (const json& element : elements) {
				context.location.push_back(to_string(count));
				if (element.is_object()) {
					scriptArray.push_back(parseFunc(element));
					if (find(ids.begin(), ids.end(), scriptArray.back().id) != ids.end()) {
						ADD_VALIDATION_ERROR(&context.validationErrors, context.location, ValidationErrorCode::Id_Duplicate, "Id '", scriptArray.back().id.c_str(), "' has already been used. Ids must be unique within the object type.");
					} else if (scriptArray.back().id.size() > 0) {
						ids.push_back(scriptArray.back().id);
					}
				} else {
					ADD_VALIDATION_ERROR(&context.validationErrors, context.location, objectErrorCode, "'", jsonTag.c_str(), "' elements must be objects.");
				}
				context.location.pop_back();
				count++;
			}

			context.location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(&context.validationErrors, context.location, arrayErrorCode, "'", jsonTag.c_str(), "' must be an array.");
		}
	}
}

void JsonScriptParseContext::reset() {
	version = 0;
	location.clear();
	validationErrors.clear();
}

const std::vector<ValidationError>& JsonScriptParser::getValidationErrors() {
	return m_context.validationErrors;
}

shared_ptr<Script> JsonScriptParser::parseScript(const json& scriptJson) {
	static const vector<string> scriptProperties = { "type", "version", "timelines", "global-actions", "input-triggers", "sequences", "component-pool", "$schema" };
	shared_ptr<Script> script = make_shared<Script>();
	vector<string> ids;

	m_context.reset();
	verifyAllowedProperties(scriptJson, scriptProperties, false, m_context);

	json::const_iterator type = scriptJson.find("type");
	if ((type == scriptJson.end()) || (!type->is_string())) {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Script_TypeMissing, "'type' is required and must be a string.");
	} else {
		script->type = *type;
		if (script->type != "not-things_timeseq_script") {
			string typeValue = (*type);
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Script_TypeUnsupported, "'type' '", typeValue.c_str(), "' is not supported.");
		}
	}

	json::const_iterator version = scriptJson.find("version");
	if ((version == scriptJson.end()) || (!version->is_string())) {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Script_VersionMissing, "'version' is required and must be a string.");
	}
	else {
		script->version = *version;
		if (script->version == "1.0.0") {
			m_context.version = 100;
		} else if (script->version == "1.1.0") {
			m_context.version = 110;
		} else if (script->version == "1.2.0") {
			m_context.version = 120;
		}
		else {
			string versionValue = (*version);
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Script_VersionUnsupported, "'version' '", versionValue.c_str(), "' is an unsupported version. Only versions 1.0.0, 1.1.0 and 1.2.0 are currently supported.");
		}
	}

	json::const_iterator timelines = scriptJson.find("timelines");
	if ((timelines != scriptJson.end()) && (timelines->is_array())) {
		m_context.location.push_back("timelines");

		int count = 0;
		vector<json> timelineElements = (*timelines);
		for (const json& timeline : timelineElements) {
			m_context.location.push_back(to_string(count));
			if (timeline.is_object()) {
				script->timelines.push_back(parseTimeline(timeline));
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Script_TimelineObject, "'timelines' elements must be objects.");
			}
			m_context.location.pop_back();
			count++;
		}

		m_context.location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Script_TimelinesMissing, "'timelines' is required and must be an array.");
	}

	json::const_iterator globalActions = scriptJson.find("global-actions");
	if (globalActions != scriptJson.end()) {
		if (globalActions->is_array()) {
			m_context.location.push_back("global-actions");

			int count = 0;
			vector<json> actionElements = (*globalActions);
			for (const json& action : actionElements) {
				m_context.location.push_back(to_string(count));
				if (action.is_object()) {
					script->globalActions.push_back(parseAction(action, true));
				} else {
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Script_GlobalActionsObject, "'global-actions' elements must be action objects.");
				}
				m_context.location.pop_back();
				count++;
			}

			m_context.location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Script_GlobalActionsArray, "'global-actions' must be an array.");
		}
	}

	json::const_iterator inputTriggers = scriptJson.find("input-triggers");
	if (inputTriggers != scriptJson.end()) {
		if (inputTriggers->is_array()) {
			m_context.location.push_back("input-triggers");

			int count = 0;
			vector<json> inputTriggerElements = (*inputTriggers);
			for (const json& inputTrigger : inputTriggerElements) {
				m_context.location.push_back(to_string(count));
				if (inputTrigger.is_object()) {
					script->inputTriggers.push_back(parseInputTrigger(inputTrigger));
				} else {
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Script_InputTriggerObject, "'input-triggers' elements must be objects.");
				}
				m_context.location.pop_back();
				count++;
			}

			m_context.location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Script_InputTriggersArray, "'input-triggers' must be an array.");
		}
	}

	parseChildArray<ScriptSequence>(m_context, scriptJson, "sequences", VERSION_1_1_0, script->sequences, [this](const json& sequence) { return parseSequence(sequence); }, ValidationErrorCode::Script_SequenceObject, ValidationErrorCode::Script_SequencesArray);

	json::const_iterator componentPool = scriptJson.find("component-pool");
	if (componentPool != scriptJson.end()) {
		if (componentPool->is_object()) {
			static const vector<string> componentPoolProperties = { "segment-blocks", "segments", "inputs", "outputs", "calcs", "values", "actions", "ifs", "tunings" };
			m_context.location.push_back("component-pool");

			verifyAllowedProperties(*componentPool, componentPoolProperties, false, m_context);

			parseChildArray<ScriptSegmentBlock>(m_context, *componentPool, "segment-blocks", 0, script->segmentBlocks, [this](const json& segmentBlock) { return parseSegmentBlock(segmentBlock); }, ValidationErrorCode::Script_SegmentBlockObject, ValidationErrorCode::Script_SegmentBlocksArray);
			parseChildArray<ScriptSegment>(m_context, *componentPool, "segments", 0, script->segments, [this](const json& segment) { return parseSegment(segment, false); }, ValidationErrorCode::Script_SegmentObject, ValidationErrorCode::Script_SegmentsArray);
			parseChildArray<ScriptInput>(m_context, *componentPool, "inputs", 0, script->inputs, [this](const json& input) { return parseFullInput(input, false, false); }, ValidationErrorCode::Script_InputObject, ValidationErrorCode::Script_InputsArray);
			parseChildArray<ScriptOutput>(m_context, *componentPool, "outputs", 0, script->outputs, [this](const json& output) { return parseFullOutput(output, false, false); }, ValidationErrorCode::Script_OutputObject, ValidationErrorCode::Script_OutputsArray);
			parseChildArray<ScriptCalc>(m_context, *componentPool, "calcs", 0, script->calcs, [this](const json& calc) { return parseCalc(calc, false); }, ValidationErrorCode::Script_CalcObject, ValidationErrorCode::Script_CalcsArray);
			parseChildArray<ScriptValue>(m_context, *componentPool, "values", 0, script->values, [this](const json& value) { return parseFullValue(value, false, false); }, ValidationErrorCode::Script_ValueObject, ValidationErrorCode::Script_ValuesArray);
			parseChildArray<ScriptAction>(m_context, *componentPool, "actions", 0, script->actions, [this](const json& action) { return parseAction(action, false); }, ValidationErrorCode::Script_ActionObject, ValidationErrorCode::Script_ActionsArray);
			parseChildArray<ScriptIf>(m_context, *componentPool, "ifs", 0, script->ifs, [this](const json& ifObj) { return parseIf(ifObj, false); }, ValidationErrorCode::Script_IfObject, ValidationErrorCode::Script_IfsArray);
			parseChildArray<ScriptTuning>(m_context, *componentPool, "tunings", VERSION_1_1_0, script->tunings, [this](const json& tuning) { return parseTuning(tuning, false); }, ValidationErrorCode::Script_TuningObject, ValidationErrorCode::Script_TuningsArray);

			m_context.location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Script_ComponentPoolObject, "'component-pool' must be an object.");
		}
	}

	return script;
}

ScriptTimeline JsonScriptParser::parseTimeline(const json& timelineJson) {
	static const vector<string> timelineProperties = { "time-scale", "lanes", "loop-lock" };
	ScriptTimeline timeline;

	verifyAllowedProperties(timelineJson, timelineProperties, false, m_context);

	json::const_iterator timeScale = timelineJson.find("time-scale");
	if (timeScale != timelineJson.end()) {
		if (timeScale->is_object()) {
			m_context.location.push_back("time-scale");
			ScriptTimeScale *scriptTimeScale = new ScriptTimeScale(parseTimeScale(*timeScale));
			timeline.timeScale.reset(scriptTimeScale);
			m_context.location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Timeline_TimeScaleObject, "'time-scale' must be an object.");
		}
	}

	json::const_iterator lanes = timelineJson.find("lanes");
	if ((lanes != timelineJson.end()) && (lanes->is_array())) {
		m_context.location.push_back("lanes");

		int count = 0;
		vector<json> laneElements = (*lanes);
		for (const json& lane : laneElements) {
			m_context.location.push_back(to_string(count));
			if (lane.is_object()) {
				timeline.lanes.push_back(parseLane(lane));
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Timeline_LaneObject, "'lanes' elements must be objects.");
			}
			m_context.location.pop_back();
			count++;
		}

		m_context.location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Timeline_LanesMissing, "'lanes' is required and must be an array.");
	}

	timeline.loopLock = false;
	json::const_iterator loopLock = timelineJson.find("loop-lock");
 	if (loopLock != timelineJson.end()) {
 		if (!loopLock->is_boolean()) {
 			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Timeline_LoopLockBoolean, "'loop-lock' must be a boolean.");
 		} else {
 			timeline.loopLock = (*loopLock);
 		}
 	}

	return timeline;
}

ScriptTimeScale JsonScriptParser::parseTimeScale(const json& timeScaleJson) {
	static const vector<string> timeScaleProperties = { "sample-rate", "bpm", "bpb" };
	ScriptTimeScale timeScale;

	verifyAllowedProperties(timeScaleJson, timeScaleProperties, false, m_context);

	json::const_iterator sampleRate = timeScaleJson.find("sample-rate");
	if (sampleRate != timeScaleJson.end()) {
		if ((sampleRate->is_number_unsigned()) && (sampleRate->get<int>() > 0)) {
			timeScale.sampleRate.reset(new int(sampleRate->get<int>()));
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::TimeScale_SampleRateNumber, "'sample-rate' must be a positive integer number.");
		}
	}

	json::const_iterator bpm = timeScaleJson.find("bpm");
	if (bpm != timeScaleJson.end()) {
		if ((bpm->is_number_unsigned()) && (bpm->get<int>() > 0)) {
			timeScale.bpm.reset(new int(bpm->get<int>()));
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::TimeScale_BpmNumber, "'bpm' must be a positive integer number.");
		}
	}

	json::const_iterator bpb = timeScaleJson.find("bpb");
	if (bpb != timeScaleJson.end()) {
		if ((bpb->is_number_unsigned()) && (bpb->get<int>() > 0)) {
			timeScale.bpb.reset(new int(bpb->get<int>()));
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::TimeScale_BpbNumber, "'bpb' must be a positive integer number.");
		}
	}

	if (!(timeScale.sampleRate) && !(timeScale.bpm)) {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::TimeScale_Empty, "One of 'sample-rate' or 'bpm' is required.");
	} else if ((timeScale.bpb) && !(timeScale.bpm)) {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::TimeScale_BpbRequiresBpm, "'bpm' must be set if 'bpb' is set.");
	}

	return timeScale;
}

ScriptLane JsonScriptParser::parseLane(const json& laneJson) {
	static const vector<string> laneProperties = { "auto-start", "loop", "repeat", "start-trigger", "restart-trigger", "stop-trigger", "segments", "disable-ui" };
	ScriptLane lane;

	verifyAllowedProperties(laneJson, laneProperties, false, m_context);

	json::const_iterator autoStart = laneJson.find("auto-start");
	lane.autoStart = true;
	if (autoStart != laneJson.end()) {
		if (autoStart->is_boolean()) {
			lane.autoStart = autoStart->get<bool>();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_AutoStartBoolean, "'auto-start' must be a boolean.");
		}
	}

	json::const_iterator loop = laneJson.find("loop");
	lane.loop = false;
	if (loop != laneJson.end()) {
		if (loop->is_boolean()) {
			lane.loop = loop->get<bool>();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_LoopBoolean, "'loop' must be a boolean.");
		}
	}

	json::const_iterator repeat = laneJson.find("repeat");
	lane.repeat = 0;
	if (repeat != laneJson.end()) {
		if (repeat->is_number_unsigned()) {
			lane.repeat = repeat->get<int>();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_RepeatNumber, "'repeat' must be an unsigned number.");
		}
	}

	json::const_iterator startTrigger = laneJson.find("start-trigger");
	if (startTrigger != laneJson.end()) {
		if (startTrigger->is_string()) {
			lane.startTrigger = *startTrigger;
			if (lane.startTrigger.length() == 0) {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_StartTriggerLength, "'start-trigger' can not be an empty string.");
			}
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_StartTriggerString, "'start-trigger' must be a string.");
		}
	}

	json::const_iterator restartTrigger = laneJson.find("restart-trigger");
	if (restartTrigger != laneJson.end()) {
		if (restartTrigger->is_string()) {
			lane.restartTrigger = *restartTrigger;
			if (lane.restartTrigger.length() == 0) {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_RestartTriggerLength, "'restart-trigger' can not be an empty string.");
			}
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_RestartTriggerString, "'restart-trigger' must be a string.");
		}
	}

	json::const_iterator stopTrigger = laneJson.find("stop-trigger");
	if (stopTrigger != laneJson.end()) {
		if (stopTrigger->is_string()) {
			lane.stopTrigger = *stopTrigger;
			if (lane.stopTrigger.length() == 0) {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_StopTriggerLength, "'stop-trigger' can not be an empty string.");
			}
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_StopTriggerString, "'stop-trigger' must be a string.");
		}
	}

	json::const_iterator segments = laneJson.find("segments");
	if ((segments != laneJson.end()) && (segments->is_array())) {
		m_context.location.push_back("segments");

		int count = 0;
		vector<json> segmentElements = (*segments);
		for (const json& segment : segmentElements) {
			m_context.location.push_back(to_string(count));
			if (segment.is_object()) {
				lane.segments.push_back(parseSegment(segment, true));
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_SegmentObject, "'segments' elements must be Segment objects.");
			}
			m_context.location.pop_back();
			count++;
		}

		m_context.location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_SegmentsMissing, "'segments' is required and must be an array.");
	}

	lane.disableUi = false;
	json::const_iterator disableUi = laneJson.find("disable-ui");
	if (disableUi != laneJson.end()) {
		if (disableUi->is_boolean()) {
			lane.disableUi = disableUi->get<bool>();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Lane_DisableUiBoolean, "'disable-ui' must be a boolean.");
		}
	}

	return lane;
}

ScriptSequence JsonScriptParser::parseSequence(const json& sequenceJson) {
	static const char* cSequenceProperties[] = { "id", "values", "shared", "retrieve-voltage-once" };
	static const vector<string> vSequenceProperties(begin(cSequenceProperties), end(cSequenceProperties));
	ScriptSequence sequence;

	verifyAllowedProperties(sequenceJson, vSequenceProperties, false, m_context);
	populateRef(sequence, sequenceJson, false);

	sequence.shared = true;
	json::const_iterator shared = sequenceJson.find("shared");
	if (shared != sequenceJson.end()) {
		if (shared->is_boolean()) {
			sequence.shared = shared->get<bool>();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Sequence_SharedBoolean, "'shared' must be a boolean.");
		}
	}

	sequence.retrieveVoltageOnce = true;
	json::const_iterator retrieveVoltageOnce = sequenceJson.find("retrieve-voltage-once");
	if (retrieveVoltageOnce != sequenceJson.end()) {
		if (retrieveVoltageOnce->is_boolean()) {
			sequence.retrieveVoltageOnce = retrieveVoltageOnce->get<bool>();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Sequence_RetrieveVoltageOnceBoolean, "'retrieve-voltage-once' must be a boolean.");
		}
	}

	json::const_iterator values = sequenceJson.find("values");
	if (values != sequenceJson.end()) {
		if (values->is_array()) {
			m_context.location.push_back("values");

			int count = 0;
			vector<json> valueElements = (*values);
			for (const json& value : valueElements) {
				sequence.values.push_back(parseValue(value, true, to_string(count), Sequence_ValueObject, "'values' elements must be objects."));
				count++;
			}

			m_context.location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Sequence_ValuesArray, "'values' must be an array.");
		}
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Sequence_ValuesArray, "'values' is required and must be an array.");
	}

	return sequence;
}

void JsonScriptParser::populateRef(ScriptRefObject &refObject, const json& refJson, bool allowRefs) {
	json::const_iterator ref = refJson.find("ref");
	json::const_iterator id = refJson.find("id");

	if (allowRefs) {
		if (ref != refJson.end()) {
			if (ref->is_string()) {
				string refValue = *ref;
				if (refValue.length() > 0) {
					refObject.ref = refValue;
				} else {
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_Length, "'ref' can not be an empty string.");
				}
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_String, "'ref' must be a string.");
			}
		}
		if (id != refJson.end()) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Id_NotAllowed, "'id' is not allowed here.");
		}
	} else {
		if ((id != refJson.end()) && (id->is_string())) {
			string idValue = *id;
			if (idValue.length() > 0) {
				refObject.id = idValue;
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Id_Length, "'id' can not be an empty string.");
			}
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Id_String, "'id' is required and must be a string.");
		}
		if (ref != refJson.end()) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotAllowed, "'ref' is not allowed here.");
		}
	}
}
