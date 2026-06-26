#include "core/timeseq-script-parser-internal.hpp"
#include "util/notes.hpp"

ScriptValue JsonScriptParser::parseValue(const json& valueJson, bool allowRefs, string subLocation, ValidationErrorCode validationErrorCode, string validationErrorMessage) {
	ScriptValue scriptValue;
	m_context.location.push_back(subLocation);

	if (valueJson.is_object()) {
		scriptValue = parseFullValue(valueJson, allowRefs, false);
	} else if (valueJson.is_number()) {
		json fullValueJson = { { "voltage", valueJson } };
		scriptValue = parseFullValue(fullValueJson, allowRefs, true);
	} else if (valueJson.is_string()) {
		json fullValueJson = { { "note", valueJson } };
		scriptValue = parseFullValue(fullValueJson, allowRefs, true);
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, validationErrorCode, validationErrorMessage.c_str());
	}

	m_context.location.pop_back();
	return scriptValue;
}

ScriptValue JsonScriptParser::parseFullValue(const json& valueJson, bool allowRefs, bool fromShorthand) {
	static const char* cValueProperties[] = { "voltage", "no-limit", "note", "variable", "input", "output", "rand", "sequence", "calc", "quantize" };
	static const vector<string> vValueProperties(begin(cValueProperties), end(cValueProperties));
	ScriptValue value;

	verifyAllowedProperties(valueJson, vValueProperties, true, m_context);

	populateRef(value, valueJson, allowRefs);
	if (value.ref.length() > 0) {
		if (hasOneOf(valueJson, cValueProperties)) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_RefOrInstance, "A ref value can not be combined other non-ref value properties.");
		}
	} else {
		int valueTypes = 0;

		unique_ptr<bool> noLimitValue;
		json::const_iterator nolimit = valueJson.find("no-limit");
		if (nolimit != valueJson.end()) {
			if (nolimit->is_boolean()) {
				noLimitValue.reset(new bool(nolimit->get<bool>()));
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_NoLimitBoolean, "'no-limit' must be a boolean.");
			}
		}

		json::const_iterator voltage = valueJson.find("voltage");
		if (voltage != valueJson.end()) {
			valueTypes++;
			if (voltage->is_number()) {
				value.voltage.reset(new float(voltage->get<float>()));
				if ((!noLimitValue) || (!*noLimitValue.get())) {
					if ((*value.voltage < -10) || (*value.voltage > 10)) {
						ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_VoltageRange, fromShorthand ? "A 'voltage' value must be a decimal number between -10 and 10." : "'voltage' must be a decimal number between -10 and 10.");
					}
				}
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_VoltageFloat, "'voltage' must be a decimal number between -10 and 10.");
			}
		} else if (noLimitValue) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_NoLimitOnNonVoltage, "'no-limit' can only be used with a 'voltage' value.");
		}

		json::const_iterator note = valueJson.find("note");
		if (note != valueJson.end()) {
			valueTypes++;
			if (note->is_string()) {
				value.note.reset(new string(*note));
				if ((value.note->size() < 2) || (value.note->size() > 3)) {
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_NoteFormat, fromShorthand ? "A 'note' value must be a string with a note name (A-G), an octave (0-9) and optionally an accidental (+ for sharp, - for flat)." : "'note' must be a string with a note name (A-G), an octave (0-9) and optionally an accidental (+ for sharp, - for flat).");
				} else {
					char n = toupper((*value.note)[0]);
					if (n < 'A' || n > 'G') {
						ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_NoteFormat, fromShorthand ? "A 'note' value must start with a valid note name (A-G)." : "'note' must start with a valid note name (A-G).");
					}
					char s = (*value.note)[1];
					if (s < '0' || s > '9') {
						ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_NoteFormat, fromShorthand ? "A 'note' value must have a valid scale (0-9) as second character." : "'note' must have a valid scale (0-9) as second character.");
					}
					if (value.note->size() == 3) {
						char a = (*value.note)[2];
						if (a != '+' && a != '-') {
							ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_NoteFormat, fromShorthand ? "The third character of a 'note' value must be a valid accidental (+ for sharp, - for flat)." : "The third character of 'note' must be a valid accidental (+ for sharp, - for flat).");
						}
					}
				}
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_NoteString, "'note' must be a string with a note name (A-G), an octave (0-9) and optionally an accidental (+ for sharp, - for flat).");
			}
		}

		json::const_iterator variable = valueJson.find("variable");
		if (variable != valueJson.end()) {
			valueTypes++;
			if (variable->is_string()) {
				value.variable.reset(new string(*variable));
				if (value.variable->length() == 0) {
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_VariableNonEmpty, "'variable' must be a non-empty string.");
				}
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_VariableString, "'variable' must be a non-empty string.");
			}
		}

		json::const_iterator input = valueJson.find("input");
		if (input != valueJson.end()) {
			valueTypes++;
			ScriptInput* scriptInput = new ScriptInput(parseInput(*input, true, "input", ValidationErrorCode::Value_InputObject, "'input' must be an object."));
			value.input.reset(scriptInput);
		}

		json::const_iterator output = valueJson.find("output");
		if (output != valueJson.end()) {
			valueTypes++;
			ScriptOutput* scriptOutput = new ScriptOutput(parseOutput(*output, true, "output", ValidationErrorCode::Value_OutputObject, "'output' must be an object."));
			value.output.reset(scriptOutput);
		}

		json::const_iterator rand = valueJson.find("rand");
		if (rand != valueJson.end()) {
			valueTypes++;
			if (rand->is_object()) {
				m_context.location.push_back("rand");
				ScriptRand* scriptRand = new ScriptRand(parseRand(*rand));
				value.rand.reset(scriptRand);
				m_context.location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_RandObject, "'rand' must be an object.");
			}
		}

		json::const_iterator sequence = valueJson.find("sequence");
		if (sequence != valueJson.end()) {
			verifyVersion(VERSION_1_2_0, m_context, "value 'sequence'");
			valueTypes++;

			m_context.location.push_back("sequence");
			ScriptSequenceValue* scriptSequenceValue = new ScriptSequenceValue(parseSequenceValue(*sequence, true));
			value.sequence.reset(scriptSequenceValue);
			m_context.location.pop_back();
		}

		if (valueTypes == 0) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_NoActualValue, "One of 'voltage', 'note', 'variable', 'input', 'output' or 'rand' must be set.");
		} else if (valueTypes > 1) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_MultipleValues, "Only one of 'voltage', 'note', 'variable', 'input', 'output' or 'rand' can be used.");
		}

		json::const_iterator calcs = valueJson.find("calc");
		if (calcs != valueJson.end()) {
			if (calcs->is_array()) {
				m_context.location.push_back("calc");

				int count = 0;
				vector<json> calcElements = (*calcs);
				for (const json& calc : calcElements) {
					m_context.location.push_back(to_string(count));
					if (calc.is_object()) {
						value.calc.push_back(parseCalc(calc, true));
					} else {
						ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_CalcObject, "'calc' elements must be objects.");
					}
					m_context.location.pop_back();
					count++;
				}

				m_context.location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_CalcArray, "'calc' must be an array.");
			}
		}

		value.quantize = false;
		json::const_iterator quantize = valueJson.find("quantize");
		if (quantize != valueJson.end()) {
			if (quantize->is_boolean()) {
				value.quantize = quantize->get<bool>();
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Value_QuantizeBool, "'quantize' must be a boolean.");
			}
		}
	}

	return value;
}

