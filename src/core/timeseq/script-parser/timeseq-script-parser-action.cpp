#include "core/timeseq-script-parser-internal.hpp"

ScriptAction JsonScriptParser::parseAction(const json& actionJson, bool allowRefs) {
	static const char* cActionProperties[] = { "timing", "set-value", "set-variable", "set-polyphony", "set-label", "assert", "trigger", "move-sequence", "clear-sequence", "add-to-sequence", "remove-from-sequence", "start-value", "end-value", "ease-factor", "ease-algorithm", "output", "variable", "if", "gate-high-ratio" };
	static const vector<string> vActionProperties(begin(cActionProperties), end(cActionProperties));
	ScriptAction action;

	verifyAllowedProperties(actionJson, vActionProperties, true, m_context);

	populateRef(action, actionJson, allowRefs);
	if (action.ref.length() > 0) {
		if (hasOneOf(actionJson, cActionProperties)) {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_RefOrInstance, "A ref action can not be combined other non-ref action properties.");
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
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_TimingEnum, "timing must be either 'start', 'end', 'glide' or 'gate'.");
			}
		} else {
			action.timing = ScriptAction::ActionTiming::START;
		}

		json::const_iterator setValue = actionJson.find("set-value");
		if (setValue != actionJson.end()) {
			actionCount++;
			if (setValue->is_object()) {
				m_context.location.push_back("set-value");
				ScriptSetValue *scriptSetValue = new ScriptSetValue(parseSetValue(*setValue));
				action.setValue.reset(scriptSetValue);
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_SetValueObject, "'set-value' must be an object.");
			}
		}

		json::const_iterator setVariable = actionJson.find("set-variable");
		if (setVariable != actionJson.end()) {
			actionCount++;
			if (setVariable->is_object()) {
				m_context.location.push_back("set-variable");
				ScriptSetVariable *scriptSetVariable = new ScriptSetVariable(parseSetVariable(*setVariable));
				action.setVariable.reset(scriptSetVariable);
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_SetVariableObject, "'set-variable' must be an object.");
			}
		}

		json::const_iterator setPolyphony = actionJson.find("set-polyphony");
		if (setPolyphony != actionJson.end()) {
			actionCount++;
			if (setPolyphony->is_object()) {
				m_context.location.push_back("set-polyphony");
				ScriptSetPolyphony *scriptSetPolyphony = new ScriptSetPolyphony(parseSetPolyphony(*setPolyphony));
				action.setPolyphony.reset(scriptSetPolyphony);
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_SetPolyphonyObject, "'set-polyphony' must be an object.");
			}
		}

		json::const_iterator setLabel = actionJson.find("set-label");
		if (setLabel != actionJson.end()) {
			actionCount++;
			if (setLabel->is_object()) {
				m_context.location.push_back("set-label");
				ScriptSetLabel *scriptSetLabel = new ScriptSetLabel(parseSetLabel(*setLabel));
				action.setLabel.reset(scriptSetLabel);
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_SetLabelObject, "'set-label' must be an object.");
			}
		}

		json::const_iterator assert = actionJson.find("assert");
		if (assert != actionJson.end()) {
			actionCount++;
			if (assert->is_object()) {
				m_context.location.push_back("assert");
				ScriptAssert *scriptAssert = new ScriptAssert(parseAssert(*assert));
				action.assert.reset(scriptAssert);
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_AssertObject, "'assert' must be an object.");
			}
		}

		json::const_iterator trigger = actionJson.find("trigger");
		if (trigger != actionJson.end()) {
			actionCount++;
			if (trigger->is_string()) {
				action.trigger = *trigger;
				if (action.trigger.size() == 0) {
					addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_TriggerLength, "'trigger' can not be an empty string.");
				}
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_TriggerString, "'trigger' must be a string.");
			}
		}

		json::const_iterator moveSequence = actionJson.find("move-sequence");
		if (moveSequence != actionJson.end()) {
			verifyVersion(VERSION_1_2_0, m_context, "'move-sequence'");
			actionCount++;
			if (moveSequence->is_object()) {
				m_context.location.push_back("move-sequence");
				ScriptMoveSequence *scriptMoveSequence = new ScriptMoveSequence(parseMoveSequence(*moveSequence));
				action.moveSequence.reset(scriptMoveSequence);
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_MoveSequenceObject, "'move-sequence' must be an object.");
			}
		}

		json::const_iterator clearSequence = actionJson.find("clear-sequence");
		if (clearSequence != actionJson.end()) {
			verifyVersion(VERSION_1_2_0, m_context, "'clear-sequence'");
			actionCount++;
			if (clearSequence->is_string()) {
				action.clearSequence = *clearSequence;
				if (action.clearSequence.size() == 0) {
					addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_ClearSequenceLength, "'clear-sequence' can not be an empty string.");
				}
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_ClearSequenceString, "'clear-sequence' must be a string.");
			}
		}

		json::const_iterator addToSequence = actionJson.find("add-to-sequence");
		if (addToSequence != actionJson.end()) {
			verifyVersion(VERSION_1_2_0, m_context, "'add-to-sequence'");
			actionCount++;
			if (addToSequence->is_object()) {
				m_context.location.push_back("add-to-sequence");
				ScriptAddToSequence *scriptAddToSequence = new ScriptAddToSequence(parseAddToSequence(*addToSequence));
				action.addToSequence.reset(scriptAddToSequence);
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_AddToSequenceObject, "'add-to-sequence' must be an object.");
			}
		}

		json::const_iterator removeFromSequence = actionJson.find("remove-from-sequence");
		if (removeFromSequence != actionJson.end()) {
			verifyVersion(VERSION_1_2_0, m_context, "'remove-from-sequence'");
			actionCount++;
			if (removeFromSequence->is_object()) {
				m_context.location.push_back("remove-from-sequence");
				ScriptRemoveFromSequence *scriptRemoveFromSequence = new ScriptRemoveFromSequence(parseRemoveFromSequence(*removeFromSequence));
				action.removeFromSequence.reset(scriptRemoveFromSequence);
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_RemoveFromSequenceObject, "'remove-from-sequence' must be an object.");
			}
		}

		json::const_iterator startValue = actionJson.find("start-value");
		if (startValue != actionJson.end()) {
			ScriptValue *scriptValue = new ScriptValue(parseValue(*startValue, true, "start-value", ValidationErrorCode::Action_StartValueObject, "'start-value' must be an object."));
			action.startValue.reset(scriptValue);
		}

		json::const_iterator endValue = actionJson.find("end-value");
		if (endValue != actionJson.end()) {
			ScriptValue *scriptValue = new ScriptValue(parseValue(*endValue, true, "end-value", ValidationErrorCode::Action_EndValueObject, "'end-value' must be an object."));
			action.endValue.reset(scriptValue);
		}

		json::const_iterator easeFactor = actionJson.find("ease-factor");
		if (easeFactor != actionJson.end()) {
			if (easeFactor->is_number()) {
				action.easeFactor.reset(new float(easeFactor->get<float>()));
				if ((*action.easeFactor.get() < -5.0) || (*action.easeFactor.get() > 5.0)) {
					addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_EaseFactorRange, "'ease-factor' must be a number between -5.0 and 5.0.");
				}
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_EaseFactorFloat, "'ease-factor' must be a number between -5.0 and 5.0.");
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
					addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_EaseAlgorithmEnum, "'ease-algorithm' must be either the string 'pow' or 'sig'.");
				}
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_EaseAlgorithmEnum, "'ease-algorithm' must be either the string 'pow' or 'sig'.");
			}
		}

		json::const_iterator gateHighRatio = actionJson.find("gate-high-ratio");
		if (gateHighRatio != actionJson.end()) {
			if (gateHighRatio->is_number()) {
				action.gateHighRatio.reset(new float(gateHighRatio->get<float>()));
				if ((*action.gateHighRatio.get() < 0.f) || (*action.gateHighRatio.get() > 1.f)) {
					addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_GateHighRatioRange, "'gate-high-ratio' must be a number between 0.0 and 1.0.");
				}
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_GateHighRatioFloat, "'gate-high-ratio' must be a number between 0.0 and 1.0.");
			}
		}

		json::const_iterator output = actionJson.find("output");
		if (output != actionJson.end()) {
			ScriptOutput *scriptOutput = new ScriptOutput(parseOutput(*output, true, "output", ValidationErrorCode::Action_OutputObject, "'output' must be an object."));
			action.output.reset(scriptOutput);
		}

		json::const_iterator variable = actionJson.find("variable");
		if (variable != actionJson.end()) {
			if (variable->is_string()) {
				action.variable = *variable;
				if (action.variable.size() == 0) {
					addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_VariableLength, "'variable' can not be an empty string.");
				}
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_VariableString, "'variable' must be a string.");
			}
		}

		json::const_iterator ifCondition = actionJson.find("if");
		if (ifCondition != actionJson.end()) {
			if (ifCondition->is_object()) {
				m_context.location.push_back("if");
				ScriptIf *scriptIf = new ScriptIf(parseIf(*ifCondition, true));
				action.condition.reset(scriptIf);
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_IfObject, "'if' must be an object.");
			}
		}

		if (action.timing == ScriptAction::ActionTiming::GLIDE) {
			if ((action.setValue) || (action.setVariable) || (action.setPolyphony) || (action.setLabel) || (action.assert) || (action.trigger.size() > 0) || (action.moveSequence) || (action.clearSequence.length() > 0) || (action.addToSequence) || (action.removeFromSequence) || (action.gateHighRatio)) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_NonGlideProperties, "'set-value', 'set-variable', 'set-polyphony', 'set-label', 'assert', 'trigger', 'move-sequence', 'clear-sequence', 'add-to-sequence', 'remove-from-sequence' and 'gate-high-ratio' can not be used in combination with 'GLIDE' timing.");
			}
			if ((!action.startValue) || (!action.endValue)) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_MissingGlideValues, "'start-value' and 'end-value' must be present when 'GLIDE' timing is used.");
			}
			if ((!action.output) && (action.variable.length() == 0)) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_MissingGlideActions, "Either 'output' or 'variable' must be present when 'GLIDE' timing is used.");
			}
			if ((action.output) && (action.variable.length() > 0)) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_TooManyGlideActions, "Only one of 'output' and 'variable' can be present when 'GLIDE' timing is used.");
			}
		} else if (action.timing == ScriptAction::ActionTiming::GATE) {
			if ((action.setValue) || (action.setVariable) || (action.setPolyphony) || (action.setLabel) || (action.assert) || (action.trigger.size() > 0) || (action.moveSequence) || (action.clearSequence.length() > 0) || (action.addToSequence) || (action.removeFromSequence) || (action.startValue) || (action.endValue) || (action.easeFactor) || (action.easeAlgorithm)) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_NonGateProperties, "'set-value', 'set-variable', 'set-polyphony', 'set-label', 'assert', 'trigger', 'move-sequence', 'clear-sequence', 'add-to-sequence', 'remove-from-sequence', 'start-value', 'end-value', 'ease-factory' and 'ease-algorithm' can not be used in combination with 'GATE' timing.");
			}
			if (!action.output) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_GateOutput, "'output' must be present when 'GATE' timing is used.");
			}
			if (action.variable.length() > 0) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_NonGateProperties, "'variable' can only be used in combination with 'GLIDE' timing.");
			}
		} else {
			if ((action.startValue) || (action.endValue) || (action.easeFactor) || (action.easeAlgorithm) || (action.gateHighRatio)) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "'start-value', 'end-value', 'ease-factory' 'ease-algorithm' and 'gate-high-ratio' can only be used in combination with 'GLIDE' timing.");
			}
			if ((action.output) || (action.variable.length() > 0)) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "'output' and 'variable' can only be used in combination with 'GLIDE' timing.");
			}
			if ((!action.setValue) && (!action.setVariable) && (!action.setPolyphony) && (!action.setLabel) && (!action.assert) && (action.trigger.size() == 0) && (!action.moveSequence) && (action.clearSequence.length() == 0) && (!action.addToSequence) && (!action.removeFromSequence)) {
				string timingStr = timing != actionJson.end() ? *timing : "start";
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_MissingNonGlideProperties, "'set-value', 'set-variable', 'set-polyphony', 'set-label', 'assert', 'trigger', 'move-sequence', 'clear-sequence', 'add-to-sequence' or 'remove-from-sequence' must be present for '", timingStr.c_str(), "' timing.");
			}
			if (actionCount > 1) {
				string timingStr = timing != actionJson.end() ? *timing : "start";
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Action_TooManyNonGlideProperties, "Only one of 'set-value', 'set-variable', 'set-polyphony', 'set-label', 'assert', 'trigger', 'move-sequence', 'clear-sequence', 'add-to-sequence' or 'remove-from-sequence' can be used in the same '", timingStr.c_str(), "' action.");
			}
		}
	}

	return action;
}

