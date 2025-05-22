#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptValue, ParseShouldSucceedWithoutValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->values.size(), 0u);
}

TEST(TimeSeqJsonScriptValue, ParseShouldSucceedWithEmptyValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->values.size(), 0u);
}

TEST(TimeSeqJsonScriptValue, ParseValuesShouldNotAllowRefAndRequireIdOnRoot) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", {
			{ { "ref", "input-ref" } }
		} }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/component-pool/values/0");
	expectError(validationErrors, ValidationErrorCode::Ref_NotAllowed, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValuesShouldFailOnNonArrayValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", "not-an-array" }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_ValuesArray, "/component-pool");
}

TEST(TimeSeqJsonScriptValue, ParseValuesShouldFailOnNonObjectValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 1 } },
			"not-an-object",
			{ { "id", "value-2" }, { "voltage", 2 } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_ValueObject, "/component-pool/values/1");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithoutValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithNonNumericVoltage) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", "not-a-number" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageFloat, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithVoltageOutsideOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", -10.1 } },
			{ { "id", "value-2" }, { "voltage", 10.1 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/values/0");
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/values/1");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldSucceedWithIntegerVoltages) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", -10 } },
			{ { "id", "value-2" }, { "voltage", -4 } },
			{ { "id", "value-3" }, { "voltage", 0 } },
			{ { "id", "value-4" }, { "voltage", 6 } },
			{ { "id", "value-5" }, { "voltage", 10 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 5u);
	ASSERT_TRUE(script->values[0].voltage);
	EXPECT_EQ(*script->values[0].voltage.get(), -10);
	ASSERT_TRUE(script->values[1].voltage);
	EXPECT_EQ(*script->values[1].voltage.get(), -4);
	ASSERT_TRUE(script->values[2].voltage);
	EXPECT_EQ(*script->values[2].voltage.get(), 0);
	ASSERT_TRUE(script->values[3].voltage);
	EXPECT_EQ(*script->values[3].voltage.get(), 6);
	ASSERT_TRUE(script->values[4].voltage);
	EXPECT_EQ(*script->values[4].voltage.get(), 10);
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailOnDuplicateIds) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", -10 } },
			{ { "id", "value-2" }, { "voltage", -10 } },
			{ { "id", "value-3" }, { "voltage", -10 } },
			{ { "id", "value-1" }, { "voltage", -10 } },
			{ { "id", "value-2" }, { "voltage", -10 } },
			{ { "id", "value-3" }, { "voltage", -10 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/values/3");
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/values/4");
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/values/5");
	EXPECT_NE(validationErrors[0].message.find("'value-1'"), std::string::npos);
	EXPECT_NE(validationErrors[1].message.find("'value-2'"), std::string::npos);
	EXPECT_NE(validationErrors[2].message.find("'value-3'"), std::string::npos);
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldSucceedWithFloatVoltages) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", -10.0f } },
			{ { "id", "value-2" }, { "voltage", -9.999f } },
			{ { "id", "value-3" }, { "voltage", 0.1f } },
			{ { "id", "value-4" }, { "voltage", 4.99f } },
			{ { "id", "value-5" }, { "voltage", 10.0f } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 5u);
	ASSERT_TRUE(script->values[0].voltage);
	EXPECT_EQ(*script->values[0].voltage.get(), -10.0f);
	ASSERT_TRUE(script->values[1].voltage);
	EXPECT_EQ(*script->values[1].voltage.get(), -9.999f);
	ASSERT_TRUE(script->values[2].voltage);
	EXPECT_EQ(*script->values[2].voltage.get(), 0.1f);
	ASSERT_TRUE(script->values[3].voltage);
	EXPECT_EQ(*script->values[3].voltage.get(), 4.99f);
	ASSERT_TRUE(script->values[4].voltage);
	EXPECT_EQ(*script->values[4].voltage.get(), 10.0f);
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithNonStringNote) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "note", 1 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_NoteString, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithWrongLengthNote) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "note", "" } },
			{ { "id", "value-2" }, { "note", "a" } },
			{ { "id", "value-3" }, { "note", "a2+1" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/0");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/1");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/2");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithNoneNoteLetter) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	json["component-pool"] = {
		{ "values", json::array() }
	};

	for (int i = 0; i < 19; i++) {
		json["component-pool"]["values"].push_back(json::object({ { "id", std::string("value-") + std::to_string(i) }, { "note", std::string(1, 'h' + i) + "4" } }));
		json["component-pool"]["values"].push_back(json::object({ { "id", std::string("value-") + std::to_string(i + 19) }, { "note", std::string(1, 'H' + i) + "4" } }));
	}

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 38u);
	for (int i = 0; i < 38; i++) {
		expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, std::string("/component-pool/values/") + std::to_string(i));
	}
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithNoneNumericOctave) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "note", "aa" } },
			{ { "id", "value-2" }, { "note", "a-" } },
			{ { "id", "value-3" }, { "note", "a+" } },
			{ { "id", "value-4" }, { "note", "a " } },
			{ { "id", "value-5" }, { "note", "a$" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 5u);
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/0");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/1");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/2");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/3");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/4");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithInvalidAccidental) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "note", "a4a" } },
			{ { "id", "value-2" }, { "note", "a4 " } },
			{ { "id", "value-3" }, { "note", "a4/" } },
			{ { "id", "value-4" }, { "note", "a44" } },
			{ { "id", "value-5" }, { "note", "a4$" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 5u);
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/0");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/1");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/2");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/3");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/4");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseAllNotesWithoutAccidental) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	json["component-pool"] = {
		{ "values", json::array() }
	};

	for (int i = 0; i < 7; i++) {
		json["component-pool"]["values"].push_back(json::object({ { "id", std::string("value-") + std::to_string(i) }, { "note", std::string(1, 'a' + i) + "4" } }));
		json["component-pool"]["values"].push_back(json::object({ { "id", std::string("value-") + std::to_string(i + 19) }, { "note", std::string(1, 'A' + i) + "4" } }));
	}

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 14u);
	for (int i = 0; i < 7; i++) {
		ASSERT_TRUE(script->values[i].note);
		EXPECT_EQ(*script->values[i * 2].note.get(), std::string(1, 'a' + i) + "4");
		ASSERT_TRUE(script->values[i * 2 + 1].note);
		EXPECT_EQ(*script->values[i * 2 + 1].note.get(), std::string(1, 'A' + i) + "4");
	}
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseAllNotesWithAccidental) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	json["component-pool"] = {
		{ "values", json::array() }
	};

	for (int i = 0; i < 7; i++) {
		json["component-pool"]["values"].push_back(json::object({ { "id", std::string("value-") + std::to_string(i * 4) }, { "note", std::string(1, 'a' + i) + "4-" } }));
		json["component-pool"]["values"].push_back(json::object({ { "id", std::string("value-") + std::to_string(i * 4 + 1) }, { "note", std::string(1, 'a' + i) + "4+" } }));
		json["component-pool"]["values"].push_back(json::object({ { "id", std::string("value-") + std::to_string(i * 4 + 2) }, { "note", std::string(1, 'A' + i) + "4-" } }));
		json["component-pool"]["values"].push_back(json::object({ { "id", std::string("value-") + std::to_string(i * 4 + 3) }, { "note", std::string(1, 'A' + i) + "4+" } }));
	}

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 28u);
	for (int i = 0; i < 7; i++) {
		ASSERT_TRUE(script->values[i].note);
		EXPECT_EQ(*script->values[i * 4].note.get(), std::string(1, 'a' + i) + "4-");
		ASSERT_TRUE(script->values[i + 1].note);
		EXPECT_EQ(*script->values[i * 4 + 1].note.get(), std::string(1, 'a' + i) + "4+");
		ASSERT_TRUE(script->values[i * 4 + 2].note);
		EXPECT_EQ(*script->values[i * 4 + 2].note.get(), std::string(1, 'A' + i) + "4-");
		ASSERT_TRUE(script->values[i * 4 + 3].note);
		EXPECT_EQ(*script->values[i * 4 + 3].note.get(), std::string(1, 'A' + i) + "4+");
	}
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithNonStringVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "variable", 1 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VariableString, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithEmptyVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "variable", "" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VariableNonEmpty, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseVariable) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "variable", "variable-name" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].variable);
	EXPECT_EQ(*script->values[0].variable.get(), "variable-name");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithNonObjectInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "input", "1" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_InputObject, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithInvalidInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "input", { { "index", 9 } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_IndexRange, "/component-pool/values/0/input");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithFloatShorthandInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "input", 3.5 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_IndexNumber, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithShorthandInputOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "input", 9 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_IndexRange, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "input", { { "ref", "input-ref" } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].input);
	EXPECT_EQ(script->values[0].input.get()->ref, "input-ref");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseShorthandInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "input", 3 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].input);
	EXPECT_EQ(script->values[0].input.get()->index, 3);
	EXPECT_FALSE(script->values[0].input.get()->channel);
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithNonObjectOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "output", "1" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_OutputObject, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithInvalidOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "output", { { "index", 9 } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexRange, "/component-pool/values/0/output");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithFloatShorthandOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "output", 2.1 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexNumber, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithShorthandOutputOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "output", 9 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexRange, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "output", { { "ref", "output-ref" } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].output);
	EXPECT_EQ(script->values[0].output.get()->ref, "output-ref");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseShorthandOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "output", 4 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].output);
	EXPECT_EQ(script->values[0].output->index, 4);
	EXPECT_FALSE(script->values[0].output->channel);
}


TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithNonObjectRand) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", 1 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_RandObject, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithRandWithoutUpper) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "lower", { { "ref", "value-lower" } } } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Rand_UpperObject, "/component-pool/values/0/rand");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithRandNonValueUpper) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "lower", { { "ref", "value-lower" } } }, { "upper", json::array() } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Rand_UpperObject, "/component-pool/values/0/rand");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithRandInvalidUpperValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "lower", { { "ref", "value-lower" } } }, { "upper", { { "voltage", -11 } } } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/values/0/rand/upper");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithRandWithoutLower) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "upper", { { "ref", "value-upper" } } } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Rand_LowerObject, "/component-pool/values/0/rand");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithRandNonValueLower) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "upper", { { "ref", "value-upper" } } }, { "lower", json::array() } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Rand_LowerObject, "/component-pool/values/0/rand");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithRandInvalidLowerValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "upper", { { "ref", "value-upper" } } }, { "lower", { { "voltage", 11 } } } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/values/0/rand/lower");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailOnShorthandRandLowerValueOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "upper", { { "ref", "value-upper" } } }, { "lower", 11 } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/values/0/rand/lower");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseFloatShorthandRandLower) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "upper", { { "ref", "value-upper" } } }, { "lower", 9.9 } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].rand);
	ASSERT_TRUE(script->values[0].rand->lower);
	EXPECT_TRUE(script->values[0].rand->lower->voltage);
	EXPECT_EQ(*script->values[0].rand->lower->voltage, 9.9f);
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailOnNonNoteStringShorthandRandLower) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "upper", { { "ref", "value-upper" } } }, { "lower", "-D5" } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/0/rand/lower");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseNoteShorthandRandLower) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "upper", { { "ref", "value-upper" } } }, { "lower", "D5-" } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].rand);
	ASSERT_TRUE(script->values[0].rand->lower);
	ASSERT_TRUE(script->values[0].rand->lower->note);
	EXPECT_EQ(*script->values[0].rand->lower->note, "D5-");
}




