#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptSequence, ParseShouldSucceedWithoutSequencessVersion110AndLower) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_0_0);

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->sequences.size(), 0u);

	json = getMinimalJson(SCRIPT_VERSION_1_1_0);

	script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->sequences.size(), 0u);
}

TEST(TimeSeqJsonScriptSequence, ParseShouldSucceedWithoutSequencessVersion120) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->sequences.size(), 0u);
}

TEST(TimeSeqJsonScriptSequence, ParseShouldFailWithSequencessPre110Version) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_0_0);
	json["sequences"] = json::array();

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/");

	validationErrors.clear();
	json = getMinimalJson(SCRIPT_VERSION_1_0_0);
	json["sequences"] = json::array();

	script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/");
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldFailOnNonArraySequences) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = "not-an-array";

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_SequencesArray, "/");
}

TEST(TimeSeqJsonScriptSequence, ParseSequenceShouldFailOnNonObjectSequence) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "values", json::array( { .5f } ) } },
		"not-an-object",
		{ { "id", "sequence-2" }, { "values", json::array( { .5f } ) } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_SequenceObject, "/sequences/1");
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldFailOnDuplicateId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "values", json::array( { .5f } ) } },
		{ { "id", "sequence-2" }, { "values", json::array( { .5f } ) } },
		{ { "id", "sequence-2" }, { "values", json::array( { .3f } ) } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/sequences/2");
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldFailOnMissingId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "values", json::array( { .5f } ) } },
		{ { "values", json::array( { .5f } ) } },
		{ { "id", 5.f }, { "values", json::array( { .5f } ) } },
		{ { "id", "" }, { "values", json::array( { .5f } ) } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/sequences/1");
	expectError(validationErrors, ValidationErrorCode::Id_String, "/sequences/2");
	expectError(validationErrors, ValidationErrorCode::Id_Length, "/sequences/3");
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldFailOnMissingValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "values", json::array( { .5f } ) } },
		{ { "id", "sequence-2" }, { "values", json::array( { .5f } ) } },
		{ { "id", "sequence-3" } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Sequence_ValuesArray, "/sequences/2");
}