ScriptSetValue JsonScriptParser::parseSetValue(const json& setValueJson) {
	static const vector<string> setValueProperties = { "output", "value" };
	ScriptSetValue setValue;

	verifyAllowedProperties(setValueJson, setValueProperties, false, m_context);

	json::const_iterator output = setValueJson.find("output");
	if (output != setValueJson.end()) {
		setValue.output = parseOutput(*output, true, "output", ValidationErrorCode::SetValue_OutputObject, "'output' is required and must be an object.");
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetValue_OutputObject, "'output' is required and must be a output object.");
	}

	json::const_iterator value = setValueJson.find("value");
	if (value != setValueJson.end()) {
		setValue.value = parseValue(*value, true, "value", ValidationErrorCode::SetValue_ValueObject, "'value' is required and must be an object.");
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetValue_ValueObject, "'value' is required and must be a value object.");
	}

	return setValue;
}

ScriptSetVariable JsonScriptParser::parseSetVariable(const json& setVariableJson) {
	static const vector<string> setVariableProperties  = { "name", "value" };
	ScriptSetVariable setVariable;

	verifyAllowedProperties(setVariableJson, setVariableProperties, false, m_context);

	json::const_iterator name = setVariableJson.find("name");
	if ((name != setVariableJson.end()) && (name->is_string())) {
		setVariable.name = *name;
		if (setVariable.name.length() == 0) {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetVariable_NameLength, "'name' must be a non-empty string.");
		}
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetVariable_NameString, "'name' is required and must be a non-empty string.");
	}

	json::const_iterator value = setVariableJson.find("value");
	if (value != setVariableJson.end()) {
		setVariable.value = parseValue(*value, true, "value", ValidationErrorCode::SetVariable_ValueObject, "'value' is required and must be an object.");
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetVariable_ValueObject, "'value' is required and must be a value object.");
	}

	return setVariable;
}

