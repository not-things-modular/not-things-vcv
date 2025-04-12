#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptAction, ParseShouldSucceedWithoutActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	EXPECT_EQ(script->actions.size(), 0u);
}

TEST(TimeSeqJsonScriptAction, ParseShouldSucceedWithEmptyActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_ActionObject, "/component-pool/actions/1");
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_SetPolyphonyObject, "/component-pool/actions/0");
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetValue_ValueObject, "/component-pool/actions/0/set-value");
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].setValue);
	EXPECT_EQ(script->actions[0].setValue->value.ref, "value-ref");
	EXPECT_EQ(script->actions[0].setValue->output.ref, "output-ref");
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SetVariable_ValueObject, "/component-pool/actions/0/set-variable");
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->actions.size(), 16u);
	for (int i = 0; i < 16; i++) {
		ASSERT_TRUE(script->actions[i].setPolyphony);
		ASSERT_EQ(script->actions[i].setPolyphony->channels, 16 - i);
		ASSERT_EQ(script->actions[i].setPolyphony->index, (i / 2) + 1);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].assert);
	EXPECT_TRUE(script->actions[0].assert->stopOnFail);
	EXPECT_EQ(script->actions[0].assert->name, "assert-name");
	EXPECT_EQ(script->actions[0].assert->expect.ifOperator, ScriptIf::IfOperator::EQ);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonObjectStartValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", "not-an-object" }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_StartValueObject, "/component-pool/actions/0");
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Id_NotAllowed, "/component-pool/actions/0/start-value");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnNonObjectEndValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "glide" }, { "end-value", "not-an-object" }, { "start-value", { { "ref", "ref-start-value" } } }, { "variable", "output-variable" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Action_EndValueObject, "/component-pool/actions/0");
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Id_NotAllowed, "/component-pool/actions/0/output");
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "set-variable", { { "dummy", "value" } } } },
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "set-polyphony", { { "dummy", "value" } } } },
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "assert", { { "dummy", "value" } } } },
			{ { "id", "action-1" }, { "timing", "glide" }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" }, { "trigger", "trigger-name" } },
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 4u);
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/0");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/1");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/2");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/3");
	expectError(validationErrors, ValidationErrorCode::Action_NonGlideProperties, "/component-pool/actions/4");
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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
			{ { "id", "action-1" }, { "timing", "glide" }, { "ease-factor", 5.1 }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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
			{ { "id", "action-1" }, { "timing", "glide" }, { "ease-factor", 0 }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } },
			{ { "id", "action-1" }, { "timing", "glide" }, { "ease-factor", 3.5 }, { "start-value", { { "ref", "ref-start-value" } } }, { "end-value", { { "ref", "ref-end-value" } } }, { "variable", "variable-name" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->actions.size(), 1u);
	EXPECT_TRUE(script->actions[0].easeAlgorithm);
	EXPECT_EQ(*script->actions[0].easeAlgorithm.get(), ScriptAction::EaseAlgorithm::POW);
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnMissingNonGlideAction) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_MissingNonGlideProperties, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnTooManyActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "set-value", { { "dummy", "value" } } }, { "set-variable", { { "dummy", "value" } } } },
			{ { "id", "action-1" }, { "set-value", { { "dummy", "value" } } }, { "set-polyphony", { { "dummy", "value" } } } },
			{ { "id", "action-1" }, { "set-value", { { "dummy", "value" } } }, { "assert", { { "dummy", "value" } } } },
			{ { "id", "action-1" }, { "set-value", { { "dummy", "value" } } }, { "trigger", "trigger-name" } },

			{ { "id", "action-1" }, { "set-variable", { { "dummy", "value" } } }, { "set-polyphony", { { "dummy", "value" } } } },
			{ { "id", "action-1" }, { "set-variable", { { "dummy", "value" } } }, { "assert", { { "dummy", "value" } } } },
			{ { "id", "action-1" }, { "set-variable", { { "dummy", "value" } } }, { "trigger", "trigger-name" } },

			{ { "id", "action-1" }, { "set-polyphony", { { "dummy", "value" } } }, { "assert", { { "dummy", "value" } } } },
			{ { "id", "action-1" }, { "set-polyphony", { { "dummy", "value" } } }, { "trigger", "trigger-name" } },

			{ { "id", "action-1" }, { "assert", { { "dummy", "value" } } }, { "trigger", "trigger-name" } },
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 9u) << json.dump();
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
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 6u);
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/0");
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/1");
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/2");
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/3");
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/4");
	expectError(validationErrors, ValidationErrorCode::Action_GlidePropertiesOnNonGlideAction, "/component-pool/actions/5");
}

TEST(TimeSeqJsonScriptAction, ParseActionsShouldFailOnRefCombinedWithOtherProperties) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["global-actions"] = json::array({
		{ { "ref", "action-ref"}, { "output", { { "ref", "output-ref" } } } },
		{ { "ref", "action-ref"}, { "variable", "variable-name" } },
		{ { "ref", "action-ref"}, { "start-value", { { "ref", "value-ref" } } } },
		{ { "ref", "action-ref"}, { "end-value", { { "ref", "value-ref" } } } },
		{ { "ref", "action-ref"}, { "ease-factor", 1 } },
		{ { "ref", "action-ref"}, { "ease-algorithm", "pow" } },
		{ { "ref", "action-ref"}, { "if", { { "dummy", "value" } } } },
		{ { "ref", "action-ref"}, { "timing", "start" } },
		{ { "ref", "action-ref"}, { "set-value", { { "dummy", "value" } } } },
		{ { "ref", "action-ref"}, { "set-variable", { { "dummy", "value" } } } },
		{ { "ref", "action-ref"}, { "set-polyphony", { { "dummy", "value" } } } },
		{ { "ref", "action-ref"}, { "assert", { { "dummy", "value" } } } },
		{ { "ref", "action-ref"}, { "trigger", "trigger-name" } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/1");
	expectError(validationErrors, ValidationErrorCode::Action_RefOrInstance, "/global-actions/12");
}
