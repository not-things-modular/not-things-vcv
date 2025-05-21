#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggerShouldFailWithoutIdAndInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object()
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::InputTrigger_IdString, "/input-triggers/0");
	expectError(validationErrors, ValidationErrorCode::InputTrigger_InputObject, "/input-triggers/0");

	EXPECT_EQ(script->inputTriggers.size(), 1u);
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggerShouldFailWithoutInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object({ { "id", "my-trigger-id"} })
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::InputTrigger_InputObject, "/input-triggers/0");

	ASSERT_EQ(script->inputTriggers.size(), 1u);
	EXPECT_EQ(script->inputTriggers[0].id, "my-trigger-id");

}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggerShouldFailWithFloatShorthandInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object({ { "id", "my-trigger-id" }, { "input", 3.5 } })
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_IndexNumber, "/input-triggers/0");
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggerShouldFailWithShorthandInputOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object({ { "id", "my-trigger-id" }, { "input", 9 } })
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_IndexRange, "/input-triggers/0");
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggerShouldFailWithoutId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object({
			{ "input", json::object({ { "ref", "input-ref" }}) } 
		})
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::InputTrigger_IdString, "/input-triggers/0");

	ASSERT_EQ(script->inputTriggers.size(), 1u);
	EXPECT_EQ(script->inputTriggers[0].input.ref, "input-ref");
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggerShouldFailWithEmptyId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object({
			{ "id", "" },
			{ "input", json::object({ { "ref", "input-ref" }}) } 
		})
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::InputTrigger_IdLength, "/input-triggers/0");

	ASSERT_EQ(script->inputTriggers.size(), 1u);
	EXPECT_EQ(script->inputTriggers[0].input.ref, "input-ref");
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggersShouldFailOnNonObjectTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		"not-an-object"
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_InputTriggerObject, "/input-triggers/0");
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggersShouldFailOnNonArray) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = { { "not", "an array" }};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_InputTriggersArray, "/");
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggersShouldFailWithEmptyIdOnSecondTrigger) {
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::InputTrigger_IdLength, "/input-triggers/1");

	ASSERT_EQ(script->inputTriggers.size(), 2u);
	EXPECT_EQ(script->inputTriggers[0].id, "input-id");
	EXPECT_EQ(script->inputTriggers[0].input.ref, "input-ref-1");
	EXPECT_EQ(script->inputTriggers[1].input.ref, "input-ref-2");
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggersShouldParseMultipleInputTriggers) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object({
			{ "id", "input-id-1" },
			{ "input", json::object({ { "ref", "input-ref-1" }}) } 
		}),
		json::object({
			{ "id", "input-id-2" },
			{ "input", json::object({ { "ref", "input-ref-2" }}) } 
		})
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);

	ASSERT_EQ(script->inputTriggers.size(), 2u);
	EXPECT_EQ(script->inputTriggers[0].id, "input-id-1");
	EXPECT_EQ(script->inputTriggers[0].input.ref, "input-ref-1");
	EXPECT_EQ(script->inputTriggers[1].id, "input-id-2");
	EXPECT_EQ(script->inputTriggers[1].input.ref, "input-ref-2");
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggersShouldParseShorthandInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		json::object({
			{ "id", "input-id-1" },
			{ "input", 5 } 
		})
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);

	ASSERT_EQ(script->inputTriggers.size(), 1u);
	EXPECT_EQ(script->inputTriggers[0].id, "input-id-1");
	EXPECT_EQ(script->inputTriggers[0].input.index, 5);
	EXPECT_FALSE(script->inputTriggers[0].input.channel);
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggerWithUnknownPropertyShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		{
			{ "id", "input-id-1" },
			{ "input", { { "ref", "input-ref-1" } } },
			{ "unknown-prop", "value" }
		}
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/input-triggers/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggerWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		{
			{ "id", "input-id-1" },
			{ "input", { { "ref", "input-ref-1" } } },
				{ "unknown-prop-1", "value" },
				{ "unknown-prop-2", { { "child", "object" } } }
		}
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/input-triggers/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptInputTrigger, ParseInputTriggerShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = {
		{
			{ "id", "input-id-1" },
			{ "input", { { "ref", "input-ref-1" } } },
				{ "x-unknown-prop-1", "value" },
				{ "x-unknown-prop-2", { { "child", "object" } } }
		}
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}
