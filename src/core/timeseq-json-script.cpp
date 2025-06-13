#include "core/timeseq-json.hpp"
#include "util/notes.hpp"

using namespace std;
using namespace timeseq;


template<size_t N>
bool hasOneOf(const json& json, const char* (&propertyNames)[N]) {
	for (const char* propertyName : propertyNames) {
		if (json.find(propertyName) != json.end()) {
			return true;
		}
	}
	return false;
}

bool verifyAllowedProperties(const json& json, vector<string> propertyNames, bool allowRef, vector<ValidationError> *validationErrors, vector<string> location) {
	uint64_t count = 0;
	std::string unexpectedKeys;

	if (json.is_object()) {
		for (json::const_iterator i = json.begin(); i != json.end(); i++) {
			if ((i.key().substr(0, 2) != "x-") && (std::find(propertyNames.begin(), propertyNames.end(), i.key()) == propertyNames.end())) {
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
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Unknown_Property, "Unknown property encountered: ", unexpectedKeys.c_str(), ".");
	} else if (count > 1) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Unknown_Property, "Unknown properties encountered: ", unexpectedKeys.c_str(), ".");
	}

	return count > 0;
}

JsonScriptParser::~JsonScriptParser() {}

std::shared_ptr<Script> JsonScriptParser::parseScript(const json& scriptJson, vector<ValidationError> *validationErrors, vector<string> location) {
	static const vector<string> scriptProperties = { "type", "version", "timelines", "global-actions", "input-triggers", "component-pool" };
	shared_ptr<Script> script = make_shared<Script>();

	verifyAllowedProperties(scriptJson, scriptProperties, false, validationErrors, location);

	json::const_iterator type = scriptJson.find("type");
	if ((type == scriptJson.end()) || (!type->is_string())) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TypeMissing, "'type' is required and must be a string.");
	} else {
		script->type = *type;
		if (script->type != "not-things_timeseq_script") {
			std::string typeValue = (*type);
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TypeUnsupported, "'type' '", typeValue.c_str(), "' is not supported.");
		}
	}

	json::const_iterator version = scriptJson.find("version");
	if ((version == scriptJson.end()) || (!version->is_string())) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_VersionMissing, "'version' is required and must be a string.");
	}
	else {
		script->version = *version;
		if (script->version != "1.0.0") {
			std::string versionValue = (*version);
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_VersionUnsupported, "'version' '", versionValue.c_str(), "' is an unsupported version. Only version 1.0.0 is currently supported.");
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
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TimelinesMissing, "'timelines' is required and must be an array.");
	}

	json::const_iterator globalActions = scriptJson.find("global-actions");
	if (globalActions != scriptJson.end()) {
		if (globalActions->is_array()) {
			location.push_back("global-actions");

			int count = 0;
			std::vector<json> actionElements = (*globalActions);
			for (const json& action : actionElements) {
				location.push_back(std::to_string(count));
				if (action.is_object()) {
					script->globalActions.push_back(parseAction(action, true, validationErrors, location));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_GlobalActionsObject, "'global-actions' elements must be action objects.");
				}
				location.pop_back();
				count++;
			}

			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_GlobalActionsArray, "'global-actions' must be an array.");
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
					script->inputTriggers.push_back(parseInputTrigger(inputTrigger, validationErrors, location));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_InputTriggerObject, "'input-triggers' elements must be objects.");
				}
				location.pop_back();
				count++;
			}

			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_InputTriggersArray, "'input-triggers' must be an array.");
		}
	}

	json::const_iterator componentPool = scriptJson.find("component-pool");
	if (componentPool != scriptJson.end()) {
		if (componentPool->is_object()) {
			static const vector<string> componentPoolProperties = { "segment-blocks", "segments", "inputs", "outputs", "calcs", "values", "actions", "ifs", "tunings" };
			location.push_back("component-pool");
			vector<string> ids;

			verifyAllowedProperties(*componentPool, componentPoolProperties, false, validationErrors, location);

			json::const_iterator segmentBlocks = componentPool->find("segment-blocks");
			if (segmentBlocks != componentPool->end()) {
				if (segmentBlocks->is_array()) {
					location.push_back("segment-blocks");

					int count = 0;
					std::vector<json> segmentBlockElements = (*segmentBlocks);
					for (const json& segmentBlock : segmentBlockElements) {
						location.push_back(std::to_string(count));
						if (segmentBlock.is_object()) {
							script->segmentBlocks.push_back(parseSegmentBlock(segmentBlock, validationErrors, location));
							if (std::find(ids.begin(), ids.end(), script->segmentBlocks.back().id) != ids.end()) {
								ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_Duplicate, "Id '", script->segmentBlocks.back().id.c_str(), "' has already been used. Ids must be unique within the object type.");
							} else if (script->segmentBlocks.back().id.size() > 0) {
								ids.push_back(script->segmentBlocks.back().id);
							}
						} else {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_SegmentBlockObject, "'segment-blocks' elements must be objects.");
						}
						location.pop_back();
						count++;
					}

					location.pop_back();
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_SegmentBlocksArray, "'segment-blocks' must be an array.");
				}
			}

			json::const_iterator segments = componentPool->find("segments");
			if (segments != componentPool->end()) {
				if (segments->is_array()) {
					location.push_back("segments");

					ids.clear();
					int count = 0;
					std::vector<json> segmentElements = (*segments);
					for (const json& segment : segmentElements) {
						location.push_back(std::to_string(count));
						if (segment.is_object()) {
							script->segments.push_back(parseSegment(segment, false, validationErrors, location));
							if (std::find(ids.begin(), ids.end(), script->segments.back().id) != ids.end()) {
								ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_Duplicate, "Id '", script->segments.back().id.c_str(), "' has already been used. Ids must be unique within the object type.");
							} else if (script->segments.back().id.size() > 0) {
								ids.push_back(script->segments.back().id);
							}
						} else {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_SegmentObject, "'segments' elements must be objects.");
						}
						location.pop_back();
						count++;
					}
					location.pop_back();
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_SegmentsArray, "'segments' must be an array.");
				}
			}

			json::const_iterator inputs = componentPool->find("inputs");
			if (inputs != componentPool->end()) {
				if (inputs->is_array()) {
					location.push_back("inputs");

					ids.clear();
					int count = 0;
					std::vector<json> inputElements = (*inputs);
					for (const json& input : inputElements) {
						location.push_back(std::to_string(count));
						if (input.is_object()) {
							script->inputs.push_back(parseFullInput(input, false, false, validationErrors, location));
							if (std::find(ids.begin(), ids.end(), script->inputs.back().id) != ids.end()) {
								ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_Duplicate, "Id '", script->inputs.back().id.c_str(), "' has already been used. Ids must be unique within the object type.");
							} else if (script->inputs.back().id.size() > 0) {
								ids.push_back(script->inputs.back().id);
							}
						} else {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_InputObject, "'inputs' elements must be objects.");
						}
						location.pop_back();
						count++;
					}

					location.pop_back();
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_InputsArray, "'inputs' must be an array.");
				}
			}

			json::const_iterator outputs = componentPool->find("outputs");
			if (outputs != componentPool->end()) {
				if (outputs->is_array()) {
					location.push_back("outputs");

					ids.clear();
					int count = 0;
					std::vector<json> outputElements = (*outputs);
					for (const json& output : outputElements) {
						location.push_back(std::to_string(count));
						if (output.is_object()) {
							script->outputs.push_back(parseFullOutput(output, false, false, validationErrors, location));
							if (std::find(ids.begin(), ids.end(), script->outputs.back().id) != ids.end()) {
								ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_Duplicate, "Id '", script->outputs.back().id.c_str(), "' has already been used. Ids must be unique within the object type.");
							} else if (script->outputs.back().id.size() > 0) {
								ids.push_back(script->outputs.back().id);
							}
						} else {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_OutputObject, "'outputs' elements must be objects.");
						}
						location.pop_back();
						count++;
					}

					location.pop_back();
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_OutputsArray, "'outputs' must be an array.");
				}
			}

			json::const_iterator calcs = componentPool->find("calcs");
			if (calcs != componentPool->end()) {
				if (calcs->is_array()) {
					location.push_back("calcs");

					ids.clear();
					int count = 0;
					std::vector<json> calcElements = (*calcs);
					for (const json& calc : calcElements) {
						location.push_back(std::to_string(count));
						if (calc.is_object()) {
							script->calcs.push_back(parseCalc(calc, false, validationErrors, location));
							if (std::find(ids.begin(), ids.end(), script->calcs.back().id) != ids.end()) {
								ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_Duplicate, "Id '", script->calcs.back().id.c_str(), "' has already been used. Ids must be unique within the object type.");
							} else if (script->calcs.back().id.size() > 0) {
								ids.push_back(script->calcs.back().id);
							}
						} else {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_CalcObject, "'calcs' elements must be objects.");
						}
						location.pop_back();
						count++;
					}

					location.pop_back();
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_CalcsArray, "'calcs' must be an array.");
				}
			}

			json::const_iterator values = componentPool->find("values");
			if (values != componentPool->end()) {
				if (values->is_array()) {
					location.push_back("values");

					ids.clear();
					int count = 0;
					std::vector<json> valueElements = (*values);
					for (const json& value : valueElements) {
						location.push_back(std::to_string(count));
						if (value.is_object()) {
							script->values.push_back(parseFullValue(value, false, false, validationErrors, location));
							if (std::find(ids.begin(), ids.end(), script->values.back().id) != ids.end()) {
								ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_Duplicate, "Id '", script->values.back().id.c_str(), "' has already been used. Ids must be unique within the object type.");
							} else if (script->values.back().id.size() > 0) {
								ids.push_back(script->values.back().id);
							}
						} else {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_ValueObject, "'values' elements must be objects.");
						}
						location.pop_back();
						count++;
					}

					location.pop_back();
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_ValuesArray, "'values' must be an array.");
				}
			}

			json::const_iterator actions = componentPool->find("actions");
			if (actions != componentPool->end()) {
				if (actions->is_array()) {
					location.push_back("actions");

					ids.clear();
					int count = 0;
					std::vector<json> actionElements = (*actions);
					for (const json& action : actionElements) {
						location.push_back(std::to_string(count));
						if (action.is_object()) {
							script->actions.push_back(parseAction(action, false, validationErrors, location));
							if (std::find(ids.begin(), ids.end(), script->actions.back().id) != ids.end()) {
								ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_Duplicate, "Id '", script->actions.back().id.c_str(), "' has already been used. Ids must be unique within the object type.");
							} else if (script->actions.back().id.size() > 0) {
								ids.push_back(script->actions.back().id);
							}
						} else {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_ActionObject, "'actions' elements must be objects.");
						}
						location.pop_back();
						count++;
					}

					location.pop_back();
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_ActionsArray, "'actions' must be an array.");
				}
			}

			json::const_iterator ifs = componentPool->find("ifs");
			if (ifs != componentPool->end()) {
				if (ifs->is_array()) {
					location.push_back("ifs");

					ids.clear();
					int count = 0;
					std::vector<json> ifElements = (*ifs);
					for (const json& ifObj : ifElements) {
						location.push_back(std::to_string(count));
						if (ifObj.is_object()) {
							script->ifs.push_back(parseIf(ifObj, false, validationErrors, location));
							if (std::find(ids.begin(), ids.end(), script->ifs.back().id) != ids.end()) {
								ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_Duplicate, "Id '", script->ifs.back().id.c_str(), "' has already been used. Ids must be unique within the object type.");
							} else if (script->ifs.back().id.size() > 0) {
								ids.push_back(script->ifs.back().id);
							}
						} else {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_IfObject, "'ifs' elements must be objects.");
						}
						location.pop_back();
						count++;
					}

					location.pop_back();
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_IfsArray, "'ifs' must be an array.");
				}
			}

			json::const_iterator tunings = componentPool->find("tunings");
			if (tunings != componentPool->end()) {
				if (tunings->is_array()) {
					location.push_back("tunings");

					ids.clear();
					int count = 0;
					std::vector<json> tuningElements = (*tunings);
					for (const json& tuningObj : tuningElements) {
						location.push_back(std::to_string(count));
						if (tuningObj.is_object()) {
							script->tunings.push_back(parseTuning(tuningObj, validationErrors, location));
							if (std::find(ids.begin(), ids.end(), script->tunings.back().id) != ids.end()) {
								ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Id_Duplicate, "Id '", script->tunings.back().id.c_str(), "' has already been used. Ids must be unique within the object type.");
							} else if (script->tunings.back().id.size() > 0) {
								ids.push_back(script->tunings.back().id);
							}
						} else {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TuningObject, "'tunings' elements must be objects.");
						}
						location.pop_back();
						count++;
					}

					location.pop_back();
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TuningsArray, "'tunings' must be an array.");
				}
			}

			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_ComponentPoolObject, "'component-pool' must be an object.");
		}
	}

	return script;
}

