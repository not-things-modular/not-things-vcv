#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptGlobalActions, ParseScriptWithNoGlobalActionsShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	EXPECT_EQ(script->globalActions.size(), 0u);
}

TEST(TimeSeqJsonScriptGlobalActions, ParseScriptWithEmptyGlobalActionsShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["global-actions"] = json::array();

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	EXPECT_EQ(script->globalActions.size(), 0u);
}

TEST(TimeSeqJsonScriptGlobalActions, ParseScriptWithNonArrayGlobalActionsShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["global-actions"] = "not-an-array";

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_GlobalActionsArray, "/");
}

TEST(TimeSeqJsonScriptGlobalActions, ParseScriptWithNonObjectActionShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["global-actions"] = json::array({
		{ { "ref", "action-1"} },
		"not-an-object",
		{ { "ref", "action-3"} }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_GlobalActionsObject, "/global-actions/1");
}

TEST(TimeSeqJsonScriptGlobalActions, ParseScriptShouldParseActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["global-actions"] = json::array({
		{ { "ref", "action-1"} },
		{ { "ref", "action-2"} },
		{ { "ref", "action-3"} }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->globalActions.size(), 3u);
	EXPECT_EQ(script->globalActions[0].ref, "action-1");
	EXPECT_EQ(script->globalActions[1].ref, "action-2");
	EXPECT_EQ(script->globalActions[2].ref, "action-3");
}