ScriptRand JsonScriptParser::parseRand(const json& randJson) {
	static const vector<string> randProperties = { "lower", "upper" };
	ScriptRand rand;

	verifyAllowedProperties(randJson, randProperties, false, m_context);

	json::const_iterator lower = randJson.find("lower");
	if (lower != randJson.end()) {
		ScriptValue *scriptValue = new ScriptValue(parseValue(*lower, true, "lower", ValidationErrorCode::Rand_LowerObject, "'lower' is required and must be an object."));
		rand.lower.reset(scriptValue);
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Rand_LowerObject, "'lower' is required and must be a value object.");
	}

	json::const_iterator upper = randJson.find("upper");
	if (upper != randJson.end()) {
		ScriptValue *scriptValue = new ScriptValue(parseValue(*upper, true, "upper", ValidationErrorCode::Rand_UpperObject, "'upper' is required and must be an object."));
		rand.upper.reset(scriptValue);
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Rand_UpperObject, "'upper' is required and must be a value object.");
	}

	return rand;
}

ScriptCalc JsonScriptParser::parseCalc(const json& calcJson, bool allowRefs) {
	static const char* cCalcProperties[] = { "add", "sub", "div", "mult", "max", "min", "remain", "frac", "trunc", "round", "quantize", "sign", "vtof" };
	static const vector<string> vCalcProperties(begin(cCalcProperties), end(cCalcProperties));
	ScriptCalc calc;

	verifyAllowedProperties(calcJson, vCalcProperties, true, m_context);

	populateRef(calc, calcJson, allowRefs);
	if (calc.ref.length() > 0) {
		if (hasOneOf(calcJson, cCalcProperties)) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_RefOrInstance, "A ref calc can not be combined other non-ref input properties.");
		}
	} else {
		int count = 0;

		json::const_iterator add = calcJson.find("add");
		if (add != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::ADD;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*add, true, "add", ValidationErrorCode::Calc_AddObject, "'add' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator sub = calcJson.find("sub");
		if (sub != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::SUB;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*sub, true, "sub", ValidationErrorCode::Calc_SubObject, "'sub' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator div = calcJson.find("div");
		if (div != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::DIV;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*div, true, "div", ValidationErrorCode::Calc_DivObject, "'div' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator mult = calcJson.find("mult");
		if (mult != calcJson.end()) {
			count++;
			calc.operation = ScriptCalc::CalcOperation::MULT;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*mult, true, "mult", ValidationErrorCode::Calc_MultObject, "'mult' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator max = calcJson.find("max");
		if (max != calcJson.end()) {
			verifyVersion(VERSION_1_1_0, m_context, "calc 'max'");
			count++;
			calc.operation = ScriptCalc::CalcOperation::MAX;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*max, true, "max", ValidationErrorCode::Calc_MaxObject, "'max' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator min = calcJson.find("min");
		if (min != calcJson.end()) {
			verifyVersion(VERSION_1_1_0, m_context, "calc 'min'");
			count++;
			calc.operation = ScriptCalc::CalcOperation::MIN;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*min, true, "min", ValidationErrorCode::Calc_MinObject, "'min' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator remain = calcJson.find("remain");
		if (remain != calcJson.end()) {
			verifyVersion(VERSION_1_1_0, m_context, "calc 'remain'");
			count++;
			calc.operation = ScriptCalc::CalcOperation::REMAIN;
			ScriptValue *scriptValue = new ScriptValue(parseValue(*remain, true, "remain", ValidationErrorCode::Calc_RemainObject, "'remain' must be an object."));
			calc.value.reset(scriptValue);
		}

		json::const_iterator trunc = calcJson.find("trunc");
		if (trunc != calcJson.end()) {
			verifyVersion(VERSION_1_1_0, m_context, "calc 'trunc'");
			count++;
			calc.operation = ScriptCalc::CalcOperation::TRUNC;
			if ((!trunc->is_boolean()) || (!trunc->get<bool>())) {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_TruncBoolean, "'trunc' must be a boolean, with its value set to true.");
			}
		}

		json::const_iterator frac = calcJson.find("frac");
		if (frac != calcJson.end()) {
			verifyVersion(VERSION_1_1_0, m_context, "calc 'frac'");
			count++;
			calc.operation = ScriptCalc::CalcOperation::FRAC;
			if ((!frac->is_boolean()) || (!frac->get<bool>())) {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_FracBoolean, "'frac' must be a boolean, with its value set to true.");
			}
		}

		json::const_iterator round = calcJson.find("round");
		if (round != calcJson.end()) {
			verifyVersion(VERSION_1_1_0, m_context, "calc 'round'");
			count++;
			calc.operation = ScriptCalc::CalcOperation::ROUND;
			if (round->is_string()) {
				string roundString = round->get<string>();
				if (roundString == "up") {
					calc.roundType.reset(new ScriptCalc::RoundType(ScriptCalc::RoundType::UP));
				} else if (roundString == "down") {
					calc.roundType.reset(new ScriptCalc::RoundType(ScriptCalc::RoundType::DOWN));
				} else if (roundString == "near") {
					calc.roundType.reset(new ScriptCalc::RoundType(ScriptCalc::RoundType::NEAR));
				} else {
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_RoundEnum, "'round' must be a string set to either 'up', 'down' or 'near'.");
				}
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_RoundString, "'round' must be a string set to either 'up', 'down' or 'near'.");
			}
		}

		json::const_iterator quantize = calcJson.find("quantize");
		if (quantize != calcJson.end()) {
			verifyVersion(VERSION_1_1_0, m_context, "calc 'quantize'");
			count++;
			calc.operation = ScriptCalc::CalcOperation::QUANTIZE;
			if (quantize->is_object()) {
				m_context.location.push_back("quantize");
				ScriptTuning *scriptTuning = new ScriptTuning(parseTuning(*quantize, true));
				calc.tuning.reset(scriptTuning);
				m_context.location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_QuantizeObject, "'quantize' must be a tuning object.");
			}
		}

		json::const_iterator sign = calcJson.find("sign");
		if (sign != calcJson.end()) {
			verifyVersion(VERSION_1_1_0, m_context, "calc 'sign'");
			count++;
			calc.operation = ScriptCalc::CalcOperation::SIGN;
			if (sign->is_string()) {
				string signString = sign->get<string>();
				if (signString == "pos") {
					calc.signType.reset(new ScriptCalc::SignType(ScriptCalc::SignType::POS));
				} else if (signString == "neg") {
					calc.signType.reset(new ScriptCalc::SignType(ScriptCalc::SignType::NEG));
				} else {
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_SignEnum, "'sign' must be a string set to either 'pos' or 'neg'.");
				}
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_SignString, "'sign' must be a string set to either 'pos' or 'neg'.");
			}
		}

		json::const_iterator vtof = calcJson.find("vtof");
		if (vtof != calcJson.end()) {
			verifyVersion(VERSION_1_1_0, m_context, "calc 'vtof'");
			count++;
			calc.operation = ScriptCalc::CalcOperation::VTOF;
			if ((!vtof->is_boolean()) || (!vtof->get<bool>())) {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_VtofBoolean, "'vtof' must be a boolean, with its value set to true.");
			}
		}

		if (count == 0) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_NoOperation, "Either 'add', 'sub', 'div', 'mult', 'max', 'min', 'remain', 'frac', 'round', 'quantize', 'sign' or 'vtof' must be set.");
		} else if (count > 1) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_MultipleOperations, "At most one of 'add', 'sub', 'div', 'mult', 'max', 'min', 'remain', 'frac', 'round', 'quantize', 'sign' or 'vtof' may be set.");
		}
	}

	return calc;
}