ScriptTimeline JsonScriptParser::parseTimeline(const json& timelineJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> timelineProperties = { "time-scale", "lanes", "loop-lock" };
	ScriptTimeline timeline;

	verifyAllowedProperties(timelineJson, timelineProperties, false, validationErrors, location);

	json::const_iterator timeScale = timelineJson.find("time-scale");
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

	timeline.loopLock = false;
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

ScriptTimeScale JsonScriptParser::parseTimeScale(const json& timeScaleJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> timeScaleProperties = { "sample-rate", "bpm", "bpb" };
	ScriptTimeScale timeScale;

	verifyAllowedProperties(timeScaleJson, timeScaleProperties, false, validationErrors, location);

	json::const_iterator sampleRate = timeScaleJson.find("sample-rate");
	if (sampleRate != timeScaleJson.end()) {
		if ((sampleRate->is_number_unsigned()) && (sampleRate->get<int>() > 0)) {
			timeScale.sampleRate.reset(new int(sampleRate->get<int>()));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_SampleRateNumber, "'sample-rate' must be a positive integer number.");
		}
	}

	json::const_iterator bpm = timeScaleJson.find("bpm");
	if (bpm != timeScaleJson.end()) {
		if ((bpm->is_number_unsigned()) && (bpm->get<int>() > 0)) {
			timeScale.bpm.reset(new int(bpm->get<int>()));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_BpmNumber, "'bpm' must be a positive integer number.");
		}
	}

	json::const_iterator bpb = timeScaleJson.find("bpb");
	if (bpb != timeScaleJson.end()) {
		if ((bpb->is_number_unsigned()) && (bpb->get<int>() > 0)) {
			timeScale.bpb.reset(new int(bpb->get<int>()));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_BpbNumber, "'bpb' must be a positive integer number.");
		}
	}

	if (!(timeScale.sampleRate) && !(timeScale.bpm)) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_Empty, "One of 'sample-rate' or 'bpm' is required.");
	} else if ((timeScale.bpb) && !(timeScale.bpm)) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_BpbRequiresBpm, "'bpm' must be set if 'bpb' is set.");
	}

	return timeScale;
}

ScriptLane JsonScriptParser::parseLane(const json& laneJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> laneProperties = { "auto-start", "loop", "repeat", "start-trigger", "restart-trigger", "stop-trigger", "segments", "disable-ui" };
	ScriptLane lane;

	verifyAllowedProperties(laneJson, laneProperties, false, validationErrors, location);

	json::const_iterator autoStart = laneJson.find("auto-start");
	lane.autoStart = true;
	if (autoStart != laneJson.end()) {
		if (autoStart->is_boolean()) {
			lane.autoStart = autoStart->get<bool>();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_AutoStartBoolean, "auto-start must be a boolean.");
		}
	}

	json::const_iterator loop = laneJson.find("loop");
	lane.loop = false;
	if (loop != laneJson.end()) {
		if (loop->is_boolean()) {
			lane.loop = loop->get<bool>();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_LoopBoolean, "loop must be a boolean.");
		}
	}

	json::const_iterator repeat = laneJson.find("repeat");
	lane.repeat = 0;
	if (repeat != laneJson.end()) {
		if (repeat->is_number_unsigned()) {
			lane.repeat = repeat->get<int>();
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

	json::const_iterator restartTrigger = laneJson.find("restart-trigger");
	if (restartTrigger != laneJson.end()) {
		if (restartTrigger->is_string()) {
			lane.restartTrigger = *restartTrigger;
			if (lane.restartTrigger.length() == 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_RestartTriggerLength, "restart-trigger can not be an empty string.");
			}
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_RestartTriggerString, "restart-trigger must be a string.");
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
				lane.segments.push_back(parseSegment(segment, true, validationErrors, location));
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_SegmentObject, "'segments' elements must be Segment objects.");
			}
			location.pop_back();
			count++;
		}

		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_SegmentsMissing, "segments is required and must be an array.");
	}

	lane.disableUi = false;
	json::const_iterator disableUi = laneJson.find("disable-ui");
	if (disableUi != laneJson.end()) {
		if (disableUi->is_boolean()) {
			lane.disableUi = disableUi->get<bool>();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Lane_DisableUiBoolean, "'disable-ui' must be a boolean.");
		}
	}

	return lane;
}

