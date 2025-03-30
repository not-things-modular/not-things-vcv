#include <gtest/gtest.h>
#include <istream>

#include "core/timeseq-json.hpp"

using namespace timeseq;
using namespace std;

#define SCRIPT_VERSION "0.0.1"


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
	EXPECT_EQ(errorMessage, "");
}

TEST(TimeSeqJsonScript, ParseInputTriggerShouldFailWithoutIdAndInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object()
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2);
	expectError(validationErrors, ValidationErrorCode::InputTrigger_IdString, "/input-triggers/0");
	expectError(validationErrors, ValidationErrorCode::InputTrigger_InputObject, "/input-triggers/0");

	EXPECT_EQ(script->inputTriggers.size(), 1);
}

TEST(TimeSeqJsonScript, ParseInputTriggerShouldFailWithoutInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object({ { "id", "my-trigger-id"} })
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::InputTrigger_InputObject, "/input-triggers/0");

	ASSERT_EQ(script->inputTriggers.size(), 1);
	EXPECT_EQ(script->inputTriggers[0].id, "my-trigger-id");

}

TEST(TimeSeqJsonScript, ParseInputTriggerShouldFailWithoutId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object({
			{ "input", json::object({ { "ref", "input-ref" }}) } 
		})
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::InputTrigger_IdString, "/input-triggers/0");

	ASSERT_EQ(script->inputTriggers.size(), 1);
	EXPECT_EQ(script->inputTriggers[0].input.ref, "input-ref");
}

TEST(TimeSeqJsonScript, ParseInputTriggerShouldFailWithEmptyId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object({
			{ "id", "" },
			{ "input", json::object({ { "ref", "input-ref" }}) } 
		})
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::InputTrigger_IdLength, "/input-triggers/0");

	ASSERT_EQ(script->inputTriggers.size(), 1);
	EXPECT_EQ(script->inputTriggers[0].input.ref, "input-ref");
}

TEST(TimeSeqJsonScript, ParseInputTriggersShouldFailOnNonObjectTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		"not-an-object"
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Script_InputTriggerObject, "/input-triggers/0");
}

TEST(TimeSeqJsonScript, ParseInputTriggersShouldFailOnEmptyInputTriggers) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = json::array();

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Script_InputTriggersItemRequired, "/input-triggers");
}

TEST(TimeSeqJsonScript, ParseInputTriggersShouldFailOnNonArray) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = { { "not", "an array" }};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Script_InputTriggersArray, "/");
}

TEST(TimeSeqJsonScript, ParseInputTriggersShouldFailWithEmptyIdOnSecondTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object({
			{ "id", "input-id" },
			{ "input", json::object({ { "ref", "input-ref-1" }}) } 
		}),
		json::object({
			{ "id", "" },
			{ "input", json::object({ { "ref", "input-ref-2" }}) } 
		})
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::InputTrigger_IdLength, "/input-triggers/1");

	ASSERT_EQ(script->inputTriggers.size(), 2);
	EXPECT_EQ(script->inputTriggers[0].id, "input-id");
	EXPECT_EQ(script->inputTriggers[0].input.ref, "input-ref-1");
	EXPECT_EQ(script->inputTriggers[1].input.ref, "input-ref-2");
}
