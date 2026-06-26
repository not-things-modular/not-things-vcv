#include "core/timeseq-script-parser-internal.hpp"

ScriptOutput JsonScriptParser::parseOutput(const json& outputJson, bool allowRefs, string subLocation, ValidationErrorCode validationErrorCode, string validationErrorMessage) {
	ScriptOutput scriptOutput;

	if (outputJson.is_object()) {
		m_context.location.push_back(subLocation);
		scriptOutput = parseFullOutput(outputJson, allowRefs, false);
		m_context.location.pop_back();
	} else if (outputJson.is_number()) {
		json fullOutputJson = { { "index", outputJson } };
		scriptOutput = parseFullOutput(fullOutputJson, allowRefs, true);
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, validationErrorCode, validationErrorMessage.c_str());
	}

	return scriptOutput;
}

ScriptOutput JsonScriptParser::parseFullOutput(const json& outputJson, bool allowRefs, bool fromShorthand) {
	static const char* cOutputProperties[] = { "index", "channel" };
	static const vector<string> vOutputProperties(begin(cOutputProperties), end(cOutputProperties));
	ScriptOutput output;

	verifyAllowedProperties(outputJson, vOutputProperties, true, m_context);

	populateRef(output, outputJson, allowRefs);
	if (output.ref.length() > 0) {
		if (hasOneOf(outputJson, cOutputProperties)) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Output_RefOrInstance, "A ref output can not be combined other non-ref output properties.");
		}
	} else {
		json::const_iterator index = outputJson.find("index");
		if ((index != outputJson.end()) && (index->is_number_unsigned())) {
			output.index = index->get<int>();
			if ((output.index < 1) || (output.index > 8)) {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Output_IndexRange, fromShorthand ? "The output 'index' must be a number between 1 and 8." : "'index' must be a number between 1 and 8.");
			}
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Output_IndexNumber, fromShorthand ? "The output 'index' is required and must be a (non-decimal) number between 1 and 8." : "'index' is required and must be a (non-decimal) number between 1 and 8.");
		}

		json::const_iterator channel = outputJson.find("channel");
		if (channel != outputJson.end()) {
			if (channel->is_number_unsigned()) {
				output.channel.reset(new int(channel->get<int>()));
				if ((*output.channel < 1) || (*output.channel > 16)) {
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Output_ChannelRange, "'channel' must be a number between 1 and 16.");
				}
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Output_ChannelNumber, "'channel' must be a number between 1 and 16.");
			}
		}
	}

	return output;
}

ScriptInput JsonScriptParser::parseInput(const json& inputJson, bool allowRefs, string subLocation, ValidationErrorCode validationErrorCode, string validationErrorMessage) {
	ScriptInput scriptInput;

	if (inputJson.is_object()) {
		m_context.location.push_back(subLocation);
		scriptInput = parseFullInput(inputJson, allowRefs, false);
		m_context.location.pop_back();
	} else if (inputJson.is_number()) {
		json fullInputJson = { { "index", inputJson } };
		scriptInput = parseFullInput(fullInputJson, allowRefs, true);
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, validationErrorCode, validationErrorMessage.c_str());
	}

	return scriptInput;
}

ScriptInput JsonScriptParser::parseFullInput(const json& inputJson, bool allowRefs, bool fromShorthand) {
	static const char* cInputProperties[] = { "index", "channel" };
	static const vector<string> vInputProperties(begin(cInputProperties), end(cInputProperties));
	ScriptInput input;

	verifyAllowedProperties(inputJson, vInputProperties, true, m_context);

	populateRef(input, inputJson, allowRefs);
	if (input.ref.length() > 0) {
		if (hasOneOf(inputJson, cInputProperties)) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Input_RefOrInstance, "A ref input can not be combined other non-ref input properties.");
		}
	} else {
		json::const_iterator index = inputJson.find("index");
		if ((index != inputJson.end()) && (index->is_number_unsigned())) {
			input.index = index->get<int>();
			if ((input.index < 1) || (input.index > 8)) {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Input_IndexRange, fromShorthand ? "The input 'index' must be a number between 1 and 8." : "'index' must be a number between 1 and 8.");
			}
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Input_IndexNumber, fromShorthand ? "The input 'index' is required and must be a (non-decimal) number between 1 and 8." : "'index' is required and must be a (non-decimal) number between 1 and 8.");
		}

		json::const_iterator channel = inputJson.find("channel");
		if (channel != inputJson.end()) {
			if (channel->is_number_unsigned()) {
				input.channel.reset(new int(channel->get<int>()));
				if ((*input.channel < 1) || (*input.channel > 16)) {
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Input_ChannelRange, "'channel' must be a number between 1 and 16.");
				}
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Input_ChannelNumber, "'channel' must be a number between 1 and 16.");
			}
		}
	}

	return input;
}

ScriptInputTrigger JsonScriptParser::parseInputTrigger(const json& inputTriggerJson) {
	static const vector<string> inputTriggerProperties = { "id", "input" };
	ScriptInputTrigger inputTrigger;

	verifyAllowedProperties(inputTriggerJson, inputTriggerProperties, false, m_context);

	json::const_iterator id = inputTriggerJson.find("id");
	if ((id != inputTriggerJson.end()) && (id->is_string())) {
		inputTrigger.id = *id;
		if (inputTrigger.id.length() == 0) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::InputTrigger_IdLength, "'id' can not be an empty string.");
		}
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::InputTrigger_IdString, "'id' is required and must be a string.");
	}

	json::const_iterator input = inputTriggerJson.find("input");
	if (input != inputTriggerJson.end()) {
		inputTrigger.input = parseInput(*input, true, "input", ValidationErrorCode::InputTrigger_InputObject, "'input' is required and must be an object.");
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::InputTrigger_InputObject, "'input' is required and must be an input object.");
	}

	return inputTrigger;
}