ScriptSegment JsonScriptParser::parseSegment(const json& segmentJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const char* cSegmentProperties[] = { "duration", "actions", "disable-ui", "segment-block" };
	static const vector<string> vSegmentProperties(begin(cSegmentProperties), end(cSegmentProperties));
	ScriptSegment segment;

	verifyAllowedProperties(segmentJson, vSegmentProperties, true, validationErrors, location);

	populateRef(segment, segmentJson, allowRefs, validationErrors, location);
	if (segment.ref.length() > 0) {
		if (hasOneOf(segmentJson, cSegmentProperties)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Segment_RefOrInstance, "A ref segment can not be combined other non-ref segment properties.");
		}
	} else {
		static const char* segmentBlockProperty[] = { "segment-block" };
		static const char* nonSegmentBlockProperties[] = { "duration", "disable-ui" };
		if (!hasOneOf(segmentJson, segmentBlockProperty)) {
			json::const_iterator duration = segmentJson.find("duration");
			if ((duration != segmentJson.end()) && (duration->is_object())) {
				location.push_back("duration");
				segment.duration = parseDuration(*duration, validationErrors, location);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Segment_DurationObject, "duration is required and must be an object.");
			}

			segment.disableUi = false;
			json::const_iterator disableUi = segmentJson.find("disable-ui");
			if (disableUi != segmentJson.end()) {
				if (disableUi->is_boolean()) {
					segment.disableUi = disableUi->get<bool>();
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Segment_DisableUiBoolean, "'disable-ui' must be a boolean.");
				}
			}
		} else if (!hasOneOf(segmentJson, nonSegmentBlockProperties)) {
			json::const_iterator segmentBlock = segmentJson.find("segment-block");
			if (segmentBlock != segmentJson.end()) {
				if (segmentBlock->is_string()) {
					string segmentBlockRef = segmentBlock->get<string>();
					if (segmentBlockRef.length() > 0) {
						segment.segmentBlock.reset(new ScriptSegmentBlock());
						segment.segmentBlock->ref = segmentBlockRef;
					} else {
						ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Segment_SegmentBlockLength, "'segment-block' must be a non-empty string");
					}
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Segment_SegmentBlockString, "'segment-block' must be a non-empty string");
				}
			}
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Segment_BlockOrSegment, "A segment must either be a single segment with a 'duration' and 'actions', or a segment block with a 'segment-block' reference and optional 'actions', but not both.");
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

ScriptSegmentBlock JsonScriptParser::parseSegmentBlock(const json& segmentBlockJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> segmentBlockProperties = { "repeat", "segments" };
	ScriptSegmentBlock segmentBlock;

	verifyAllowedProperties(segmentBlockJson, segmentBlockProperties, true, validationErrors, location); // Refs aren't really allowed, but that will be caught by the populateRef method.

	populateRef(segmentBlock, segmentBlockJson, false, validationErrors, location);

	json::const_iterator repeat = segmentBlockJson.find("repeat");
	if (repeat != segmentBlockJson.end()) {
		if ((repeat->is_number_unsigned()) && (repeat->is_number_unsigned() > 0)) {
			segmentBlock.repeat.reset(new int(repeat->get<int>()));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SegmentBlock_RepeatNumber, "'repeat' must be a positive number.");
		}
	}
	json::const_iterator segments = segmentBlockJson.find("segments");
	if ((segments != segmentBlockJson.end()) && (segments->is_array())) {
		location.push_back("segments");

		int count = 0;
		std::vector<json> segmentElements = (*segments);
		for (const json& segment : segmentElements) {
			location.push_back(std::to_string(count));
			if (segment.is_object()) {
				segmentBlock.segments.push_back(parseSegment(segment, true, validationErrors, location));
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SegmentBlock_SegmentObject, "'segments' elements must be Segment objects.");
			}
			location.pop_back();
			count++;
		}

		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SegmentBlock_SegmentsArray, "'segments' is required and must be an array.");
	}

	return segmentBlock;
}

ScriptDuration JsonScriptParser::parseDuration(const json& durationJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> durationProperties = { "samples", "millis", "bars", "beats", "hz" };
	ScriptDuration duration;
	int durationCount = 0;

	verifyAllowedProperties(durationJson, durationProperties, false, validationErrors, location);

	json::const_iterator samples = durationJson.find("samples");
	if (samples != durationJson.end()) {
		if ((samples->is_number_unsigned()) && (samples->get<uint64_t>() > 0)) {
			duration.samples.reset(new uint64_t(samples->get<uint64_t>()));
			durationCount++;
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_SamplesNumber, "samples must be a positive integer number.");
			durationCount++; // It's not configured correctly, but we did encounter a duration entry
		}
	}

	json::const_iterator millis = durationJson.find("millis");
	if (millis != durationJson.end()) {
		if ((millis->is_number()) && (millis->get<float>() > 0)) {
			duration.millis.reset(new float(millis->get<float>()));
			durationCount++;
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_MillisNumber, "millis must be a positive decimal number.");
			durationCount++; // It's not configured correctly, but we did encounter a duration entry
		}
	}

	json::const_iterator bars = durationJson.find("bars");
	if (bars != durationJson.end()) {
		if ((bars->is_number_unsigned()) && (bars->get<uint64_t>() > 0)) {
			duration.bars.reset(new uint64_t(bars->get<uint64_t>()));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_BarsNumber, "bars must be a positive integer number.");
		}
	}

	json::const_iterator beats = durationJson.find("beats");
	if (beats != durationJson.end()) {
		if ((beats->is_number()) && (beats->get<float>() >= 0)) {
			duration.beats.reset(new float(beats->get<float>()));
			durationCount++;
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_BeatsNumber, "beats must be a positive decimal number.");
			durationCount++; // It's not configured correctly, but we did encounter a duration entry
		}
	}

	json::const_iterator hz = durationJson.find("hz");
	if (hz != durationJson.end()) {
		if ((hz->is_number()) && (hz->get<float>() > 0)) {
			duration.hz.reset(new float(hz->get<float>()));
			durationCount++;
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_HzNumber, "'hz' must be a positive decimal number.");
			durationCount++; // It's not configured correctly, but we did encounter a duration entry
		}
	}

	if (durationCount == 0) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_NoSamplesOrMillisOrBeatsOrHz, "either 'samples', 'millis', 'beats' or 'hz' must be used.");
	} else if (durationCount > 1) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_EitherSamplesOrMillisOrBeatsOrHz, "only one of 'samples', 'millis', 'beats' or 'hz' can be used at a time.");
	} else if (duration.bars && !duration.beats) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_BarsRequiresBeats, "'bars' can not be used without 'beats'.");
	} else if ((!duration.bars) && (duration.beats) && (*duration.beats.get() == 0)) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Duration_BeatsNotZero, "'beats' can not be 0 unless 'bars' is also used.");
	}

	return duration;
}