ScriptSetPolyphony JsonScriptParser::parseSetPolyphony(const json& setPolyphonyJson) {
	static const vector<string> setPolyphonyProperties = { "index", "channels" };
	ScriptSetPolyphony setPolyphony;

	verifyAllowedProperties(setPolyphonyJson, setPolyphonyProperties, false, m_context);

	json::const_iterator index = setPolyphonyJson.find("index");
	if ((index != setPolyphonyJson.end()) && (index->is_number_unsigned())) {
		setPolyphony.index = index->get<int>();
		if ((setPolyphony.index < 1) || (setPolyphony.index > 8)) {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetPolyphony_IndexRange, "'index' must be a number between 1 and 8.");
		}
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetPolyphony_IndexNumber, "'index' is required and must be a number between 1 and 8.");
	}

	json::const_iterator channels = setPolyphonyJson.find("channels");
	if ((channels != setPolyphonyJson.end()) && (channels->is_number_unsigned())) {
		setPolyphony.channels = channels->get<int>();
		if ((setPolyphony.channels < 1) || (setPolyphony.channels > 16)) {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetPolyphony_ChannelsRange, "'channels' must be a number between 1 and 16.");
		}
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetPolyphony_ChannelsNumber, "'channels' is required and must be a number between 1 and 16.");
	}

	return setPolyphony;
}

