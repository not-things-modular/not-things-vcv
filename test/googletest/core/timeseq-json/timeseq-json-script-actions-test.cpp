#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptAction, ParseShouldSucceedWithoutActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->actions.size(), 0u);
}

TEST(TimeSeqJsonScriptAction, ParseShouldSucceedWithEmptyActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->values.size(), 0u);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldNotAllowRefAndRequireIdOnRoot) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "ref", "action-ref" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/component-pool/actions/0");
	expectError(validationErrors, ValidationErrorCode::Ref_NotAllowed, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonArrayActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", "not-an-array" }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_ActionsArray, "/component-pool");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonObjectAction) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
			"not-an-object",
			{ { "id", "action-2" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_ActionObject, "/component-pool/actions/1");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnDuplicateIds) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
			{ { "id", "action-2" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
			{ { "id", "action-1" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
			{ { "id", "action-3" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
			{ { "id", "action-2" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
			{ { "id", "action-1" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/actions/2");
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/actions/4");
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/actions/5");

	EXPECT_NE(validationErrors[0].message.find("'action-1'"), std::string::npos);
	EXPECT_NE(validationErrors[1].message.find("'action-2'"), std::string::npos);
	EXPECT_NE(validationErrors[2].message.find("'action-1'"), std::string::npos);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldDefaultToStartTiming) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_EQ(script->actions[0].timing, ScriptAction::ActionTiming::START);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseStartTiming) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "start" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_EQ(script->actions[0].timing, ScriptAction::ActionTiming::START);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseEndTiming) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "end" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_EQ(script->actions[0].timing, ScriptAction::ActionTiming::END);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnUnknownTiming) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "dunno" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_TimingEnum, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonObjectSetValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", "not-an-object" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_SetValueObject, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonObjectSetVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", "not-an-object" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_SetVariableObject, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonObjectSetPolyphony) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-polyphony", "not-an-object" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_SetPolyphonyObject, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonObjectSetLabel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-label", "not-an-object" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_SetLabelObject, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonObjectAssert) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", "not-an-object" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_AssertObject, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingSetValueOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "value", { { "ref", "value-ref" } } } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetValue_OutputObject, "/component-pool/actions/0/set-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingSetValueValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "output", { { "ref", "output-ref" } } } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetValue_ValueObject, "/component-pool/actions/0/set-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnShorthandSetValueOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "output", { { "ref", "output-ref" } } }, { "value", -10.01 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/actions/0/set-value/value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseFloatShorthandSetValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "output", { { "ref", "output-ref" } } }, { "value", -6.6 } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].setValue);
	EXPECT_TRUE(script->actions[0].setValue->value.voltage);
	EXPECT_EQ(*script->actions[0].setValue->value.voltage, -6.6f);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonNoteStringShorthandSetValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "output", { { "ref", "output-ref" } } }, { "value", "4D+" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/actions/0/set-value/value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseNoteShorthandSetValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "output", { { "ref", "output-ref" } } }, { "value", "D4+" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].setValue);
	EXPECT_TRUE(script->actions[0].setValue->value.note);
	EXPECT_EQ(*script->actions[0].setValue->value.note, "D4+");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseSetValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "value", { { "ref", "value-ref" } } }, { "output", { { "ref", "output-ref" } } } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].setValue);
	EXPECT_EQ(script->actions[0].setValue->value.ref, "value-ref");
	EXPECT_EQ(script->actions[0].setValue->output.ref, "output-ref");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseSetValueWithShorthandOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "value", { { "ref", "value-ref" } } }, { "output", 3 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].setValue);
	EXPECT_EQ(script->actions[0].setValue->value.ref, "value-ref");
	EXPECT_EQ(script->actions[0].setValue->output.index, 3);
	EXPECT_FALSE(script->actions[0].setValue->output.channel);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnSetValueWithFloatShorthandOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "value", { { "ref", "value-ref" } } }, { "output", 3.3 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexNumber, "/component-pool/actions/0/set-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnSetvalueWithShorthandOutputOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "value", { { "ref", "value-ref" } } }, { "output", 9 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexRange, "/component-pool/actions/0/set-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingSetVariableName) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", { { "value", { { "ref", "value-ref" } } } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetVariable_NameString, "/component-pool/actions/0/set-variable");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnEmptySetVariableName) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", { { "name", "" }, { "value", { { "ref", "value-ref" } } } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetVariable_NameLength, "/component-pool/actions/0/set-variable");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingSetVariableValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", { { "name", "variable-name" } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetVariable_ValueObject, "/component-pool/actions/0/set-variable");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnShorthandSetVariableOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", { { "name", "variable-name" }, { "value", -10.01 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/actions/0/set-variable/value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseFloatShorthandSetVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", { { "name", "variable-name" }, { "value", -6.6 } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].setVariable);
	EXPECT_TRUE(script->actions[0].setVariable->value.voltage);
	EXPECT_EQ(*script->actions[0].setVariable->value.voltage, -6.6f);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonNoteStringShorthandSetVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", { { "name", "variable-name" }, { "value", "4D+" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/actions/0/set-variable/value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseNoteShorthandSetVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", { { "name", "variable-name" }, { "value", "D4+" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].setVariable);
	EXPECT_TRUE(script->actions[0].setVariable->value.note);
	EXPECT_EQ(*script->actions[0].setVariable->value.note, "D4+");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseSetVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "ref", "value-ref" } } } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].setVariable);
	EXPECT_EQ(script->actions[0].setVariable->name, "variable-name");
	EXPECT_EQ(script->actions[0].setVariable->value.ref, "value-ref");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingSetPolyphonyIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-polyphony", { { "channels", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetPolyphony_IndexNumber, "/component-pool/actions/0/set-polyphony");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonNumericSetPolyphonyIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-polyphony", { { "index", "not-a-number" }, { "channels", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetPolyphony_IndexNumber, "/component-pool/actions/0/set-polyphony");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnSetPolyphonyIndexOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-polyphony", { { "index", 0 }, { "channels", 1 } } } },
			{ { "id", "action-2" }, { "set-polyphony", { { "index", 9 }, { "channels", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::SetPolyphony_IndexRange, "/component-pool/actions/0/set-polyphony");
	expectError(validationErrors, ValidationErrorCode::SetPolyphony_IndexRange, "/component-pool/actions/1/set-polyphony");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonIntegerPolyphonyIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-polyphony", { { "index", 1.1 }, { "channels", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetPolyphony_IndexNumber, "/component-pool/actions/0/set-polyphony");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingSetPolyphonyChannels) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-polyphony", { { "index", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetPolyphony_ChannelsNumber, "/component-pool/actions/0/set-polyphony");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonNumericSetPolyphonyChannels) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-polyphony", { { "channels", "not-a-number" }, { "index", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetPolyphony_ChannelsNumber, "/component-pool/actions/0/set-polyphony");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnSetPolyphonyChannelsOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-polyphony", { { "channels", 0 }, { "index", 1 } } } },
			{ { "id", "action-2" }, { "set-polyphony", { { "channels", 17 }, { "index", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::SetPolyphony_ChannelsRange, "/component-pool/actions/0/set-polyphony");
	expectError(validationErrors, ValidationErrorCode::SetPolyphony_ChannelsRange, "/component-pool/actions/1/set-polyphony");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonIntegerChannelsIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-polyphony", { { "channels", 1.1 }, { "index", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetPolyphony_ChannelsNumber, "/component-pool/actions/0/set-polyphony");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseSetPolyphony) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-polyphony", { { "index", 1 }, { "channels", 16 } } } },
			{ { "id", "action-2" }, { "set-polyphony", { { "index", 1 }, { "channels", 15 } } } },
			{ { "id", "action-3" }, { "set-polyphony", { { "index", 2 }, { "channels", 14 } } } },
			{ { "id", "action-4" }, { "set-polyphony", { { "index", 2 }, { "channels", 13 } } } },
			{ { "id", "action-5" }, { "set-polyphony", { { "index", 3 }, { "channels", 12 } } } },
			{ { "id", "action-6" }, { "set-polyphony", { { "index", 3 }, { "channels", 11 } } } },
			{ { "id", "action-7" }, { "set-polyphony", { { "index", 4 }, { "channels", 10 } } } },
			{ { "id", "action-8" }, { "set-polyphony", { { "index", 4 }, { "channels", 9 } } } },
			{ { "id", "action-9" }, { "set-polyphony", { { "index", 5 }, { "channels", 8 } } } },
			{ { "id", "action-10" }, { "set-polyphony", { { "index", 5 }, { "channels", 7 } } } },
			{ { "id", "action-11" }, { "set-polyphony", { { "index", 6 }, { "channels", 6 } } } },
			{ { "id", "action-12" }, { "set-polyphony", { { "index", 6 }, { "channels", 5 } } } },
			{ { "id", "action-13" }, { "set-polyphony", { { "index", 7 }, { "channels", 4 } } } },
			{ { "id", "action-14" }, { "set-polyphony", { { "index", 7 }, { "channels", 3 } } } },
			{ { "id", "action-15" }, { "set-polyphony", { { "index", 8 }, { "channels", 2 } } } },
			{ { "id", "action-16" }, { "set-polyphony", { { "index", 8 }, { "channels", 1 } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 16u);
	for (int i = 0; i < 16; i++) {
		ASSERT_TRUE(script->actions[i].setPolyphony);
		ASSERT_EQ(script->actions[i].setPolyphony->channels, 16 - i);
		ASSERT_EQ(script->actions[i].setPolyphony->index, (i / 2) + 1);
	}
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingSetLabelIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-label", { { "label", "the-label" } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetLabel_IndexNumber, "/component-pool/actions/0/set-label");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonNumericSetLabelIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-label", { { "index", "not-a-number" }, { "label", "the-label" } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetLabel_IndexNumber, "/component-pool/actions/0/set-label");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnSetLabelIndexOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-label", { { "index", 0 }, { "label", "the-label-1" } } } },
			{ { "id", "action-2" }, { "set-label", { { "index", 9 }, { "label", "the-label-2" } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::SetLabel_IndexRange, "/component-pool/actions/0/set-label");
	expectError(validationErrors, ValidationErrorCode::SetLabel_IndexRange, "/component-pool/actions/1/set-label");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonIntegerLabelIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-label", { { "index", 1.1 }, { "label", "the-label" } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetLabel_IndexNumber, "/component-pool/actions/0/set-label");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingSetLabelLabel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-label", { { "index", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetLabel_LabelString, "/component-pool/actions/0/set-label");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonNumericSetLabelLabel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-label", { { "label", 3 }, { "index", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetLabel_LabelString, "/component-pool/actions/0/set-label");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnSetLabelEmptyLabel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-label", { { "label", "" }, { "index", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetLabel_LabelLength, "/component-pool/actions/0/set-label");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseSetLabel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-label", { { "index", 8 }, { "label", "label-8" } } } },
			{ { "id", "action-2" }, { "set-label", { { "index", 7 }, { "label", "label-7" } } } },
			{ { "id", "action-3" }, { "set-label", { { "index", 6 }, { "label", "label-6" } } } },
			{ { "id", "action-4" }, { "set-label", { { "index", 5 }, { "label", "label-5" } } } },
			{ { "id", "action-5" }, { "set-label", { { "index", 4 }, { "label", "label-4" } } } },
			{ { "id", "action-6" }, { "set-label", { { "index", 3 }, { "label", "label-3" } } } },
			{ { "id", "action-7" }, { "set-label", { { "index", 2 }, { "label", "label-2" } } } },
			{ { "id", "action-8" }, { "set-label", { { "index", 1 }, { "label", "label-1" } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 8u);
	for (int i = 0; i < 8; i++) {
		ASSERT_TRUE(script->actions[i].setLabel);
		ASSERT_EQ(script->actions[i].setLabel->index, 8 - i);
		ASSERT_EQ(script->actions[i].setLabel->label, "label-" + to_string(8 - i));
	}
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonStringTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", 1 } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_TriggerString, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnEmptyTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_TriggerLength, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	EXPECT_EQ(script->actions[0].trigger, "trigger-name");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnAssertWithMissingName) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", { { "expect", {
				{ "eq", json::array({
					{ { "ref", "value-ref-1" } },
					{ { "ref", "value-ref-2" } }
				}) }
			} } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Assert_NameString, "/component-pool/actions/0/assert");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnAssertWithNonStringName) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", { { "name", 9 }, { "expect", {
				{ "eq", json::array({
					{ { "ref", "value-ref-1" } },
					{ { "ref", "value-ref-2" } }
				}) }
			} } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Assert_NameString, "/component-pool/actions/0/assert");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnAssertWithEmptyName) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", { { "name", "" }, { "expect", {
				{ "eq", json::array({
					{ { "ref", "value-ref-1" } },
					{ { "ref", "value-ref-2" } }
				}) }
			} } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Assert_NameLength, "/component-pool/actions/0/assert");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingAssertExpect) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", { { "name", "assert-name" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Assert_ExpectObject, "/component-pool/actions/0/assert");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonObjectAssertExpect) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", { { "name", "assert-name" }, { "expect", "not-an-object" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Assert_ExpectObject, "/component-pool/actions/0/assert");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailWithNonBooleanStopOnFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", { { "name", "assert-name" }, { "stop-on-fail", "not-a-bool" }, { "expect", {
				{ "eq", json::array({
					{ { "ref", "value-ref-1" } },
					{ { "ref", "value-ref-2" } }
				}) }
			} } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Assert_StopOnFailBool, "/component-pool/actions/0/assert");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldDefaultAssertToStopOnFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", { { "name", "assert-name" }, { "expect", {
				{ "eq", json::array({
					{ { "ref", "value-ref-1" } },
					{ { "ref", "value-ref-2" } }
				}) }
			} } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].assert);
	EXPECT_TRUE(script->actions[0].assert->stopOnFail);
	EXPECT_EQ(script->actions[0].assert->name, "assert-name");
	EXPECT_EQ(script->actions[0].assert->expect.ifOperator, ScriptIf::IfOperator::EQ);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseAssertStopOnFailFalse) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", { { "name", "assert-name" }, { "stop-on-fail", false }, { "expect", {
				{ "eq", json::array({
					{ { "ref", "value-ref-1" } },
					{ { "ref", "value-ref-2" } }
				}) }
			} } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].assert);
	EXPECT_FALSE(script->actions[0].assert->stopOnFail);
	EXPECT_EQ(script->actions[0].assert->name, "assert-name");
	EXPECT_EQ(script->actions[0].assert->expect.ifOperator, ScriptIf::IfOperator::EQ);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseAssertStopOnFailTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", { { "name", "assert-name" }, { "stop-on-fail", true }, { "expect", {
				{ "eq", json::array({
					{ { "ref", "value-ref-1" } },
					{ { "ref", "value-ref-2" } }
				}) }
			} } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].assert);
	EXPECT_TRUE(script->actions[0].assert->stopOnFail);
	EXPECT_EQ(script->actions[0].assert->name, "assert-name");
	EXPECT_EQ(script->actions[0].assert->expect.ifOperator, ScriptIf::IfOperator::EQ);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonObjectStringOrNumberStartValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", json::array({}) }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_StartValueObject, "/component-pool/actions/0/start-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnInvalidStartValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "id", "not-allowed" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Id_NotAllowed, "/component-pool/actions/0/start-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnShorthandStartValueOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", 11.f }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/actions/0/start-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseFloatShorthandStartValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", 3.45f }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].startValue);
	ASSERT_TRUE(script->actions[0].startValue->voltage);
	EXPECT_EQ(*script->actions[0].startValue->voltage, 3.45f);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonNoteStringShorthandStartValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", "C4*" }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/actions/0/start-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseNoteShorthandStartValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", "F6+" }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].startValue);
	ASSERT_TRUE(script->actions[0].startValue->note);
	EXPECT_EQ(*script->actions[0].startValue->note, "F6+");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonObjectStringOrNumberEndValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "end-value", json::array({}) }, { "start-value", { { "ref", "ref-start-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_EndValueObject, "/component-pool/actions/0/end-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnShorthandEndValueOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "end-value", 11.f }, { "start-value", { { "ref", "ref-start-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/actions/0/end-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseFloatShorthandEndValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "end-value", 3.45f }, { "start-value", { { "ref", "ref-start-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].endValue);
	ASSERT_TRUE(script->actions[0].endValue->voltage);
	EXPECT_EQ(*script->actions[0].endValue->voltage, 3.45f);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonNoteStringShorthandEndValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "end-value", "C4*" }, { "start-value", { { "ref", "ref-start-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/actions/0/end-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseNoteShorthandEndValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "end-value", "F6+" }, { "start-value", { { "ref", "ref-start-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].endValue);
	ASSERT_TRUE(script->actions[0].endValue->note);
	EXPECT_EQ(*script->actions[0].endValue->note, "F6+");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnInvalidEndValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "end-value", { { "id", "not-allowed" } } }, { "start-value", { { "ref", "ref-start-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Id_NotAllowed, "/component-pool/actions/0/end-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailNonObjectOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "output", "not-an-object" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_OutputObject, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailWithInvalidOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "output", { { "id", "not-allowed" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Id_NotAllowed, "/component-pool/actions/0/output");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailWithFloatShorthandOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "output", 2.4 } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexNumber, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailWithShorthandOutputOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "output", 9 } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexRange, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseShorthandOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "output", 7 } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].output);
	EXPECT_EQ(script->actions[0].output->index, 7);
	EXPECT_FALSE(script->actions[0].output->channel);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailNonStringVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", 1 } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_VariableString, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailEmptyStringVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_VariableLength, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailWithNoOutputOrVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_MissingGlideActions, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingStartValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_MissingGlideValues, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingEndValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_MissingGlideValues, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingOutputAndVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_MissingGlideActions, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonGlideActionsCombinedWithGlideTiming) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "set-value", { { "dummy", "value" } } } },
			{ { "id", "action-2" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "set-variable", { { "dummy", "value" } } } },
			{ { "id", "action-3" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "set-polyphony", { { "dummy", "value" } } } },
			{ { "id", "action-4" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "assert", { { "dummy", "value" } } } },
			{ { "id", "action-5" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "trigger", "trigger-name" } },
			{ { "id", "action-6" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "gate-high-ratio", 0.1f } },
			{ { "id", "action-7" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "move-sequence", { { "id", "seq" } } } },
			{ { "id", "action-8" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "clear-sequence", "seq" } },
			{ { "id", "action-9" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "add-to-sequence", { { "id", "seq" }, { "value", 1 } } } },
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 9u);
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/0");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/1");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/2");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/3");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/4");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/5");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/6");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/7");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/8");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonGateActionsCombinedWithGateTiming) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "gate" }, { "output", { { "ref", "output-id" } } }, { "set-value", { { "dummy", "value" } } } },
			{ { "id", "action-2" }, { "timing", "gate" }, { "output", { { "ref", "output-id" } } }, { "set-variable", { { "dummy", "value" } } } },
			{ { "id", "action-3" }, { "timing", "gate" }, { "output", { { "ref", "output-id" } } }, { "set-polyphony", { { "dummy", "value" } } } },
			{ { "id", "action-4" }, { "timing", "gate" }, { "output", { { "ref", "output-id" } } }, { "assert", { { "dummy", "value" } } } },
			{ { "id", "action-5" }, { "timing", "gate" }, { "output", { { "ref", "output-id" } } }, { "trigger", "trigger-name" } },
			{ { "id", "action-6" }, { "timing", "gate" }, { "output", { { "ref", "output-id" } } }, { "variable", "variable-name" } },
			{ { "id", "action-7" }, { "timing", "gate" }, { "output", { { "ref", "output-id" } } }, { "start-value", { { "ref", "value-ref" } } } },
			{ { "id", "action-8" }, { "timing", "gate" }, { "output", { { "ref", "output-id" } } }, { "end-value", { { "ref", "value-ref" } } } },
			{ { "id", "action-9" }, { "timing", "gate" }, { "output", { { "ref", "output-id" } } }, { "ease-factor", 1 } },
			{ { "id", "action-10" }, { "timing", "gate" }, { "output", { { "ref", "output-id" } } }, { "ease-algorithm", "pow" } },
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 10u);
	expectError(validationErrors, ValidationErrorCode::Action_NonGateProperties, "/component-pool/actions/0");
	expectError(validationErrors, ValidationErrorCode::Action_NonGateProperties, "/component-pool/actions/1");
	expectError(validationErrors, ValidationErrorCode::Action_NonGateProperties, "/component-pool/actions/2");
	expectError(validationErrors, ValidationErrorCode::Action_NonGateProperties, "/component-pool/actions/3");
	expectError(validationErrors, ValidationErrorCode::Action_NonGateProperties, "/component-pool/actions/4");
	expectError(validationErrors, ValidationErrorCode::Action_NonGateProperties, "/component-pool/actions/5");
	expectError(validationErrors, ValidationErrorCode::Action_NonGateProperties, "/component-pool/actions/6");
	expectError(validationErrors, ValidationErrorCode::Action_NonGateProperties, "/component-pool/actions/7");
	expectError(validationErrors, ValidationErrorCode::Action_NonGateProperties, "/component-pool/actions/8");
	expectError(validationErrors, ValidationErrorCode::Action_NonGateProperties, "/component-pool/actions/9");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnBothVariableAndOutputActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "output", { { "ref", "output-ref" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_TooManyGlideActions, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseGlideActionWithNoEaseAndVariableAction) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_EQ(script->actions[0].id, "action-1");
	EXPECT_EQ(script->actions[0].timing, ScriptAction::ActionTiming::GLIDE);
	ASSERT_TRUE(script->actions[0].startValue);
	EXPECT_EQ(script->actions[0].startValue->ref, "ref-start-value");
	ASSERT_TRUE(script->actions[0].endValue);
	EXPECT_EQ(script->actions[0].endValue->ref, "ref-end-value");
	EXPECT_FALSE(script->actions[0].output);
	EXPECT_EQ(script->actions[0].variable, "variable-name");
	EXPECT_FALSE(script->actions[0].easeFactor);
	EXPECT_FALSE(script->actions[0].easeAlgorithm);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseGlideActionWithNoEaseAndOutputAction) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "output", { { "ref", "output-ref" }} } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_EQ(script->actions[0].id, "action-1");
	EXPECT_EQ(script->actions[0].timing, ScriptAction::ActionTiming::GLIDE);
	ASSERT_TRUE(script->actions[0].startValue);
	EXPECT_EQ(script->actions[0].startValue->ref, "ref-start-value");
	ASSERT_TRUE(script->actions[0].endValue);
	EXPECT_EQ(script->actions[0].endValue->ref, "ref-end-value");
	EXPECT_EQ(script->actions[0].variable, "");
	ASSERT_TRUE(script->actions[0].output);
	EXPECT_EQ(script->actions[0].output->ref, "output-ref");
	EXPECT_FALSE(script->actions[0].easeFactor);
	EXPECT_FALSE(script->actions[0].easeAlgorithm);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailWithNoneNumericEaseFactor) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "ease-factor", "not-a-number" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_EaseFactorFloat, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailWithEaseFactorOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "ease-factor", -5.1 }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } },
			{ { "id", "action-2" }, { "timing", "glide" }, { "ease-factor", 5.1 }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Action_EaseFactorRange, "/component-pool/actions/0");
	expectError(validationErrors, ValidationErrorCode::Action_EaseFactorRange, "/component-pool/actions/1");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseEaseFactor) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "ease-factor", -4 }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } },
			{ { "id", "action-2" }, { "timing", "glide" }, { "ease-factor", 0 }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } },
			{ { "id", "action-3" }, { "timing", "glide" }, { "ease-factor", 3.5 }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 3u);
	EXPECT_TRUE(script->actions[0].easeFactor);
	EXPECT_EQ(*script->actions[0].easeFactor.get(), -4);
	EXPECT_TRUE(script->actions[1].easeFactor);
	EXPECT_EQ(*script->actions[1].easeFactor.get(), 0);
	EXPECT_TRUE(script->actions[2].easeFactor);
	EXPECT_EQ(*script->actions[2].easeFactor.get(), 3.5);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailWithNonStringEaseAlgorithm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "ease-algorithm", -4 }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_EaseAlgorithmEnum, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailWithUnknownEaseAlgorithm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "ease-algorithm", "dunno" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_EaseAlgorithmEnum, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseSigEaseAlgorithm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "ease-algorithm", "sig" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	EXPECT_TRUE(script->actions[0].easeAlgorithm);
	EXPECT_EQ(*script->actions[0].easeAlgorithm.get(), ScriptAction::EaseAlgorithm::SIG);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParsePowEaseAlgorithm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "ease-algorithm", "pow" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	EXPECT_TRUE(script->actions[0].easeAlgorithm);
	EXPECT_EQ(*script->actions[0].easeAlgorithm.get(), ScriptAction::EaseAlgorithm::POW);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailWithNoneNumericGateHighRatio) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "gate" }, { "gate-high-ratio", "not-a-number" }, { "output", { { "ref", "output-id" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_GateHighRatioFloat, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailWithGateHighRatioOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "gate" }, { "gate-high-ratio", -0.01 }, { "output", { { "ref", "output-id" } } } },
			{ { "id", "action-2" }, { "timing", "gate" }, { "gate-high-ratio", 1.01 }, { "output", { { "ref", "output-id" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Action_GateHighRatioRange, "/component-pool/actions/0");
	expectError(validationErrors, ValidationErrorCode::Action_GateHighRatioRange, "/component-pool/actions/1");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingNonGlideAction) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			// Test for the three possible timings (absent, start & end) to validate that the error message is constructed accordingly
			{ { "id", "action-1" } },
			{ { "id", "action-2" }, { "timing", "start" } },
			{ { "id", "action-3" }, { "timing", "end" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Action_MissingNonGlideProperties, "/component-pool/actions/0");
	expectError(validationErrors, ValidationErrorCode::Action_MissingNonGlideProperties, "/component-pool/actions/1");
	expectError(validationErrors, ValidationErrorCode::Action_MissingNonGlideProperties, "/component-pool/actions/2");
	EXPECT_NE(validationErrors[0].message.find("'start'"), std::string::npos);
	EXPECT_NE(validationErrors[1].message.find("'start'"), std::string::npos);
	EXPECT_NE(validationErrors[2].message.find("'end'"), std::string::npos);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnTooManyActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "value", { { "ref", "value-ref" } } }, { "output", { { "ref", "output-ref" } } } } }, { "set-variable",  { { "value", { { "ref", "value-ref" } } }, { "name", "variable-name" } } } },
			{ { "id", "action-2" }, { "timing", "start" }, { "set-value", { { "value", { { "ref", "value-ref" } } }, { "output", { { "ref", "output-ref" } } } } }, { "set-polyphony", { { "index", 1 }, { "channels", 1 } } } },
			{ { "id", "action-3" }, { "timing", "end" }, { "set-value", { { "value", { { "ref", "value-ref" } } }, { "output", { { "ref", "output-ref" } } } } }, { "assert", { { "name", "assert-name" }, { "expect",  { { "eq", json::array({ { { "ref", "ref-value-1" } }, { { "ref", "ref-value-2" } }  }) } } } } } },
			{ { "id", "action-4" }, { "set-value", { { "dummy", "value" } } }, { "trigger", "trigger-name" } },
			{ { "id", "action-5" }, { "set-value", { { "dummy", "value" } } }, { "move-sequence", "sequence" } },
			{ { "id", "action-6" }, { "set-value", { { "dummy", "value" } } }, { "clear-sequence", "sequence" } },
			{ { "id", "action-7" }, { "set-value", { { "dummy", "value" } } }, { "add-to-sequence", { { "id", "sequence" }, { "value", 1.f } } } },

			{ { "id", "action-8" }, { "set-variable", { { "dummy", "value" } } }, { "set-polyphony", { { "dummy", "value" } } } },
			{ { "id", "action-9" }, { "set-variable", { { "dummy", "value" } } }, { "assert", { { "dummy", "value" } } } },
			{ { "id", "action-10" }, { "set-variable", { { "dummy", "value" } } }, { "trigger", "trigger-name" } },
			{ { "id", "action-11" }, { "set-variable", { { "dummy", "value" } } }, { "move-sequence", "sequence" } },
			{ { "id", "action-12" }, { "set-variable", { { "dummy", "value" } } }, { "clear-sequence", "sequence" } },
			{ { "id", "action-13" }, { "set-variable", { { "dummy", "value" } } }, { "add-to-sequence", { { "id", "sequence" }, { "value", 1.f } } } },

			{ { "id", "action-14" }, { "set-polyphony", { { "dummy", "value" } } }, { "assert", { { "dummy", "value" } } } },
			{ { "id", "action-15" }, { "set-polyphony", { { "dummy", "value" } } }, { "trigger", "trigger-name" } },
			{ { "id", "action-16" }, { "set-polyphony", { { "dummy", "value" } } }, { "move-sequence", "sequence" } },
			{ { "id", "action-17" }, { "set-polyphony", { { "dummy", "value" } } }, { "clear-sequence", "sequence" } },
			{ { "id", "action-18" }, { "set-polyphony", { { "dummy", "value" } } }, { "add-to-sequence", { { "id", "sequence" }, { "value", 1.f } } } },

			{ { "id", "action-19" }, { "assert", { { "dummy", "value" } } }, { "trigger", "trigger-name" } },
			{ { "id", "action-20" }, { "assert", { { "dummy", "value" } } }, { "move-sequence", "sequence" } },
			{ { "id", "action-21" }, { "assert", { { "dummy", "value" } } }, { "clear-sequence", "sequence" } },
			{ { "id", "action-22" }, { "assert", { { "dummy", "value" } } }, { "add-to-sequence", { { "id", "sequence" }, { "value", 1.f } } } },

			{ { "id", "action-23" }, { "trigger", "trigger-name" }, { "move-sequence", "sequence" } },
			{ { "id", "action-24" }, { "trigger", "trigger-name" }, { "clear-sequence", "sequence" } },
			{ { "id", "action-25" }, { "trigger", "trigger-name" }, { "add-to-sequence", { { "id", "sequence" }, { "value", 1.f } } } },

			{ { "id", "action-26" }, { "move-sequence", "sequence" }, { "clear-sequence", "sequence" } },
			{ { "id", "action-27" }, { "move-sequence", "sequence" }, { "add-to-sequence", { { "id", "sequence" }, { "value", 1.f } } } },

			{ { "id", "action-28" }, { "clear-sequence", "sequence" }, { "add-to-sequence", { { "id", "sequence" }, { "value", 1.f } } } },
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 28u);
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/0");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/1");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/2");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/3");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/4");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/5");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/6");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/7");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/8");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/9");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/10");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/11");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/12");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/13");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/14");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/15");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/16");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/17");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/18");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/19");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/20");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/21");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/22");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/23");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/24");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/25");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/26");
	expectError(validationErrors, ValidationErrorCode::Action_TooManyNonGlideProperties, "/component-pool/actions/27");

	// Verify that the action timing is correctly embedded in the error message (using the first three test actions, which represent the possible applicable timing inputs: absent, start and end)
	EXPECT_NE(validationErrors[0].message.find("'start'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[1].message.find("'start'"), std::string::npos) << validationErrors[1].message;
	EXPECT_NE(validationErrors[2].message.find("'end'"), std::string::npos) << json.dump() << " " << validationErrors[2].message;
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnGlidePropertiesOnNonGlideAction) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "output", { { "ref", "output-ref" } } } },
			{ { "id", "action-2" }, { "trigger", "trigger-name" }, { "variable", "variable-name" } },
			{ { "id", "action-3" }, { "trigger", "trigger-name" }, { "start-value", { { "ref", "value-ref" } } } },
			{ { "id", "action-4" }, { "trigger", "trigger-name" }, { "end-value", { { "ref", "value-ref" } } } },
			{ { "id", "action-5" }, { "trigger", "trigger-name" }, { "ease-factor", 1 } },
			{ { "id", "action-6" }, { "trigger", "trigger-name" }, { "ease-algorithm", "pow" } },
			{ { "id", "action-7" }, { "trigger", "trigger-name" }, { "gate-high-ratio", 0.2f } },
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 7u);
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/0");
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/1");
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/2");
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/3");
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/4");
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/5");
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/6");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnRefCombinedWithOtherProperties) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["global-actions"] = json::array({
		{ { "ref", "action-ref" }, { "output", { { "ref", "output-ref" } } } },
		{ { "ref", "action-ref" }, { "variable", "variable-name" } },
		{ { "ref", "action-ref" }, { "start-value", { { "ref", "value-ref" } } } },
		{ { "ref", "action-ref" }, { "end-value", { { "ref", "value-ref" } } } },
		{ { "ref", "action-ref" }, { "ease-factor", 1 } },
		{ { "ref", "action-ref" }, { "ease-algorithm", "pow" } },
		{ { "ref", "action-ref" }, { "if", { { "dummy", "value" } } } },
		{ { "ref", "action-ref" }, { "timing", "start" } },
		{ { "ref", "action-ref" }, { "set-value", { { "dummy", "value" } } } },
		{ { "ref", "action-ref" }, { "set-variable", { { "dummy", "value" } } } },
		{ { "ref", "action-ref" }, { "set-polyphony", { { "dummy", "value" } } } },
		{ { "ref", "action-ref" }, { "assert", { { "dummy", "value" } } } },
		{ { "ref", "action-ref" }, { "trigger", "trigger-name" } },
		{ { "ref", "action-ref" }, { "gate-high-ratio", 0.1f } },
		{ { "ref", "action-ref" }, { "move-sequence", "sequence-id" } },
		{ { "ref", "action-ref" }, { "clear-sequence", "sequence-id" } },
		{ { "ref", "action-ref" }, { "add-to-sequence", { { "id", "sequence-id" }, { "value", 1.f } } } },
		{ { "ref", "action-ref" }, { "remove-from-sequence", { { "id", "sequence-id" }, { "position", 1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 11u);
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/0");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/1");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/2");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/3");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/4");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/5");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/7");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/8");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/9");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/10");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/11");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/12");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/13");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/14");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/15");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/16");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseGateActionWithNoGateHighRatio) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "gate" }, { "output", { { "ref", "ref-output" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_EQ(script->actions[0].id, "action-1");
	EXPECT_EQ(script->actions[0].timing, ScriptAction::ActionTiming::GATE);
	EXPECT_FALSE(script->actions[0].gateHighRatio);
	ASSERT_TRUE(script->actions[0].output);
	EXPECT_EQ(script->actions[0].output->ref, "ref-output");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldParseGateActionWithGateHighRatio) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "gate" }, { "output", { { "ref", "ref-output" } } }, { "gate-high-ratio", .69f } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_EQ(script->actions[0].id, "action-1");
	EXPECT_EQ(script->actions[0].timing, ScriptAction::ActionTiming::GATE);
	ASSERT_TRUE(script->actions[0].gateHighRatio);
	EXPECT_EQ(*script->actions[0].gateHighRatio, .69f);
	ASSERT_TRUE(script->actions[0].output);
	EXPECT_EQ(script->actions[0].output->ref, "ref-output");
}