ScriptAction JsonScriptParser::parseAction(const json& actionJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const char* cActionProperties[] = { "timing", "set-value", "set-variable", "set-polyphony", "set-label", "assert", "trigger", "start-value", "end-value", "ease-factor", "ease-algorithm", "output", "variable", "if", "gate-high-ratio" };
	static const vector<string> vActionProperties(begin(cActionProperties), end(cActionProperties));
	ScriptAction action;

	verifyAllowedProperties(actionJson, vActionProperties, true, validationErrors, location);

	populateRef(action, actionJson, allowRefs, validationErrors, location);
	if (action.ref.length() > 0) {
		if (hasOneOf(actionJson, cActionProperties)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_RefOrInstance, "A ref action can not be combined other non-ref action properties.");
		}
	} else {
		int actionCount = 0;

		json::const_iterator timing = actionJson.find("timing");
		if ((timing != actionJson.end()) && (timing->is_string())) {
			if (*timing == "start") {
				action.timing = ScriptAction::ActionTiming::START;
			} else if (*timing == "end") {
				action.timing = ScriptAction::ActionTiming::END;
			} else if (*timing == "glide") {
				action.timing = ScriptAction::ActionTiming::GLIDE;
			} else if (*timing == "gate") {
				action.timing = ScriptAction::ActionTiming::GATE;
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_TimingEnum, "timing must be either 'start', 'end', 'glide' or 'gate'.");
			}
		} else {
			action.timing = ScriptAction::ActionTiming::START;
		}

		json::const_iterator setValue = actionJson.find("set-value");
		if (setValue != actionJson.end()) {
			actionCount++;
			if (setValue->is_object()) {
				location.push_back("set-value");
				ScriptSetValue *scriptSetValue = new ScriptSetValue(parseSetValue(*setValue, validationErrors, location));
				action.setValue.reset(scriptSetValue);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_SetValueObject, "'set-value' must be an object.");
			}
		}

		json::const_iterator setVariable = actionJson.find("set-variable");
		if (setVariable != actionJson.end()) {
			actionCount++;
			if (setVariable->is_object()) {
				location.push_back("set-variable");
				ScriptSetVariable *scriptSetVariable = new ScriptSetVariable(parseSetVariable(*setVariable, validationErrors, location));
				action.setVariable.reset(scriptSetVariable);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_SetVariableObject, "'set-variable' must be an object.");
			}
		}

		json::const_iterator setPolyphony = actionJson.find("set-polyphony");
		if (setPolyphony != actionJson.end()) {
			actionCount++;
			if (setPolyphony->is_object()) {
				location.push_back("set-polyphony");
				ScriptSetPolyphony *scriptSetPolyphony = new ScriptSetPolyphony(parseSetPolyphony(*setPolyphony, validationErrors, location));
				action.setPolyphony.reset(scriptSetPolyphony);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_SetPolyphonyObject, "'set-polyphony' must be an object.");
			}
		}

		json::const_iterator setLabel = actionJson.find("set-label");
		if (setLabel != actionJson.end()) {
			actionCount++;
			if (setLabel->is_object()) {
				location.push_back("set-label");
				ScriptSetLabel *scriptSetLabel = new ScriptSetLabel(parseSetLabel(*setLabel, validationErrors, location));
				action.setLabel.reset(scriptSetLabel);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_SetLabelObject, "'set-label' must be an object.");
			}
		}

		json::const_iterator assert = actionJson.find("assert");
		if (assert != actionJson.end()) {
			actionCount++;
			if (assert->is_object()) {
				location.push_back("assert");
				ScriptAssert *scriptAssert = new ScriptAssert(parseAssert(*assert, validationErrors, location));
				action.assert.reset(scriptAssert);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_AssertObject, "'assert' must be an object.");
			}
		}

		json::const_iterator trigger = actionJson.find("trigger");
		if (trigger != actionJson.end()) {
			actionCount++;
			if (trigger->is_string()) {
				action.trigger = *trigger;
				if (action.trigger.size() == 0) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_TriggerLength, "'trigger' can not be an empty string.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_TriggerString, "'trigger' must be a string.");
			}
		}

		json::const_iterator startValue = actionJson.find("start-value");
		if (startValue != actionJson.end()) {
			ScriptValue *scriptValue = new ScriptValue(parseValue(*startValue, true, validationErrors, location, "start-value", ValidationErrorCode::Action_StartValueObject, "'start-value' must be an object."));
			action.startValue.reset(scriptValue);
		}

		json::const_iterator endValue = actionJson.find("end-value");
		if (endValue != actionJson.end()) {
			ScriptValue *scriptValue = new ScriptValue(parseValue(*endValue, true, validationErrors, location, "end-value", ValidationErrorCode::Action_EndValueObject, "'end-value' must be an object."));
			action.endValue.reset(scriptValue);
		}

		json::const_iterator easeFactor = actionJson.find("ease-factor");
		if (easeFactor != actionJson.end()) {
			if (easeFactor->is_number()) {
				action.easeFactor.reset(new float(easeFactor->get<float>()));
				if ((*action.easeFactor.get() < -5.0) || (*action.easeFactor.get() > 5.0)) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_EaseFactorRange, "'ease-factor' must be a number between -5.0 and 5.0.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_EaseFactorFloat, "'ease-factor' must be a number between -5.0 and 5.0.");
			}
		}

		json::const_iterator easeAlgorithm = actionJson.find("ease-algorithm");
		if (easeAlgorithm != actionJson.end()) {
			if (easeAlgorithm->is_string()) {
				if (easeAlgorithm->get<string>() == "pow") {
					action.easeAlgorithm.reset(new ScriptAction::EaseAlgorithm(ScriptAction::EaseAlgorithm::POW));
				} else if (easeAlgorithm->get<string>() == "sig") {
					action.easeAlgorithm.reset(new ScriptAction::EaseAlgorithm(ScriptAction::EaseAlgorithm::SIG));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_EaseAlgorithmEnum, "'ease-algorithm' must be either the string 'pow' or 'sig'.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_EaseAlgorithmEnum, "'ease-algorithm' must be either the string 'pow' or 'sig'.");
			}
		}

		json::const_iterator gateHighRatio = actionJson.find("gate-high-ratio");
		if (gateHighRatio != actionJson.end()) {
			if (gateHighRatio->is_number()) {
				action.gateHighRatio.reset(new float(gateHighRatio->get<float>()));
				if ((*action.gateHighRatio.get() < 0.f) || (*action.gateHighRatio.get() > 1.f)) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_GateHighRatioRange, "'gate-high-ratio' must be a number between 0.0 and 1.0.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_GateHighRatioFloat, "'gate-high-ratio' must be a number between 0.0 and 1.0.");
			}
		}

		json::const_iterator output = actionJson.find("output");
		if (output != actionJson.end()) {
			ScriptOutput *scriptOutput = new ScriptOutput(parseOutput(*output, true, validationErrors, location, "output", ValidationErrorCode::Action_OutputObject, "'output' must be an object."));
			action.output.reset(scriptOutput);
		}

		json::const_iterator variable = actionJson.find("variable");
		if (variable != actionJson.end()) {
			if (variable->is_string()) {
				action.variable = *variable;
				if (action.variable.size() == 0) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_VariableLength, "'variable' can not be an empty string.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_VariableString, "'variable' must be a string.");
			}
		}

		json::const_iterator ifCondition = actionJson.find("if");
		if (ifCondition != actionJson.end()) {
			if (ifCondition->is_object()) {
				location.push_back("if");
				ScriptIf *scriptIf = new ScriptIf(parseIf(*ifCondition, true, validationErrors, location));
				action.condition.reset(scriptIf);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_IfObject, "'if' must be an object.");
			}
		}

		if (action.timing == ScriptAction::ActionTiming::GLIDE) {
			if ((action.setValue) || (action.setVariable) || (action.setPolyphony) || (action.setLabel) || (action.assert) || (action.trigger.size() > 0) || (action.gateHighRatio)) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_NonGlideProperties, "'set-value', 'set-variable', 'set-polyphony', 'set-label', 'assert', 'trigger' and 'gate-high-ratio' can not be used in combination with 'GLIDE' timing.");
			}
			if ((!action.startValue) || (!action.endValue)) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_MissingGlideValues, "'start-value' and 'end-value' must be present when 'GLIDE' timing is used.");
			}
			if ((!action.output) && (action.variable.length() == 0)) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_MissingGlideActions, "Either 'output' or 'variable' must be present when 'GLIDE' timing is used.");
			}
			if ((action.output) && (action.variable.length() > 0)) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_TooManyGlideActions, "Only one of 'output' and 'variable' can be present when 'GLIDE' timing is used.");
			}
		} else if (action.timing == ScriptAction::ActionTiming::GATE) {
			if ((action.setValue) || (action.setVariable) || (action.setPolyphony) || (action.setLabel) || (action.assert) || (action.trigger.size() > 0) || (action.startValue) || (action.endValue) || (action.easeFactor) || (action.easeAlgorithm)) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_NonGateProperties, "'set-value', 'set-variable', 'set-polyphony', 'set-label', 'assert', 'trigger', 'start-value', 'end-value', 'ease-factory' and 'ease-algorithm' can not be used in combination with 'GATE' timing.");
			}
			if (!action.output) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_GateOutput, "'output' must be present when 'GATE' timing is used.");
			}
			if (action.variable.length() > 0) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_NonGateProperties, "'variable' can only be used in combination with 'GLIDE' timing.");
			}
		} else {
			if ((action.startValue) || (action.endValue) || (action.easeFactor) || (action.easeAlgorithm) || (action.gateHighRatio)) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "'start-value', 'end-value', 'ease-factory' 'ease-algorithm' and 'gate-high-ratio' can only be used in combination with 'GLIDE' timing.");
			}
			if ((action.output) || (action.variable.length() > 0)) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "'output' and 'variable' can only be used in combination with 'GLIDE' timing.");
			}
			if ((!action.setValue) && (!action.setVariable) && (!action.setPolyphony) && (!action.setLabel) && (!action.assert) && (action.trigger.size() == 0)) {
				std::string timingStr = timing != actionJson.end() ? *timing : "start";
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_MissingNonGlideProperties, "'set-value', 'set-variable', 'set-polyphony', 'set-label', 'assert' or 'trigger' must be present for '", timingStr.c_str(), "' timing.");
			}
			if (actionCount > 1) {
				std::string timingStr = timing != actionJson.end() ? *timing : "start";
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Action_TooManyNonGlideProperties, "Only one of 'set-value', 'set-variable', 'set-polyphony', 'set-label', 'assert' or 'trigger' can be used in the same '", timingStr.c_str(), "' action.");
			}
		}
	}

	return action;
}