ScriptSetLabel JsonScriptParser::parseSetLabel(const json& setLabelJson) {
	static const vector<string> setLabelProperties = { "index", "label" };
	ScriptSetLabel setLabel;

	verifyAllowedProperties(setLabelJson, setLabelProperties, false, m_context);

	json::const_iterator index = setLabelJson.find("index");
	if ((index != setLabelJson.end()) && (index->is_number_unsigned())) {
		setLabel.index = index->get<int>();
		if ((setLabel.index < 1) || (setLabel.index > 8)) {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetLabel_IndexRange, "'index' must be a number between 1 and 8.");
		}
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetLabel_IndexNumber, "'index' is required and must be a number between 1 and 8.");
	}

	json::const_iterator label = setLabelJson.find("label");
	if ((label != setLabelJson.end()) && (label->is_string())) {
		setLabel.label = label->get<string>();
		if (setLabel.label.size() == 0) {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetLabel_LabelLength, "'label' can not be an empty string.");
		}
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::SetLabel_LabelString, "'label' must be a non-empty string.");
	}

	return setLabel;
}

ScriptAssert JsonScriptParser::parseAssert(const json& assertJson) {
	static const vector<string> assertProperties = { "name", "expect", "stop-on-fail" };
	ScriptAssert scriptAssert;

	verifyAllowedProperties(assertJson, assertProperties, false, m_context);

	json::const_iterator name = assertJson.find("name");
	if (name != assertJson.end()) {
		if (name->is_string()) {
			scriptAssert.name = name->get<string>();
			if (scriptAssert.name.length() < 1) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Assert_NameLength, "'name' can not be an empty string.");
			}
		} else {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Assert_NameString, "'name' is required and must be a non-empty string.");
		}
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Assert_NameString, "'name' must be a non-empty string.");
	}

	json::const_iterator expect = assertJson.find("expect");
	if (expect != assertJson.end()) {
		if (expect->is_object()) {
			m_context.location.push_back("expect");
			scriptAssert.expect = parseIf(*expect, true);
			m_context.location.pop_back();
		} else {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Assert_ExpectObject, "'expect' must be an object.");
		}
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Assert_ExpectObject, "'expect' is required and must be an object.");
	}

	json::const_iterator stopOnFail = assertJson.find("stop-on-fail");
	scriptAssert.stopOnFail = true;
	if (stopOnFail != assertJson.end()) {
		if (stopOnFail->is_boolean()) {
			scriptAssert.stopOnFail = stopOnFail->get<bool>();
		} else {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::Assert_StopOnFailBool, "'stop-on-fail' must be a boolean.");
		}
	}

	return scriptAssert;
}