TEST(TimeSeqJsonScriptValue, ParseValueShouldFailOnShorthandRandUpperValueOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "lower", { { "ref", "value-lower" } } }, { "upper", 11 } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/values/0/rand/upper");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseFloatShorthandRandUpper) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "lower", { { "ref", "value-lower" } } }, { "upper", 9.9 } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].rand);
	ASSERT_TRUE(script->values[0].rand->upper);
	EXPECT_TRUE(script->values[0].rand->upper->voltage);
	EXPECT_EQ(*script->values[0].rand->upper->voltage, 9.9f);
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailOnNonNoteStringShorthandRandUpper) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "lower", { { "ref", "value-lower" } } }, { "upper", "-D5" } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/0/rand/upper");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseNoteShorthandRandUpper) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "lower", { { "ref", "value-lower" } } }, { "upper", "D5-" } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].rand);
	ASSERT_TRUE(script->values[0].rand->upper);
	ASSERT_TRUE(script->values[0].rand->upper->note);
	EXPECT_EQ(*script->values[0].rand->upper->note, "D5-");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseRand) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", { { "upper", { { "ref", "value-upper" } } }, { "lower", { { "ref", "value-lower" } } } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].rand);
	ASSERT_TRUE(script->values[0].rand.get()->lower);
	EXPECT_EQ(script->values[0].rand.get()->lower.get()->ref, "value-lower");
	ASSERT_TRUE(script->values[0].rand.get()->upper);
	EXPECT_EQ(script->values[0].rand.get()->upper.get()->ref, "value-upper");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailOnMultipleValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		// Combine each of the possible values with each other
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 1 }, { "note", "a4" } },
			{ { "id", "value-2" }, { "voltage", 1 }, { "variable", "variable-name" } },
			{ { "id", "value-3" }, { "voltage", 1 }, { "input", { { "index", 1 } } } },
			{ { "id", "value-4" }, { "voltage", 1 }, { "output", { { "index", 1 } } } },
			{ { "id", "value-5" }, { "voltage", 1 }, { "rand", { { "lower", { { "voltage", 1 } } }, { "upper", { { "voltage", 1 } } } } } },

			{ { "id", "value-6" }, { "variable", "variable-name" }, { "input", { { "index", 1 } } } },
			{ { "id", "value-7" }, { "variable", "variable-name" }, { "output", { { "index", 1 } } } },
			{ { "id", "value-8" }, { "variable", "variable-name" }, { "rand", { { "lower", { { "voltage", 1 } } }, { "upper", { { "voltage", 1 } } } } } },

			{ { "id", "value-9" }, { "input", { { "index", 1 } } }, { "output", { { "index", 1 } } } },
			{ { "id", "value-10" }, { "input", { { "index", 1 } } }, { "rand", { { "lower", { { "voltage", 1 } } }, { "upper", { { "voltage", 1 } } } } } },

			{ { "id", "value-11" }, { "output", { { "index", 1 } } }, { "rand", { { "lower", { { "voltage", 1 } } }, { "upper", { { "voltage", 1 } } } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 11u);
	expectError(validationErrors, ValidationErrorCode::Value_MultipleValues, "/component-pool/values/0");
	expectError(validationErrors, ValidationErrorCode::Value_MultipleValues, "/component-pool/values/1");
	expectError(validationErrors, ValidationErrorCode::Value_MultipleValues, "/component-pool/values/2");
	expectError(validationErrors, ValidationErrorCode::Value_MultipleValues, "/component-pool/values/3");
	expectError(validationErrors, ValidationErrorCode::Value_MultipleValues, "/component-pool/values/4");
	expectError(validationErrors, ValidationErrorCode::Value_MultipleValues, "/component-pool/values/5");
	expectError(validationErrors, ValidationErrorCode::Value_MultipleValues, "/component-pool/values/6");
	expectError(validationErrors, ValidationErrorCode::Value_MultipleValues, "/component-pool/values/7");
	expectError(validationErrors, ValidationErrorCode::Value_MultipleValues, "/component-pool/values/8");
	expectError(validationErrors, ValidationErrorCode::Value_MultipleValues, "/component-pool/values/9");
	expectError(validationErrors, ValidationErrorCode::Value_MultipleValues, "/component-pool/values/10");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldWorkParseQuantize) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 5 } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].voltage);
	EXPECT_FALSE(script->values[0].quantize);
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithNonBooleanQuantize) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 5 }, { "quantize", "not-a-bool"} }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_QuantizeBool, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseQuantizeFalse) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 5 } , { "quantize", false}}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].voltage);
	EXPECT_FALSE(script->values[0].quantize);
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseQuantizeTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 5 } , { "quantize", true}}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_TRUE(script->values[0].voltage);
	EXPECT_TRUE(script->values[0].quantize);
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldParseWithoutCalcs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 5 } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	EXPECT_EQ(script->values[0].calc.size(), 0u);
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldFailWithNonArrayCalc) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 5 }, { "calc", "not-an-array" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_CalcArray, "/component-pool/values/0");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldSucceedWithEmptyCalcArray) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 5 }, { "calc", json::array() } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	EXPECT_EQ(script->values[0].calc.size(), 0u);
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldSucceedFailWithNonObjectCalc) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 5 }, { "calc", json::array({
				{ { "add", { { "voltage", 5} } } },
				"not-an-object",
				{ { "add", { { "voltage", 5} } } }
			}) } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_CalcObject, "/component-pool/values/0/calc/1");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldSucceedWithSingleCalc) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 5 }, { "calc", json::array({
				{ { "ref", "calc-ref-1" } }
			}) } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_EQ(script->values[0].calc.size(), 1u);
	EXPECT_EQ(script->values[0].calc[0].ref, "calc-ref-1");
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldSucceedWithMultipleCalcs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 5 }, { "calc", json::array({
				{ { "ref", "calc-ref-1" } },
				{ { "ref", "calc-ref-2" } },
				{ { "ref", "calc-ref-3" } }
			}) } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->values.size(), 1u);
	ASSERT_EQ(script->values[0].calc.size(), 3u);
	EXPECT_EQ(script->values[0].calc[0].ref, "calc-ref-1");
	EXPECT_EQ(script->values[0].calc[1].ref, "calc-ref-2");
	EXPECT_EQ(script->values[0].calc[2].ref, "calc-ref-3");
}