ScriptIf JsonScriptParser::parseIf(const json& ifJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const char* cIfProperties[] = { "eq", "ne", "lt", "lte", "gt", "gte", "and", "or", "tolerance" };
	static const vector<string> vIfProperties(begin(cIfProperties), end(cIfProperties));
	ScriptIf scriptIf;

	populateRef(scriptIf, ifJson, allowRefs, validationErrors, location);
	if (scriptIf.ref.length() > 0) {
		if (hasOneOf(ifJson, cIfProperties)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_RefOrinstance, "A ref if can not be combined other non-ref if properties.");
		}
	} else {
		int operatorCount = 0;

		verifyAllowedProperties(ifJson, vIfProperties, true, validationErrors, location);

		json::const_iterator eqValue = ifJson.find("eq");
		if (eqValue != ifJson.end()) {
			operatorCount++;
			if (eqValue->is_array()) {
				location.push_back("eq");
				scriptIf.ifOperator = ScriptIf::IfOperator::EQ;
				scriptIf.values.reset(new std::pair<ScriptValue, ScriptValue>(parseIfValues("eq", *eqValue, validationErrors, location)));
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_EqArray, "'eq' must be an array.");
			}
		}

		json::const_iterator neValue = ifJson.find("ne");
		if (neValue != ifJson.end()) {
			operatorCount++;
			if (neValue->is_array()) {
				location.push_back("ne");
				scriptIf.ifOperator = ScriptIf::IfOperator::NE;
				scriptIf.values.reset(new std::pair<ScriptValue, ScriptValue>(parseIfValues("ne", *neValue, validationErrors, location)));
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_NeArray, "'ne' must be an array.");
			}
		}

		json::const_iterator ltValue = ifJson.find("lt");
		if (ltValue != ifJson.end()) {
			operatorCount++;
			if (ltValue->is_array()) {
				location.push_back("lt");
				scriptIf.ifOperator = ScriptIf::IfOperator::LT;
				scriptIf.values.reset(new std::pair<ScriptValue, ScriptValue>(parseIfValues("lt", *ltValue, validationErrors, location)));
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_LtArray, "'lt' must be an array.");
			}
		}

		json::const_iterator lteValue = ifJson.find("lte");
		if (lteValue != ifJson.end()) {
			operatorCount++;
			if (lteValue->is_array()) {
				location.push_back("lte");
				scriptIf.ifOperator = ScriptIf::IfOperator::LTE;
				scriptIf.values.reset(new std::pair<ScriptValue, ScriptValue>(parseIfValues("lte", *lteValue, validationErrors, location)));
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_LteArray, "'lte' must be an array.");
			}
		}

		json::const_iterator gtValue = ifJson.find("gt");
		if (gtValue != ifJson.end()) {
			operatorCount++;
			if (gtValue->is_array()) {
				location.push_back("gt");
				scriptIf.ifOperator = ScriptIf::IfOperator::GT;
				scriptIf.values.reset(new std::pair<ScriptValue, ScriptValue>(parseIfValues("gt", *gtValue, validationErrors, location)));
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_GtArray, "'gt' must be an array.");
			}
		}

		json::const_iterator gteValue = ifJson.find("gte");
		if (gteValue != ifJson.end()) {
			operatorCount++;
			if (gteValue->is_array()) {
				location.push_back("gte");
				scriptIf.ifOperator = ScriptIf::IfOperator::GTE;
				scriptIf.values.reset(new std::pair<ScriptValue, ScriptValue>(parseIfValues("gte", *gteValue, validationErrors, location)));
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_GteArray, "'gte' must be an array.");
			}
		}

		json::const_iterator andValue = ifJson.find("and");
		if (andValue != ifJson.end()) {
			operatorCount++;
			if (andValue->is_array()) {
				location.push_back("and");
				scriptIf.ifOperator = ScriptIf::IfOperator::AND;
				scriptIf.ifs = parseIfIfs("and", *andValue, validationErrors, location);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_AndArray, "'and' must be an array.");
			}
		}

		json::const_iterator orValue = ifJson.find("or");
		if (orValue != ifJson.end()) {
			operatorCount++;
			if (orValue->is_array()) {
				location.push_back("or");
				scriptIf.ifOperator = ScriptIf::IfOperator::OR;
				scriptIf.ifs = parseIfIfs("or", *orValue, validationErrors, location);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_OrArray, "'or' must be an array.");
			}
		}

		json::const_iterator toleranceValue = ifJson.find("tolerance");
		if (toleranceValue != ifJson.end()) {
			if (toleranceValue->is_number()) {
				if (toleranceValue->get<float>() >= 0.f) {
					scriptIf.tolerance.reset(new float(toleranceValue->get<float>()));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_ToleranceNumber, "'tolerance' must be a positive number.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_ToleranceNumber, "'tolerance' must be a positive number.");
			}
		}

		if (operatorCount == 0) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_NoOperation, "One of 'eq', 'ne', 'lt', 'lte', 'gt', 'gte', 'and' or 'or' is required.");
		} else if (operatorCount > 1) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_MultpleOperations, "Only one of 'eq', 'ne', 'lt', 'lte', 'gt', 'gte', 'and' or 'or' is allowed.");
		}
	}

	return scriptIf;
}

std::pair<ScriptValue, ScriptValue> JsonScriptParser::parseIfValues(std::string ifOperator, const json& valuesJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	std::pair<ScriptValue, ScriptValue> valuePair;

	std::vector<json> valueElements = valuesJson.get<std::vector<json>>();
	if (valueElements.size() == 2) {
		valuePair.first = parseValue(valueElements[0], true, validationErrors, location, "0", ValidationErrorCode::If_ValueObject, "'" + ifOperator + "' children must be value objects.");
		valuePair.second = parseValue(valueElements[1], true, validationErrors, location, "1", ValidationErrorCode::If_ValueObject, "'" + ifOperator + "' children must be value objects.");
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_TwoValues, "Exactly two value items are expected in the '", ifOperator.c_str(), "' array");
	}

	return valuePair;
}

std::unique_ptr<std::pair<ScriptIf, ScriptIf>> JsonScriptParser::parseIfIfs(std::string ifOperator, const json& ifsJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	std::unique_ptr<std::pair<ScriptIf, ScriptIf>> ifPair;

	std::vector<json> ifElements = ifsJson.get<std::vector<json>>();
	if (ifElements.size() == 2) {
		ifPair.reset(new std::pair<ScriptIf, ScriptIf>());
		location.push_back("0");
		ifPair->first = parseIf(ifElements[0], true, validationErrors, location);
		location.pop_back();
		location.push_back("1");
		ifPair->second = parseIf(ifElements[1], true, validationErrors, location);
		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::If_TwoValues, "Exactly two if items are expected in the '", ifOperator.c_str(), "' array");
	}

	return ifPair;
}