ScriptTuning JsonScriptParser::parseTuning(const json& tuningJson, bool allowRefs) {
	static const char* cTuningProperties[] = { "notes" };
	static const vector<string> vTuningProperties(begin(cTuningProperties), end(cTuningProperties));
	ScriptTuning tuning;

	verifyAllowedProperties(tuningJson, vTuningProperties, true, m_context);

	populateRef(tuning, tuningJson, allowRefs);
	if (tuning.ref.length() > 0) {
		if (hasOneOf(tuningJson, cTuningProperties)) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Tuning_RefOrInstance, "A ref tuning can not be combined other non-ref tuning properties.");
		}
	} else {
		json::const_iterator notes = tuningJson.find("notes");
		if ((notes != tuningJson.end()) && (notes->is_array())) {
			m_context.location.push_back("notes");

			int count = 0;
			vector<json> noteElements = (*notes);
			for (const json& noteElement : noteElements) {
				m_context.location.push_back(to_string(count));
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
						ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Tuning_NoteFormat, "A note value must be a string with a note name (A-G) and optionally an accidental (+ for sharp, - for flat).");
					} else {
						int noteIndex = 0;
						char n = toupper((noteString)[0]);
						if (n < 'A' || n > 'G') {
							ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Tuning_NoteFormat, "A note value must start with a valid note name (A-G).");
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
								ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Tuning_NoteFormat, "The second character of a note value must be a valid accidental (+ for sharp, - for flat).");
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
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Tuning_NoteFloatOrString, "'notes' elements must be either 1V/Oct floats or note name strings.");
				}
				m_context.location.pop_back();
				count++;
			}

			// Sort the notes in the tuning from low to high and remove duplicates.
			sort(tuning.notes.begin(), tuning.notes.end());
			vector<float>::iterator end = unique(tuning.notes.begin(), tuning.notes.end());
			tuning.notes.erase(end, tuning.notes.end());

			if (noteElements.size() == 0) {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Tuning_NotesArraySize, "'notes' must contain at least one element.");
			}

			m_context.location.pop_back();
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Tuning_NotesArray, "'notes' is required and must be an array.");
		}
	}

	return tuning;
}