ScriptMoveSequence JsonScriptParser::parseMoveSequence(const json& moveSequenceJson) {
	static const vector<string> moveSequenceProperties = { "id", "direction", "wrap", "position" };
	ScriptMoveSequence scriptMoveSequence;

	verifyAllowedProperties(moveSequenceJson, moveSequenceProperties, false, m_context);

	json::const_iterator id = moveSequenceJson.find("id");
	if (id != moveSequenceJson.end()) {
		if (id->is_string()) {
			scriptMoveSequence.id = id->get<string>();
			if (scriptMoveSequence.id.length() < 1) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::MoveSequence_IdLength, "'id' can not be an empty string.");
			}
		} else {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::MoveSequence_IdString, "'id' is required and must be a non-empty string.");
		}
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::MoveSequence_IdString, "'id' must be a non-empty string.");
	}

	json::const_iterator direction = moveSequenceJson.find("direction");
	if (direction != moveSequenceJson.end()) {
		scriptMoveSequence.direction.reset(new ScriptSequenceMoveDirection(parseScriptSequenceMoveDirection(*direction, "direction", ValidationErrorCode::MoveSequence_MoveDirectionEnum, ValidationErrorCode::MoveSequence_MoveDirectionString, m_context)));
	}

	json::const_iterator position = moveSequenceJson.find("position");
	if (position != moveSequenceJson.end()) {
		if (position->is_number_integer()) {
			scriptMoveSequence.position.reset(new int(position->get<int>()));
		} else {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::MoveSequence_PositionNumber, "'position' must be a number.");
		}
	}

	scriptMoveSequence.wrap = true;
	json::const_iterator wrap = moveSequenceJson.find("wrap");
	if (wrap != moveSequenceJson.end()) {
		if (scriptMoveSequence.position) {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::MoveSequence_NoWrapWithPosition, "'wrap' can not be used in combination with 'position'.");
		}
		if (wrap->is_boolean()) {
			scriptMoveSequence.wrap = wrap->get<bool>();
		} else {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::MoveSequence_WrapBoolean, "'wrap' must be a boolean.");
		}
	}

	if ((scriptMoveSequence.direction) && (scriptMoveSequence.position)) {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::MoveSequence_EitherDirectionOrPosition, "Only one of 'direction' or 'position' can be used at a time.");
	}

	if ((!scriptMoveSequence.direction) && (!scriptMoveSequence.position)) {
		scriptMoveSequence.direction.reset(new ScriptSequenceMoveDirection(ScriptSequenceMoveDirection::FORWARD));
	}

	return scriptMoveSequence;
}