ScriptSetValue JsonScriptParser::parseSetValue(const json& setValueJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> setValueProperties = { "output", "value" };
	ScriptSetValue setValue;

	verifyAllowedProperties(setValueJson, setValueProperties, false, validationErrors, location);

	json::const_iterator output = setValueJson.find("output");
	if (output != setValueJson.end()) {
		setValue.output = parseOutput(*output, true, validationErrors, location, "output", ValidationErrorCode::SetValue_OutputObject, "'output' is required and must be an object.");
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetValue_OutputObject, "'output' is required and must be a output object.");
	}

	json::const_iterator value = setValueJson.find("value");
	if (value != setValueJson.end()) {
		setValue.value = parseValue(*value, true, validationErrors, location, "value", ValidationErrorCode::SetValue_ValueObject, "'value' is required and must be an object.");
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetValue_ValueObject, "'value' is required and must be a value object.");
	}

	return setValue;
}

ScriptSetVariable JsonScriptParser::parseSetVariable(const json& setVariableJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> setVariableProperties  = { "name", "value" };
	ScriptSetVariable setVariable;

	verifyAllowedProperties(setVariableJson, setVariableProperties, false, validationErrors, location);

	json::const_iterator name = setVariableJson.find("name");
	if ((name != setVariableJson.end()) && (name->is_string())) {
		setVariable.name = *name;
		if (setVariable.name.length() == 0) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetVariable_NameLength, "'name' must be a non-empty string.");
		}
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetVariable_NameString, "'name' is required and must be a non-empty string.");
	}

	json::const_iterator value = setVariableJson.find("value");
	if (value != setVariableJson.end()) {
		setVariable.value = parseValue(*value, true, validationErrors, location, "value", ValidationErrorCode::SetVariable_ValueObject, "'value' is required and must be an object.");
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetVariable_ValueObject, "'value' is required and must be a value object.");
	}

	return setVariable;
}

ScriptSetPolyphony JsonScriptParser::parseSetPolyphony(const json& setPolyphonyJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> setPolyphonyProperties = { "index", "channels" };
	ScriptSetPolyphony setPolyphony;

	verifyAllowedProperties(setPolyphonyJson, setPolyphonyProperties, false, validationErrors, location);

	json::const_iterator index = setPolyphonyJson.find("index");
	if ((index != setPolyphonyJson.end()) && (index->is_number_unsigned())) {
		setPolyphony.index = index->get<int>();
		if ((setPolyphony.index < 1) || (setPolyphony.index > 8)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetPolyphony_IndexRange, "'index' must be a number between 1 and 8.");
		}
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetPolyphony_IndexNumber, "'index' is required and must be a number between 1 and 8.");
	}

	json::const_iterator channels = setPolyphonyJson.find("channels");
	if ((channels != setPolyphonyJson.end()) && (channels->is_number_unsigned())) {
		setPolyphony.channels = channels->get<int>();
		if ((setPolyphony.channels < 1) || (setPolyphony.channels > 16)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetPolyphony_ChannelsRange, "'channels' must be a number between 1 and 16.");
		}
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetPolyphony_ChannelsNumber, "'channels' is required and must be a number between 1 and 16.");
	}

	return setPolyphony;
}

ScriptSetLabel JsonScriptParser::parseSetLabel(const json& setLabelJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> setLabelProperties = { "index", "label" };
	ScriptSetLabel setLabel;

	verifyAllowedProperties(setLabelJson, setLabelProperties, false, validationErrors, location);

	json::const_iterator index = setLabelJson.find("index");
	if ((index != setLabelJson.end()) && (index->is_number_unsigned())) {
		setLabel.index = index->get<int>();
		if ((setLabel.index < 1) || (setLabel.index > 8)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetLabel_IndexRange, "'index' must be a number between 1 and 8.");
		}
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetLabel_IndexNumber, "'index' is required and must be a number between 1 and 8.");
	}

	json::const_iterator label = setLabelJson.find("label");
	if ((label != setLabelJson.end()) && (label->is_string())) {
		setLabel.label = label->get<string>();
		if (setLabel.label.size() == 0) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetLabel_LabelLength, "'label' can not be an empty string.");
		}
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::SetLabel_LabelString, "'label' must be a non-empty string.");
	}

	return setLabel;
}

ScriptAssert JsonScriptParser::parseAssert(const json& assertJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> assertProperties = { "name", "expect", "stop-on-fail" };
	ScriptAssert scriptAssert;

	verifyAllowedProperties(assertJson, assertProperties, false, validationErrors, location);

	json::const_iterator name = assertJson.find("name");
	if (name != assertJson.end()) {
		if (name->is_string()) {
			scriptAssert.name = name->get<string>();
			if (scriptAssert.name.length() < 1) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Assert_NameLength, "'name' can not be an empty string.");
			}
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Assert_NameString, "'name' is required and must be a non-empty string.");
		}
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Assert_NameString, "'name' must be a non-empty string.");
	}

	json::const_iterator expect = assertJson.find("expect");
	if (expect != assertJson.end()) {
		if (expect->is_object()) {
			location.push_back("expect");
			scriptAssert.expect = parseIf(*expect, true, validationErrors, location);
			location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Assert_ExpectObject, "'expect' must be an object.");
		}
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Assert_ExpectObject, "'expect' is required and must be an object.");
	}

	json::const_iterator stopOnFail = assertJson.find("stop-on-fail");
	scriptAssert.stopOnFail = true;
	if (stopOnFail != assertJson.end()) {
		if (stopOnFail->is_boolean()) {
			scriptAssert.stopOnFail = stopOnFail->get<bool>();
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Assert_StopOnFailBool, "'stop-on-fail' must be a boolean.");
		}
	}

	return scriptAssert;
}

ScriptValue JsonScriptParser::parseValue(const json& valueJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage) {
	ScriptValue scriptValue;

	if (valueJson.is_object()) {
		location.push_back(subLocation);
		scriptValue = parseFullValue(valueJson, allowRefs, false, validationErrors, location);
		location.pop_back();
	} else if (valueJson.is_number()) {
		json fullValueJson = { { "voltage", valueJson } };
		location.push_back(subLocation);
		scriptValue = parseFullValue(fullValueJson, allowRefs, true, validationErrors, location);
		location.pop_back();
	} else if (valueJson.is_string()) {
		json fullValueJson = { { "note", valueJson } };
		location.push_back(subLocation);
		scriptValue = parseFullValue(fullValueJson, allowRefs, true, validationErrors, location);
		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, validationErrorCode, validationErrorMessage.c_str());
	}

	return scriptValue;
}