TEST(TimeSeqJsonScriptAction, ParseActionWithUnknownPropertyShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "trigger", "trigger-name" },
				{ "unknown-prop", "value" }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseActionWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "trigger", "trigger-name" },
				{ "unknown-prop-1", "value" },
				{ "unknown-prop-2", { { "child", "object" } } }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseActionShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "trigger", "trigger-name" },
				{ "x-unknown-prop-1", "value" },
				{ "x-unknown-prop-2", { { "child", "object" } } }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}

TEST(TimeSeqJsonScriptAction, ParseSetValueActionWithUnknownPropertyShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "set-value", {
					{ "value", { { "ref", "value-ref" } } },
					{ "output", { { "ref", "output-ref" } } },
					{ "unknown-prop", "value" }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0/set-value");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseSetValueActionWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "set-value", {
					{ "value", { { "ref", "value-ref" } } },
					{ "output", { { "ref", "output-ref" } } },
					{ "unknown-prop-1", "value" },
					{ "unknown-prop-2", { { "child", "object" } } }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0/set-value");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseSetValueActionShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "set-value", {
					{ "value", { { "ref", "value-ref" } } },
					{ "output", { { "ref", "output-ref" } } },
					{ "x-unknown-prop-1", "value" },
					{ "x-unknown-prop-2", { { "child", "object" } } }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}

TEST(TimeSeqJsonScriptAction, ParseSetVariableActionWithUnknownPropertyShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "set-variable", {
					{ "name", "variable-name" },
					{ "value", { { "output", { { "ref", "output-ref" } } } } },
					{ "unknown-prop", "value" }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0/set-variable");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseSetVariableActionWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "set-variable", {
					{ "name", "variable-name" },
					{ "value", { { "output", { { "ref", "output-ref" } } } } },
					{ "unknown-prop-1", "value" },
					{ "unknown-prop-2", { { "child", "object" } } }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0/set-variable");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseSetVariableActionShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "set-variable", {
					{ "name", "variable-name" },
					{ "value", { { "output", { { "ref", "output-ref" } } } } },
					{ "x-unknown-prop-1", "value" },
					{ "x-unknown-prop-2", { { "child", "object" } } }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}

TEST(TimeSeqJsonScriptAction, ParseSetPolyphonyActionWithUnknownPropertyShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "set-polyphony", {
					{ "index", 1 },
					{ "channels", 1 },
					{ "unknown-prop", "value" }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0/set-polyphony");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseSetPolyphonyActionWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "set-polyphony", {
					{ "index", 1 },
					{ "channels", 1 },
					{ "unknown-prop-1", "value" },
					{ "unknown-prop-2", { { "child", "object" } } }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0/set-polyphony");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseSetPolyphonyActionShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "set-polyphony", {
					{ "index", 1 },
					{ "channels", 1 },
					{ "x-unknown-prop-1", "value" },
					{ "x-unknown-prop-2", { { "child", "object" } } }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}

TEST(TimeSeqJsonScriptAction, ParseAssertActionWithUnknownPropertyShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", {
				{ "name", "assert-name" },
				{ "expect", {
					{ "eq", json::array({
						{ { "ref", "value-ref-1" } },
						{ { "ref", "value-ref-2" } }
					}) }
				} },
				{ "unknown-prop", "value" }
			} } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0/assert");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseAssertActionWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", {
				{ "name", "assert-name" },
				{ "expect", {
					{ "eq", json::array({
						{ { "ref", "value-ref-1" } },
						{ { "ref", "value-ref-2" } }
					}) }
				} },
				{ "unknown-prop-1", "value" },
				{ "unknown-prop-2", { { "child", "object" } } }
			} } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0/assert");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseAssertActionShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "assert", {
				{ "name", "assert-name" },
				{ "expect", {
					{ "eq", json::array({
						{ { "ref", "value-ref-1" } },
						{ { "ref", "value-ref-2" } }
					}) }
				} },
				{ "x-unknown-prop-1", "value" },
				{ "x-unknown-prop-2", { { "child", "object" } } }
			} } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceActionWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", {
				{ "id", "sequence-id" },
				{ "unknown-prop-1", "value" },
				{ "unknown-prop-2", { { "child", "object" } } }
			} } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0/move-sequence");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceActionShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", {
				{ "id", "sequence-id" },
				{ "x-unknown-prop-1", "value" },
				{ "x-unknown-prop-2", { { "child", "object" } } }
			} } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}