ScriptAddToSequence JsonScriptParser::parseAddToSequence(const json& addToSequenceJson) {
	static const vector<string> addToSequenceProperties = { "id", "value", "position", "as-constant-voltage" };
	ScriptAddToSequence scriptAddToSequence;

	verifyAllowedProperties(addToSequenceJson, addToSequenceProperties, false, m_context);

	json::const_iterator id = addToSequenceJson.find("id");
	if (id != addToSequenceJson.end()) {
		if (id->is_string()) {
			scriptAddToSequence.id = id->get<string>();
			if (scriptAddToSequence.id.length() < 1) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::AddToSequence_IdLength, "'id' can not be an empty string.");
			}
		} else {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::AddToSequence_IdString, "'id' is required and must be a non-empty string.");
		}
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::AddToSequence_IdString, "'id' must be a non-empty string.");
	}

	json::const_iterator value = addToSequenceJson.find("value");
	if (value != addToSequenceJson.end()) {
		scriptAddToSequence.value = parseValue(*value, true, "value", ValidationErrorCode::AddToSequence_ValueObject, "'value' must be a value object.");
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::AddToSequence_ValueObject, "'value' is required and must be a value object.");
	}

	scriptAddToSequence.position = -1;
	json::const_iterator position = addToSequenceJson.find("position");
	if (position != addToSequenceJson.end()) {
		if (position->is_number_integer()) {
			scriptAddToSequence.position = position->get<int>();
		} else {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::AddToSequence_PositionNumber, "'position' must be a number.");
		}
	}

	scriptAddToSequence.asConstantVoltage = false;
	json::const_iterator asConstantVoltage = addToSequenceJson.find("as-constant-voltage");
	if (asConstantVoltage != addToSequenceJson.end()) {
		if (asConstantVoltage->is_boolean()) {
			scriptAddToSequence.asConstantVoltage = asConstantVoltage->get<bool>();
		} else {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::AddToSequence_AsConstantVoltageBoolean, "'as-constant-voltage' must be a boolean.");
		}
	}

	return scriptAddToSequence;
}

ScriptRemoveFromSequence JsonScriptParser::parseRemoveFromSequence(const json& removeFromJson) {
	static const vector<string> removeFromSequenceProperties = { "id", "position" };
	ScriptRemoveFromSequence scriptRemoveFromSquence;

	verifyAllowedProperties(removeFromJson, removeFromSequenceProperties, false, m_context);

	scriptRemoveFromSquence.position = -1;

	json::const_iterator id = removeFromJson.find("id");
	if (id != removeFromJson.end()) {
		if (id->is_string()) {
			scriptRemoveFromSquence.id = id->get<string>();
			if (scriptRemoveFromSquence.id.length() < 1) {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::RemoveFromSequence_IdLength, "'id' can not be an empty string.");
			}
		} else {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::RemoveFromSequence_IdString, "'id' is required and must be a non-empty string.");
		}
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::RemoveFromSequence_IdString, "'id' must be a non-empty string.");
	}

	json::const_iterator position = removeFromJson.find("position");
	if (position != removeFromJson.end()) {
		if (position->is_number_integer()) {
			scriptRemoveFromSquence.position = position->get<int>();
		} else {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::RemoveFromSequence_PositionNumber, "'position' must be a number.");
		}
	}

	return scriptRemoveFromSquence;
}