ScriptValue JsonScriptParser::parseFullValue(const json& valueJson, bool allowRefs, bool fromShorthand, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const char* cValueProperties[] = { "voltage", "note", "variable", "input", "output", "rand", "calc", "quantize" };
	static const vector<string> vValueProperties(begin(cValueProperties), end(cValueProperties));
	ScriptValue value;

	verifyAllowedProperties(valueJson, vValueProperties, true, validationErrors, location);

	populateRef(value, valueJson, allowRefs, validationErrors, location);
	if (value.ref.length() > 0) {
		if (hasOneOf(valueJson, cValueProperties)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_RefOrInstance, "A ref value can not be combined other non-ref value properties.");
		}
	} else {
		int valueTypes = 0;

		json::const_iterator voltage = valueJson.find("voltage");
		if (voltage != valueJson.end()) {
			valueTypes++;
			if (voltage->is_number()) {
				value.voltage.reset(new float(voltage->get<float>()));
				if ((*value.voltage < -10) || (*value.voltage > 10)) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_VoltageRange, fromShorthand ? "A voltage value must be a decimal number between -10 and 10." : "'voltage' must be a decimal number between -10 and 10.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_VoltageFloat, "'voltage' must be a decimal number between -10 and 10.");
			}
		}

		json::const_iterator note = valueJson.find("note");
		if (note != valueJson.end()) {
			valueTypes++;
			if (note->is_string()) {
				value.note.reset(new std::string(*note));
				if ((value.note->size() < 2) || (value.note->size() > 3)) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_NoteFormat, fromShorthand ? "A note value must be a string with a note name (A-G), an octave (0-9) and optionally an accidental (+ for sharp, - for flat)." : "'note' must be a string with a note name (A-G), an octave (0-9) and optionally an accidental (+ for sharp, - for flat).");
				} else {
					char n = toupper((*value.note)[0]);
					if (n < 'A' || n > 'G') {
						ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_NoteFormat, fromShorthand ? "A note value must start with a valid note name (A-G)." : "'note' must start with a valid note name (A-G).");
					}
					char s = (*value.note)[1];
					if (s < '0' || s > '9') {
						ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_NoteFormat, fromShorthand ? "A note value must have a valid scale (0-9) as second character." : "'note' must have a valid scale (0-9) as second character.");
					}
					if (value.note->size() == 3) {
						char a = (*value.note)[2];
						if (a != '+' && a != '-') {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_NoteFormat, fromShorthand ? "The third character of a note value must be a valid accidental (+ for sharp, - for flat)." : "The third character of 'note' must be a valid accidental (+ for sharp, - for flat).");
						}
					}
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_NoteString, "'note' must be a string with a note name (A-G), an octave (0-9) and optionally an accidental (+ for sharp, - for flat).");
			}
		}

		json::const_iterator variable = valueJson.find("variable");
		if (variable != valueJson.end()) {
			valueTypes++;
			if (variable->is_string()) {
				value.variable.reset(new string(*variable));
				if (value.variable->length() == 0) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_VariableNonEmpty, "'variable' must be a non-empty string.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_VariableString, "'variable' must be a non-empty string.");
			}
		}

		json::const_iterator input = valueJson.find("input");
		if (input != valueJson.end()) {
			valueTypes++;
			ScriptInput* scriptInput = new ScriptInput(parseInput(*input, true, validationErrors, location, "input", ValidationErrorCode::Value_InputObject, "'input' must be an object."));
			value.input.reset(scriptInput);
		}

		json::const_iterator output = valueJson.find("output");
		if (output != valueJson.end()) {
			valueTypes++;
			ScriptOutput* scriptOutput = new ScriptOutput(parseOutput(*output, true, validationErrors, location, "output", ValidationErrorCode::Value_OutputObject, "'output' must be an object."));
			value.output.reset(scriptOutput);
		}

		json::const_iterator rand = valueJson.find("rand");
		if (rand != valueJson.end()) {
			valueTypes++;
			if (rand->is_object()) {
				location.push_back("rand");
				ScriptRand* scriptRand = new ScriptRand(parseRand(*rand, validationErrors, location));
				value.rand.reset(scriptRand);
				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_RandObject, "'rand' must be an object.");
			}
		}

		if (valueTypes == 0) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_NoActualValue, "One of 'voltage', 'note', 'variable', 'input', 'output' or 'rand' must be set.");
		} else if (valueTypes > 1) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_MultipleValues, "Only one of 'voltage', 'note', 'variable', 'input', 'output' or 'rand' can be used.");
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
						ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_CalcObject, "'calc' elements must be objects.");
					}
					location.pop_back();
					count++;
				}

				location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_CalcArray, "'calc' must be an array.");
			}
		}

		value.quantize = false;
		json::const_iterator quantize = valueJson.find("quantize");
		if (quantize != valueJson.end()) {
			if (quantize->is_boolean()) {
				value.quantize = quantize->get<bool>();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Value_QuantizeBool, "'quantize' must be a boolean.");
			}
		}
	}

	return value;
}

ScriptOutput JsonScriptParser::parseOutput(const json& outputJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage) {
	ScriptOutput scriptOutput;

	if (outputJson.is_object()) {
		location.push_back(subLocation);
		scriptOutput = parseFullOutput(outputJson, allowRefs, false, validationErrors, location);
		location.pop_back();
	} else if (outputJson.is_number()) {
		json fullOutputJson = { { "index", outputJson } };
		scriptOutput = parseFullOutput(fullOutputJson, allowRefs, true, validationErrors, location);
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, validationErrorCode, validationErrorMessage.c_str());
	}

	return scriptOutput;
}

ScriptOutput JsonScriptParser::parseFullOutput(const json& outputJson, bool allowRefs, bool fromShorthand, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const char* cOutputProperties[] = { "index", "channel" };
	static const vector<string> vOutputProperties(begin(cOutputProperties), end(cOutputProperties));
	ScriptOutput output;

	verifyAllowedProperties(outputJson, vOutputProperties, true, validationErrors, location);

	populateRef(output, outputJson, allowRefs, validationErrors, location);
	if (output.ref.length() > 0) {
		if (hasOneOf(outputJson, cOutputProperties)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Output_RefOrInstance, "A ref output can not be combined other non-ref output properties.");
		}
	} else {
		json::const_iterator index = outputJson.find("index");
		if ((index != outputJson.end()) && (index->is_number_unsigned())) {
			output.index = index->get<int>();
			if ((output.index < 1) || (output.index > 8)) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Output_IndexRange, fromShorthand ? "The output index must be a number between 1 and 8." : "'index' must be a number between 1 and 8.");
			}
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Output_IndexNumber, fromShorthand ? "The output index is required and must be a (non-decimal) number between 1 and 8." : "'index' is required and must be a (non-decimal) number between 1 and 8.");
		}

		json::const_iterator channel = outputJson.find("channel");
		if (channel != outputJson.end()) {
			if (channel->is_number_unsigned()) {
				output.channel.reset(new int(channel->get<int>()));
				if ((*output.channel < 1) || (*output.channel > 16)) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Output_ChannelRange, "'channel' must be a number between 1 and 16.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Output_ChannelNumber, "'channel' must be a number between 1 and 16.");
			}
		}
	}

	return output;
}

ScriptInput JsonScriptParser::parseInput(const json& inputJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage) {
	ScriptInput scriptInput;

	if (inputJson.is_object()) {
		location.push_back(subLocation);
		scriptInput = parseFullInput(inputJson, allowRefs, false, validationErrors, location);
		location.pop_back();
	} else if (inputJson.is_number()) {
		json fullInputJson = { { "index", inputJson } };
		scriptInput = parseFullInput(fullInputJson, allowRefs, true, validationErrors, location);
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, validationErrorCode, validationErrorMessage.c_str());
	}

	return scriptInput;
}

ScriptInput JsonScriptParser::parseFullInput(const json& inputJson, bool allowRefs, bool fromShorthand, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const char* cInputProperties[] = { "index", "channel" };
	static const vector<string> vInputProperties(begin(cInputProperties), end(cInputProperties));
	ScriptInput input;

	verifyAllowedProperties(inputJson, vInputProperties, true, validationErrors, location);

	populateRef(input, inputJson, allowRefs, validationErrors, location);
	if (input.ref.length() > 0) {
		if (hasOneOf(inputJson, cInputProperties)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Input_RefOrInstance, "A ref input can not be combined other non-ref input properties.");
		}
	} else {
		json::const_iterator index = inputJson.find("index");
		if ((index != inputJson.end()) && (index->is_number_unsigned())) {
			input.index = index->get<int>();
			if ((input.index < 1) || (input.index > 8)) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Input_IndexRange, fromShorthand ? "The input index must be a number between 1 and 8." : "'index' must be a number between 1 and 8.");
			}
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Input_IndexNumber, fromShorthand ? "The input index is required and must be a (non-decimal) number between 1 and 8." : "'index' is required and must be a (non-decimal) number between 1 and 8.");
		}

		json::const_iterator channel = inputJson.find("channel");
		if (channel != inputJson.end()) {
			if (channel->is_number_unsigned()) {
				input.channel.reset(new int(channel->get<int>()));
				if ((*input.channel < 1) || (*input.channel > 16)) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Input_ChannelRange, "'channel' must be a number between 1 and 16.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Input_ChannelNumber, "'channel' must be a number between 1 and 16.");
			}
		}
	}

	return input;
}

ScriptRand JsonScriptParser::parseRand(const json& randJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> randProperties = { "lower", "upper" };
	ScriptRand rand;

	verifyAllowedProperties(randJson, randProperties, false, validationErrors, location);

	json::const_iterator lower = randJson.find("lower");
	if (lower != randJson.end()) {
		ScriptValue *scriptValue = new ScriptValue(parseValue(*lower, true, validationErrors, location, "lower", ValidationErrorCode::Rand_LowerObject, "'lower' is required and must be an object."));
		rand.lower.reset(scriptValue);
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Rand_LowerObject, "'lower' is required and must be a value object.");
	}

	json::const_iterator upper = randJson.find("upper");
	if (upper != randJson.end()) {
		ScriptValue *scriptValue = new ScriptValue(parseValue(*upper, true, validationErrors, location, "upper", ValidationErrorCode::Rand_UpperObject, "'upper' is required and must be an object."));
		rand.upper.reset(scriptValue);
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Rand_UpperObject, "'upper' is required and must be a value object.");
	}

	return rand;
}