ScriptSequenceValue JsonScriptParser::parseSequenceValue(const json& sequenceJson, bool allowRefs) {
	ScriptSequenceValue scriptSequenceValue;

	// Set the default values
	scriptSequenceValue.moveBefore = ScriptSequenceMoveDirection::NONE;
	scriptSequenceValue.moveAfter = ScriptSequenceMoveDirection::FORWARD;
	scriptSequenceValue.wrap = true;

	// Either accept a string as an id shorthand, or a full object
	if (sequenceJson.is_string()) {
		scriptSequenceValue.id = sequenceJson.get<string>();
		if (scriptSequenceValue.id.length() == 0) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::SequenceValue_EmptyString, "'sequence' can not be an empty string.");
		}
	} else if (sequenceJson.is_object()) {
		json::const_iterator id = sequenceJson.find("id");
		if ((id != sequenceJson.end()) && (id->is_string())) {
			string idValue = *id;
			if (idValue.length() > 0) {
				scriptSequenceValue.id = idValue;
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::SequenceValue_IdLength, "'id' can not be an empty string.");
			}
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::SequenceValue_IdString, "'id' is required and must be a string.");
		}

		json::const_iterator moveBefore = sequenceJson.find("move-before");
		if (moveBefore != sequenceJson.end()) {
			scriptSequenceValue.moveBefore = parseScriptSequenceMoveDirection(*moveBefore, "move-before", ValidationErrorCode::SequenceValue_MoveDirectionEnum, ValidationErrorCode::SequenceValue_MoveDirectionString, m_context);
		}

		json::const_iterator moveAfter = sequenceJson.find("move-after");
		if (moveAfter != sequenceJson.end()) {
			scriptSequenceValue.moveAfter = parseScriptSequenceMoveDirection(*moveAfter, "move-after", ValidationErrorCode::SequenceValue_MoveDirectionEnum, ValidationErrorCode::SequenceValue_MoveDirectionString, m_context);
		}

		json::const_iterator wrap = sequenceJson.find("wrap");
		if (wrap != sequenceJson.end()) {
			if (wrap->is_boolean()) {
				scriptSequenceValue.wrap = wrap->get<bool>();
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::SequenceValue_WrapBoolean, "'wrap' must be a boolean.");
			}
		}
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::SequenceValue_StringOrObject, "A 'sequence' value should either be a string or an object.");
	}

	return scriptSequenceValue;
}