TEST(TimeSeqJsonScriptSequence, ParseSequenceshouldFailOnNonArrayValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "values", json::array( { .5f } ) } },
		{ { "id", "sequence-2" }, { "values", "not-correct" } },
		{ { "id", "sequence-3" }, { "values", json::array( { .5f } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Sequence_ValuesArray, "/sequences/1");
}

TEST(TimeSeqJsonScriptSequence, ParseSequenceShouldFailOnNonUnknownSequenceProperty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "values", json::array( { .5f } ) } },
		{ { "id", "sequence-2" }, { "values", json::array( { .5f } ) }, { "boats", "floats" } },
		{ { "id", "sequence-3" }, { "values", json::array( { .5f } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/sequences/1");
}

TEST(TimeSeqJsonScriptSequence, ParseSequenceshouldAllowEmptyValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-2" }, { "values", json::array() } },
		{ { "id", "sequence-1" }, { "values", json::array( { .5f } ) } },
		{ { "id", "sequence-3" }, { "values", json::array( { .5f } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->sequences.size(), 3u);
	EXPECT_EQ(script->sequences[0].values.size(), 0u);
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldFailOnInvalidFormatValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "values", json::array( { .5f }) } },
		{ { "id", "sequence-2" }, { "values", json::array( { json::array() } ) } },
		{ { "id", "sequence-3" }, { "values", json::array( { .5f } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Sequence_ValueObject, "/sequences/1/values/0");
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldFailOnInvalidValueFormat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "values", json::array( { 10.1f, 5.f, "", "h5", "a4", { { "voltage",  0.f } }, { { "voltage", 10.1f } } } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 4u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/sequences/0/values/0");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/sequences/0/values/2");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/sequences/0/values/3");
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/sequences/0/values/6");
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldParseInlineValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "values", json::array( { 1.1f, 5.f, "c1", "g5", "a4", { { "voltage",  0.f } }, { { "voltage", -5.1f } } } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	ASSERT_EQ(script->sequences.size(), 1u);
	ASSERT_EQ(script->sequences[0].values.size(), 7u);
	EXPECT_EQ(*script->sequences[0].values[0].voltage, 1.1f);
	EXPECT_EQ(*script->sequences[0].values[1].voltage, 5.f);
	EXPECT_EQ(*script->sequences[0].values[2].note, "c1");
	EXPECT_EQ(*script->sequences[0].values[3].note, "g5");
	EXPECT_EQ(*script->sequences[0].values[4].note, "a4");
	EXPECT_EQ(*script->sequences[0].values[5].voltage, 0.f);
	EXPECT_EQ(*script->sequences[0].values[6].voltage, -5.1f);
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldDefaultRetrieveVoltageOnceAndSharedToTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "values", json::array( { 1.1f, 5.f, "c1", "g5", "a4", { { "voltage",  0.f } }, { { "voltage", -5.1f } } } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	ASSERT_EQ(script->sequences.size(), 1u);
	EXPECT_EQ(script->sequences[0].retrieveVoltageOnce, true);
	EXPECT_EQ(script->sequences[0].shared, true);
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldFailOnNonBooleanShared) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "shared", "true" }, { "values", json::array( { 1.1f, 5.f, "c1", "g5", "a4", { { "voltage",  0.f } }, { { "voltage", -5.1f } } } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Sequence_SharedBoolean, "/sequences/0");
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldParseSharedBoolean) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "shared", true }, { "values", json::array( { 1.1f, 5.f, "c1", "g5", "a4", { { "voltage",  0.f } }, { { "voltage", -5.1f } } } ) } },
		{ { "id", "sequence-2" }, { "shared", false }, { "values", json::array( { 1.1f, 5.f, "c1", "g5", "a4", { { "voltage",  0.f } }, { { "voltage", -5.1f } } } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	ASSERT_EQ(script->sequences.size(), 2u);
	EXPECT_EQ(script->sequences[0].id, "sequence-1");
	EXPECT_EQ(script->sequences[0].shared, true);
	EXPECT_EQ(script->sequences[1].id, "sequence-2");
	EXPECT_EQ(script->sequences[1].shared, false);
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldFailOnNonBooleanRetrieveVoltageOnce) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "retrieve-voltage-once", "true" }, { "values", json::array( { 1.1f, 5.f, "c1", "g5", "a4", { { "voltage",  0.f } }, { { "voltage", -5.1f } } } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Sequence_RetrieveVoltageOnceBoolean, "/sequences/0");
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldParseSharedRetrieveVoltageOnce) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "retrieve-voltage-once", true }, { "values", json::array( { 1.1f, 5.f, "c1", "g5", "a4", { { "voltage",  0.f } }, { { "voltage", -5.1f } } } ) } },
		{ { "id", "sequence-2" }, { "retrieve-voltage-once", false }, { "values", json::array( { 1.1f, 5.f, "c1", "g5", "a4", { { "voltage",  0.f } }, { { "voltage", -5.1f } } } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	ASSERT_EQ(script->sequences.size(), 2u);
	EXPECT_EQ(script->sequences[0].id, "sequence-1");
	EXPECT_EQ(script->sequences[0].retrieveVoltageOnce, true);
	EXPECT_EQ(script->sequences[1].id, "sequence-2");
	EXPECT_EQ(script->sequences[1].retrieveVoltageOnce, false);
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesFailOnUnknownProperty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "unknown-property", true }, { "values", json::array( { 1.1f, 5.f, "c1", "g5", "a4", { { "voltage",  0.f } }, { { "voltage", -5.1f } } } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/sequences/0");
}

TEST(TimeSeqJsonScriptSequence, ParseSequencesShouldAllowUnknownXProperties) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["sequences"] = json::array({
		{ { "id", "sequence-1" }, { "x-unknown-property", true }, { "values", json::array( { 1.1f, 5.f, "c1", "g5", "a4", { { "voltage",  0.f } }, { { "voltage", -5.1f } } } ) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	ASSERT_EQ(script->sequences.size(), 1u);
	EXPECT_EQ(script->sequences[0].id, "sequence-1");
}