ScriptCalc JsonScriptParser::parseCalc(const json& calcJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const char* cCalcProperties[] = { "add", "sub", "div", "mult", "max", "min", "remain", "frac", "round", "quantize", "sign" };
	static const vector<string> vCalcProperties(begin(cCalcProperties), end(cCalcProperties));
	ScriptCalc calc;

	verifyAllowedProperties(calcJson, vCalcProperties, true, validationErrors, location);

	populateRef(calc, calcJson, allowRefs, validationErrors, location);
	if (calc.ref.length() > 0) {
		if (hasOneOf(calcJson, cCalcProperties)) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_RefOrInstance, "A ref calc can not be combined other non-ref input properties.");
		}
	} else {
		int count = 0;

		json::const_iterator add = calcJson.find("add");
		if (add != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::ADD;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*add, true, validationErrors, location, "add", ValidationErrorCode::Calc_AddObject, "'add' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator sub = calcJson.find("sub");
		if (sub != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::SUB;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*sub, true, validationErrors, location, "sub", ValidationErrorCode::Calc_SubObject, "'sub' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator div = calcJson.find("div");
		if (div != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::DIV;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*div, true, validationErrors, location, "div", ValidationErrorCode::Calc_DivObject, "'div' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator mult = calcJson.find("mult");
		if (mult != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::MULT;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*mult, true, validationErrors, location, "mult", ValidationErrorCode::Calc_MultObject, "'mult' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator max = calcJson.find("max");
		if (max != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::MAX;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*max, true, validationErrors, location, "max", ValidationErrorCode::Calc_MaxObject, "'max' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator min = calcJson.find("min");
		if (min != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::MIN;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*min, true, validationErrors, location, "min", ValidationErrorCode::Calc_MinObject, "'min' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator remain = calcJson.find("remain");
		if (remain != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::REMAIN;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*remain, true, validationErrors, location, "remain", ValidationErrorCode::Calc_RemainObject, "'remain' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator frac = calcJson.find("frac");
		if (frac != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::FRAC;
			if ((!frac->is_boolean()) || (!frac->get<bool>())) {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_FracBoolean, "'frac' must be a boolean, with its value set to true.");
			}
		}

		json::const_iterator round = calcJson.find("round");
		if (round != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::ROUND;
			if (round->is_string()) {
				std::string roundString = round->get<string>();
				if (roundString == "up") {
					calc.roundType.reset(new ScriptCalc::RoundType(ScriptCalc::RoundType::UP));
				} else if (roundString == "down") {
					calc.roundType.reset(new ScriptCalc::RoundType(ScriptCalc::RoundType::DOWN));
				} else if (roundString == "near") {
					calc.roundType.reset(new ScriptCalc::RoundType(ScriptCalc::RoundType::NEAR));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_RoundEnum, "'round' must be a string set to either 'up', 'down' or 'near'.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_RoundString, "'round' must be a string set to either 'up', 'down' or 'near'.");
			}
		}

		json::const_iterator quantize = calcJson.find("quantize");
		if (quantize != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::QUANTIZE;
			if ((quantize->is_string()) && (quantize->get<string>().length() > 0)) {
				calc.tuning = quantize->get<string>();
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_QuantizeString, "'quantize' must be a non-empty string.");
			}
		}

		json::const_iterator sign = calcJson.find("sign");
		if (sign != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::SIGN;
			if (sign->is_string()) {
				std::string signString = sign->get<string>();
				if (signString == "pos") {
					calc.signType.reset(new ScriptCalc::SignType(ScriptCalc::SignType::POS));
				} else if (signString == "NEG") {
					calc.signType.reset(new ScriptCalc::SignType(ScriptCalc::SignType::NEG));
				} else {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_SignEnum, "'sign' must be a string set to either 'pos' or 'neg'.");
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_SignString, "'sign' must be a string set to either 'pos' or 'neg'.");
			}
		}

		if (count == 0) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_NoOperation, "Either 'add', 'sub', 'div', 'mult', 'max', 'min', 'remain', 'frac', 'round', 'quantize' or 'sign' must be set.");
		} else if (count > 1) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Calc_MultipleOperations, "At most one of 'add', 'sub', 'div', 'mult', 'max', 'min', 'remain', 'frac', 'round', 'quantize' or 'sign' may be set.");
		}
	}

	return calc;
}

ScriptInputTrigger JsonScriptParser::parseInputTrigger(const json& inputTriggerJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> inputTriggerProperties = { "id", "input" };
	ScriptInputTrigger inputTrigger;

	verifyAllowedProperties(inputTriggerJson, inputTriggerProperties, false, validationErrors, location);

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
	if (input != inputTriggerJson.end()) {
		inputTrigger.input = parseInput(*input, true, validationErrors, location, "input", ValidationErrorCode::InputTrigger_InputObject, "'input' is required and must be an object.");
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::InputTrigger_InputObject, "'input' is required and must be an input object.");
	}

	return inputTrigger;
}

ScriptTuning JsonScriptParser::parseTuning(const json& tuningJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	static const vector<string> tuningProperties = { "id", "notes" };
	ScriptTuning tuning;

	verifyAllowedProperties(tuningJson, tuningProperties, false, validationErrors, location);

	json::const_iterator id = tuningJson.find("id");
	if ((id != tuningJson.end()) && (id->is_string())) {
		tuning.id = *id;
		if (tuning.id.length() == 0) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Tuning_IdLength, "'id' can not be an empty string.");
		}
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Tuning_IdString, "'id' is required and must be a string.");
	}

	json::const_iterator notes = tuningJson.find("notes");
	if ((notes != tuningJson.end()) && (notes->is_array())) {
		location.push_back("notes");

		int count = 0;
		std::vector<json> noteElements = (*notes);
		for (const json& noteElement : noteElements) {
			location.push_back(std::to_string(count));
			if (noteElement.is_number()) {
				float x;
				float note = noteElement.get<float>();
				note = modf(note, &x); // If values > 1.0 are supplied, keep only the value within one octave
				if (note < 0.f) {
					note += 1.f;
				}
				tuning.notes.push_back(note);
			} else if (noteElement.is_string()) {
				string noteString = noteElement.get<string>();
				if ((noteString.size() < 1) || (noteString.size() > 2)) {
					ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Tuning_NoteFormat, "A note value must be a string with a note name (A-G) and optionally an accidental (+ for sharp, - for flat).");
				} else {
					int noteIndex = 0;
					char n = toupper((noteString)[0]);
					if (n < 'A' || n > 'G') {
						ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Tuning_NoteFormat, "A note value must start with a valid note name (A-G).");
					} else {
						noteIndex = noteNameToIndex(n);
					}
					if (noteString.size() == 2) {
						char a = (noteString)[1];
						if (a == '+') {
							noteIndex++;
						} else if (a == '-') {
							noteIndex--;
						} else {
							ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Tuning_NoteFormat, "The second character of a note value must be a valid accidental (+ for sharp, - for flat).");
						}
					}
					if (noteIndex > 11) {
						noteIndex -= 12;
					} else if (noteIndex < 0) {
						noteIndex += 12;
					}
					tuning.notes.push_back((float) noteIndex / 12);
				}
			} else {
				ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Tuning_NoteFloatOrString, "'notes' elements must be either 1V/Oct floats or note name strings.");
			}
			location.pop_back();
			count++;
		}

		// Sort the notes in the tuning from low to high and remove duplicates.
		sort(tuning.notes.begin(), tuning.notes.end());
		vector<float>::iterator end = unique(tuning.notes.begin(), tuning.notes.end());
		tuning.notes.erase(end, tuning.notes.end());

		if (noteElements.size() == 0) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Tuning_NotesArraySize, "'notes' must contain at least one element.");
		}

		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Tuning_NotesArray, "'notes' is required and must be an array.");
	}

	return tuning;
}

void JsonScriptParser::populateRef(ScriptRefObject &refObject, const json& refJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
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