TEST(TimeSeqJsonScriptValue, ParseValueNotAllowOtherPropertiesOnRef) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-id-1" }, { "add", { { "ref", "value-ref" }, { "voltage", 5 } } } },
			{ { "id", "calc-id-2" }, { "add", { { "ref", "value-ref" }, { "note", "a4" } } } },
			{ { "id", "calc-id-3" }, { "add", { { "ref", "value-ref" }, { "variable", "variable-name" } } } },
			{ { "id", "calc-id-4" }, { "add", { { "ref", "value-ref" }, { "input", { { "dummy", "calc" } } } } } },
			{ { "id", "calc-id-5" }, { "add", { { "ref", "value-ref" }, { "output", { { "dummy", "calc" } } } } } },
			{ { "id", "calc-id-6" }, { "add", { { "ref", "value-ref" }, { "rand", { { "dummy", "rand" }} } } } },
			{ { "id", "calc-id-7" }, { "add", { { "ref", "value-ref" }, { "calc", { { "dummy", "calc" } } } } } },
			{ { "id", "calc-id-8" }, { "add", { { "ref", "value-ref" }, { "quantize", true } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 8u);
	expectError(validationErrors, ValidationErrorCode::Value_RefOrInstance, "/component-pool/calcs/0/add");
	expectError(validationErrors, ValidationErrorCode::Value_RefOrInstance, "/component-pool/calcs/1/add");
	expectError(validationErrors, ValidationErrorCode::Value_RefOrInstance, "/component-pool/calcs/2/add");
	expectError(validationErrors, ValidationErrorCode::Value_RefOrInstance, "/component-pool/calcs/3/add");
	expectError(validationErrors, ValidationErrorCode::Value_RefOrInstance, "/component-pool/calcs/4/add");
	expectError(validationErrors, ValidationErrorCode::Value_RefOrInstance, "/component-pool/calcs/5/add");
	expectError(validationErrors, ValidationErrorCode::Value_RefOrInstance, "/component-pool/calcs/6/add");
	expectError(validationErrors, ValidationErrorCode::Value_RefOrInstance, "/component-pool/calcs/7/add");
}

TEST(TimeSeqJsonScriptValue, ParseValueWithUnknownPropertyShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{
				{ "id", "value-1" },
				{ "voltage", 5 },
				{ "unknown-prop", "value" }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/values/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptValue, ParseValueWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{
				{ "id", "value-1" },
				{ "voltage", 5 },
				{ "unknown-prop-1", "value" },
				{ "unknown-prop-2", { { "child", "object" } } }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/values/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{
				{ "id", "value-1" },
				{ "voltage", 5 },
				{ "x-unknown-prop-1", "value" },
				{ "x-unknown-prop-2", { { "child", "object" } } }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}

TEST(TimeSeqJsonScriptValue, ParseValueWithUnknownPropertyOnRandShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", {
				{ "upper", { { "ref", "value-upper" } } },
				{ "lower", { { "ref", "value-lower" } } },
				{ "unknown-prop", "value" }
			} } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/values/0/rand");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptValue, ParseValueWithUnknownPropertiesOnRandShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", {
				{ "upper", { { "ref", "value-upper" } } },
				{ "lower", { { "ref", "value-lower" } } },
				{ "unknown-prop-1", "value" },
				{ "unknown-prop-2", { { "child", "object" } } }
			} } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/values/0/rand");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptValue, ParseValueShouldAllowUnknownPropertyWithXPrefixOnRand) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "rand", {
				{ "upper", { { "ref", "value-upper" } } },
				{ "lower", { { "ref", "value-lower" } } },
				{ "x-unknown-prop-1", "value" },
				{ "x-unknown-prop-2", { { "child", "object" } } }
			} } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}
