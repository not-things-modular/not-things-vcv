#include "timeseq-json-shared.hpp"

shared_ptr<Script> loadScript(JsonLoader& jsonLoader, json& json, bool validate, vector<ValidationError> *validationErrors) {
	string jsonString = json.dump();
	istringstream is(jsonString);

	return jsonLoader.loadScript(is, validationErrors);
}

json getMinimalJson() {
	json json = {
		{ "type", "not-things_timeseq_script" },
		{ "version", SCRIPT_VERSION },
		{ "timelines", json::array() }
	};

	return json;
}

void expectError(vector<ValidationError>& validationErrors, int errorCode, string errorLocation) {
	string errorCodeString = "[" + to_string(errorCode) + "]";
	for (vector<ValidationError>::iterator it = validationErrors.begin(); it != validationErrors.end(); it++) {
		ValidationError& validationError = *it;
		if ((validationError.location == errorLocation) && (equal(errorCodeString.rbegin(), errorCodeString.rend(), validationError.message.rbegin()))) {
			return;
		}
	}

	string errorMessage = "Expected error code " + errorCodeString + " to be part of the validation errors at '" + errorLocation + "'";
	EXPECT_EQ(errorMessage, "") << validationErrors[0].location << " " << validationErrors[0].message;
}