TEST(TimeSeqJsonScriptAction, ParseAddToSequenceActionWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "add-to-sequence", {
				{ "id", "sequence-id" },
				{ "value", 1.f },
				{ "unknown-prop-1", "value" },
				{ "unknown-prop-2", { { "child", "object" } } }
			} } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0/add-to-sequence");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseAddToSequenceActionShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "add-to-sequence", {
				{ "id", "sequence-id" },
				{ "value", 1.f },
				{ "x-unknown-prop-1", "value" },
				{ "x-unknown-prop-2", { { "child", "object" } } }
			} } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}

TEST(TimeSeqJsonScriptAction, ParseRemoveFromSequenceActionWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "remove-from-sequence", {
				{ "id", "sequence-id" },
				{ "unknown-prop-1", "value" },
				{ "unknown-prop-2", { { "child", "object" } } }
			} } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/actions/0/remove-from-sequence");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptAction, ParseRemoveFromSequenceActionShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "remove-from-sequence", {
				{ "id", "sequence-id" },
				{ "x-unknown-prop-1", "value" },
				{ "x-unknown-prop-2", { { "child", "object" } } }
			} } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldRequireVersion120) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", {
				{ "id", "sequence-id" }
			} } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldFailOnNonObjectDefinition) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", "not-an-object" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GE(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_MoveSequenceObject, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldFailOnMissingId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "wrap", true } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GE(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::MoveSequence_IdString, "/component-pool/actions/0/move-sequence");
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldFailOnNonStringId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", true } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GE(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::MoveSequence_IdString, "/component-pool/actions/0/move-sequence");
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldFailOnEmptyId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", "" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GE(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::MoveSequence_IdLength, "/component-pool/actions/0/move-sequence");
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceWithOnlyIdShouldUseDefaultsForOtherProperties) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", "the-sequence" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].moveSequence);
	ASSERT_FALSE(script->actions[0].moveSequence->direction);
	EXPECT_TRUE(script->actions[0].moveSequence->wrap);
	EXPECT_FALSE(script->actions[0].moveSequence->position);
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldParseDirectionValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", "the-sequence" } } } },
			{ { "id", "action-2" }, { "move-sequence", { { "id", "the-sequence" }, { "direction", "forward" } } } },
			{ { "id", "action-3" }, { "move-sequence", { { "id", "the-sequence" }, { "direction", "backward" } } } },
			{ { "id", "action-4" }, { "move-sequence", { { "id", "the-sequence" }, { "direction", "random" } } } },
			{ { "id", "action-5" }, { "move-sequence", { { "id", "the-sequence" }, { "direction", "none" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 5u);
	ASSERT_TRUE(script->actions[0].moveSequence);
	EXPECT_FALSE(script->actions[0].moveSequence->direction);
	ASSERT_TRUE(script->actions[1].moveSequence);
	ASSERT_TRUE(script->actions[1].moveSequence->direction);
	EXPECT_EQ(*script->actions[1].moveSequence->direction, ScriptSequenceMoveDirection::FORWARD);
	ASSERT_TRUE(script->actions[2].moveSequence);
	ASSERT_TRUE(script->actions[2].moveSequence->direction);
	EXPECT_EQ(*script->actions[2].moveSequence->direction, ScriptSequenceMoveDirection::BACKWARD);
	ASSERT_TRUE(script->actions[3].moveSequence);
	ASSERT_TRUE(script->actions[3].moveSequence->direction);
	EXPECT_EQ(*script->actions[3].moveSequence->direction, ScriptSequenceMoveDirection::RANDOM);
	ASSERT_TRUE(script->actions[4].moveSequence);
	ASSERT_TRUE(script->actions[4].moveSequence->direction);
	EXPECT_EQ(*script->actions[4].moveSequence->direction, ScriptSequenceMoveDirection::NONE);

	EXPECT_FALSE(script->actions[0].moveSequence->position);
	EXPECT_FALSE(script->actions[1].moveSequence->position);
	EXPECT_FALSE(script->actions[2].moveSequence->position);
	EXPECT_FALSE(script->actions[3].moveSequence->position);
	EXPECT_FALSE(script->actions[4].moveSequence->position);
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldFailOnNonStringDirection) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", "the-sequence" }, { "direction", 1.f } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::MoveSequence_MoveDirectionString, "/component-pool/actions/0/move-sequence");
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldFailOnInvalidDirection) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", "the-sequence" }, { "direction", "randward" } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::MoveSequence_MoveDirectionEnum, "/component-pool/actions/0/move-sequence");
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldParsePosition) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", "the-sequence" }, { "position", 1 } } } },
			{ { "id", "action-2" }, { "move-sequence", { { "id", "the-sequence" }, { "position", 100 } } } },
			{ { "id", "action-3" }, { "move-sequence", { { "id", "the-sequence" }, { "position", 0 } } } },
			{ { "id", "action-4" }, { "move-sequence", { { "id", "the-sequence" }, { "position", -5 } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 4u);
	ASSERT_TRUE(script->actions[0].moveSequence);
	ASSERT_TRUE(script->actions[0].moveSequence->position);
	EXPECT_EQ(*script->actions[0].moveSequence->position, 1.f);
	ASSERT_TRUE(script->actions[1].moveSequence);
	ASSERT_TRUE(script->actions[1].moveSequence->position);
	EXPECT_EQ(*script->actions[1].moveSequence->position, 100.f);
	ASSERT_TRUE(script->actions[2].moveSequence);
	ASSERT_TRUE(script->actions[2].moveSequence->position);
	EXPECT_EQ(*script->actions[2].moveSequence->position, 0.f);
	ASSERT_TRUE(script->actions[3].moveSequence);
	ASSERT_TRUE(script->actions[3].moveSequence->position);
	EXPECT_EQ(*script->actions[3].moveSequence->position, -5.f);

	EXPECT_FALSE(script->actions[0].moveSequence->direction);
	EXPECT_FALSE(script->actions[1].moveSequence->direction);
	EXPECT_FALSE(script->actions[2].moveSequence->direction);
	EXPECT_FALSE(script->actions[3].moveSequence->direction);
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldFailOnNonInteger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", "the-sequence" }, { "position", "1" } } } },
			{ { "id", "action-2" }, { "move-sequence", { { "id", "the-sequence" }, { "position", 1.1f } } } },
			{ { "id", "action-3" }, { "move-sequence", { { "id", "the-sequence" }, { "position", json::array() } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::MoveSequence_PositionNumber, "/component-pool/actions/0/move-sequence");
	expectError(validationErrors, ValidationErrorCode::MoveSequence_PositionNumber, "/component-pool/actions/1/move-sequence");
	expectError(validationErrors, ValidationErrorCode::MoveSequence_PositionNumber, "/component-pool/actions/2/move-sequence");
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldParseWrapProperty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", "the-sequence" } } } },
			{ { "id", "action-2" }, { "move-sequence", { { "id", "the-sequence" }, { "wrap", true } } } },
			{ { "id", "action-3" }, { "move-sequence", { { "id", "the-sequence" }, { "wrap", false } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->actions.size(), 3u);
	ASSERT_TRUE(script->actions[0].moveSequence);
	ASSERT_TRUE(script->actions[0].moveSequence->wrap);
	ASSERT_TRUE(script->actions[1].moveSequence);
	ASSERT_TRUE(script->actions[1].moveSequence->wrap);
	ASSERT_TRUE(script->actions[2].moveSequence);
	ASSERT_FALSE(script->actions[2].moveSequence->wrap);
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldFailOnNonBoolenWrap) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", "the-sequence" }, { "wrap", "true" } } } },
			{ { "id", "action-2" }, { "move-sequence", { { "id", "the-sequence" }, { "wrap", json::array() } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::MoveSequence_WrapBoolean, "/component-pool/actions/0/move-sequence");
	expectError(validationErrors, ValidationErrorCode::MoveSequence_WrapBoolean, "/component-pool/actions/1/move-sequence");
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldFailWhenWrapIsCombinedWithPosition) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", "the-sequence" }, { "wrap", true }, { "position", 1 } } } },
			{ { "id", "action-2" }, { "move-sequence", { { "id", "the-sequence" }, { "wrap", false }, { "position", 1 } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::MoveSequence_NoWrapWithPosition, "/component-pool/actions/0/move-sequence");
	expectError(validationErrors, ValidationErrorCode::MoveSequence_NoWrapWithPosition, "/component-pool/actions/1/move-sequence");
}

TEST(TimeSeqJsonScriptAction, ParseMoveSequenceShouldFailWhenDirectionIsCombinedWithPosition) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "move-sequence", { { "id", "the-sequence" }, { "direction", "forward" }, { "position", 1 } } } },
			{ { "id", "action-2" }, { "move-sequence", { { "id", "the-sequence" }, { "direction", "backward" }, { "position", 1 } } } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::MoveSequence_EitherDirectionOrPosition, "/component-pool/actions/0/move-sequence");
	expectError(validationErrors, ValidationErrorCode::MoveSequence_EitherDirectionOrPosition, "/component-pool/actions/1/move-sequence");
}
