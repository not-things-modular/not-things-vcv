#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptTuning, ParseShouldSucceedWithoutTuningsVersion100) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_0_0);

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->calcs.size(), 0u);
}

TEST(TimeSeqJsonScriptTuning, ParseShouldSucceedWithoutTuningsVersion110) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->calcs.size(), 0u);
}

TEST(TimeSeqJsonScriptTuning, ParseShouldFailWithTuningsPre110Version) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_0_0);
	json["component-pool"] = {
		{ "tunings", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/component-pool");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningsShouldFailOnNonArrayTunings) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", "not-an-array" }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_TuningsArray, "/component-pool");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldFailOnNonObjectTuning) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { .5f } ) } },
			"not-an-object",
			{ { "id", "tuning-2" }, { "notes", json::array( { .5f } ) } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_TuningObject, "/component-pool/tunings/1");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldFailOnDuplicateId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { .5f } ) } },
			{ { "id", "tuning-2" }, { "notes", json::array( { .5f } ) } },
			{ { "id", "tuning-2" }, { "notes", json::array( { .3f } ) } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/tunings/2");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldFailOnMissingId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { .5f } ) } },
			{ { "notes", json::array( { .5f } ) } },
			{ { "id", 5.f }, { "notes", json::array( { .5f } ) } },
			{ { "id", "" }, { "notes", json::array( { .5f } ) } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/component-pool/tunings/1");
	expectError(validationErrors, ValidationErrorCode::Id_String, "/component-pool/tunings/2");
	expectError(validationErrors, ValidationErrorCode::Id_Length, "/component-pool/tunings/3");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldFailOnMissingNotes) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { .5f } ) } },
			{ { "id", "tuning-2" }, { "notes", json::array( { .5f } ) } },
			{ { "id", "tuning-3" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Tuning_NotesArray, "/component-pool/tunings/2");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldFailOnNonArrayNotes) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { .5f } ) } },
			{ { "id", "tuning-2" }, { "notes", "not-correct" } },
			{ { "id", "tuning-3" }, { "notes", json::array( { .5f } ) } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Tuning_NotesArray, "/component-pool/tunings/1");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldFailOnNonUnknownTuningProperty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { .5f } ) } },
			{ { "id", "tuning-2" }, { "notes", json::array( { .5f } ) }, { "boats", "floats" } },
			{ { "id", "tuning-3" }, { "notes", json::array( { .5f } ) } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/tunings/1");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldFailOnEmptyNotes) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-2" }, { "notes", json::array() } },
			{ { "id", "tuning-1" }, { "notes", json::array( { .5f } ) } },
			{ { "id", "tuning-3" }, { "notes", json::array( { .5f } ) } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Tuning_NotesArraySize, "/component-pool/tunings/0/notes");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldFailOnInvalidFormatNotes) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { .5f }) } },
			{ { "id", "tuning-2" }, { "notes", json::array( { json::array() } ) } },
			{ { "id", "tuning-3" }, { "notes", json::array( { .5f } ) } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Tuning_NoteFloatOrString, "/component-pool/tunings/1/notes/0");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldFailOnInvalidNoteFormat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { "c", "", "d", "h", "c+", "c/", "abc" }) } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 4u);
	expectError(validationErrors, ValidationErrorCode::Tuning_NoteFormat, "/component-pool/tunings/0/notes/1");
	expectError(validationErrors, ValidationErrorCode::Tuning_NoteFormat, "/component-pool/tunings/0/notes/3");
	expectError(validationErrors, ValidationErrorCode::Tuning_NoteFormat, "/component-pool/tunings/0/notes/5");
	expectError(validationErrors, ValidationErrorCode::Tuning_NoteFormat, "/component-pool/tunings/0/notes/6");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldMapFloatNotesToSingleOctaveAndSort) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { -.5f, -2.003f, -.001f, 0.f, 0.001f, .49f, .998f, 1.005f, 5.096f } ) } },
			{ { "id", "tuning-2" }, { "notes", json::array( { 1.f } ) } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	
	ASSERT_EQ(script->tunings.size(), 2u);
	EXPECT_EQ(script->tunings[0].id, "tuning-1");
	ASSERT_EQ(script->tunings[0].notes.size(), 9u);
	EXPECT_NEAR(script->tunings[0].notes[0], 0.f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[1], .001f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[2], .005f, 0.00001f);
	EXPECT_NEAR(script->tunings[0].notes[3], .096f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[4], .49f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[5], .5f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[6], .997f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[7], .998f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[8], .999f, 0.000001f);
	EXPECT_EQ(script->tunings[1].id, "tuning-2");
	ASSERT_EQ(script->tunings[1].notes.size(), 1u);
	EXPECT_NEAR(script->tunings[1].notes[0], 0.f, 0.000001f);
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldMapFloatNotesToSingleOctaveAndFilterDuplicates) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { -.5f, -2.f, .25f, 0.f, 3.f, .5f, .75f, 3.75f } ) } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	
	ASSERT_EQ(script->tunings.size(), 1u);
	EXPECT_EQ(script->tunings[0].id, "tuning-1");
	ASSERT_EQ(script->tunings[0].notes.size(), 4u);
	EXPECT_NEAR(script->tunings[0].notes[0], 0.f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[1], .25f, 0.00001f);
	EXPECT_NEAR(script->tunings[0].notes[2], .5f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[3], .75f, 0.000001f);
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldMapNoteStringToCorrespondingVoltage) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { "C", "d", "E", "f", "G", "a", "B" } ) } },
			{ { "id", "tuning-2" }, { "notes", json::array( { "c+", "D+", "e+", "F+", "g+", "A+", "b+" } ) } },
			{ { "id", "tuning-3" }, { "notes", json::array( { "C-", "D-", "E-", "F-", "G-", "A-", "B-" } ) } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	
	ASSERT_EQ(script->tunings.size(), 3u);

	EXPECT_EQ(script->tunings[0].id, "tuning-1");
	ASSERT_EQ(script->tunings[0].notes.size(), 7u);
	EXPECT_NEAR(script->tunings[0].notes[0], 0.f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[1], 2.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[2], 4.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[3], 5.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[4], 7.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[5], 9.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[6], 11.f / 12, 0.000001f);

	EXPECT_EQ(script->tunings[1].id, "tuning-2");
	ASSERT_EQ(script->tunings[1].notes.size(), 7u);
	EXPECT_NEAR(script->tunings[1].notes[0], 0.f, 0.000001f); // notes are sorted, and a b+ ends up as a c (i.e. a 0.f), so it gets moved to the front.
	EXPECT_NEAR(script->tunings[1].notes[1], 1.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[1].notes[2], 3.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[1].notes[3], 5.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[1].notes[4], 6.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[1].notes[5], 8.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[1].notes[6], 10.f / 12, 0.000001f);

	EXPECT_EQ(script->tunings[2].id, "tuning-3");
	ASSERT_EQ(script->tunings[2].notes.size(), 7u);
	EXPECT_NEAR(script->tunings[2].notes[0], 1.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[2].notes[1], 3.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[2].notes[2], 4.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[2].notes[3], 6.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[2].notes[4], 8.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[2].notes[5], 10.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[2].notes[6], 11.f / 12, 0.000001f); // notes are sorted, and C- ends up as a B (i.e. 11.f / 12), so that gets moved to the back.
}


TEST(TimeSeqJsonScriptTuning, ParseTuningShouldMixNotesAndVoltagesAndFilterDuplicates) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "tunings", json::array({
			{ { "id", "tuning-1" }, { "notes", json::array( { "C", .25f, 0.f, "d+", "E", .75 } ) } },
			{ { "id", "tuning-2" }, { "notes", json::array( { 0.f, "d+", "C", .25f, "E", .75 } ) } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	
	ASSERT_EQ(script->tunings.size(), 2u);

	EXPECT_EQ(script->tunings[0].id, "tuning-1");
	ASSERT_EQ(script->tunings[0].notes.size(), 4u);
	EXPECT_NEAR(script->tunings[0].notes[0], 0.f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[1], .25f, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[2], 4.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[0].notes[3], .75f, 0.000001f);

	EXPECT_EQ(script->tunings[1].id, "tuning-2");
	ASSERT_EQ(script->tunings[1].notes.size(), 4u);
	EXPECT_NEAR(script->tunings[1].notes[0], 0.f, 0.000001f);
	EXPECT_NEAR(script->tunings[1].notes[1], .25f, 0.000001f);
	EXPECT_NEAR(script->tunings[1].notes[2], 4.f / 12, 0.000001f);
	EXPECT_NEAR(script->tunings[1].notes[3], .75f, 0.000001f);
}

TEST(TimeSeqJsonScriptTuning, ParseTuningWorkWithInlineRefCalcTuning) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc" }, { "quantize", { { "ref", "my-ref-tuning" } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	
	ASSERT_EQ(script->calcs.size(), 1u);
	EXPECT_EQ(script->calcs[0].operation, ScriptCalc::CalcOperation::QUANTIZE);
	ASSERT_TRUE(script->calcs[0].tuning);
	EXPECT_EQ(script->calcs[0].tuning->ref, "my-ref-tuning");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningWorkFailWithInlineRefCalcTuningWithAdditionalProperties) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc" }, { "quantize", { { "ref", "my-ref-tuning" }, { "notes", json::array({ 0.f }) } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Tuning_RefOrInstance, "/component-pool/calcs/0/quantize");
}

TEST(TimeSeqJsonScriptTuning, ParseTuningWorkWithInlineFullTuning) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc" }, { "quantize", { { "notes", json::array({ 1.f, 5.25f, 3.33f }) } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	
	ASSERT_EQ(script->calcs.size(), 1u);
	EXPECT_EQ(script->calcs[0].operation, ScriptCalc::CalcOperation::QUANTIZE);
	ASSERT_TRUE(script->calcs[0].tuning);
	EXPECT_NEAR(script->calcs[0].tuning->notes[0], 0.f, 0.000001f);
	EXPECT_NEAR(script->calcs[0].tuning->notes[1], .25f, 0.000001f);
	EXPECT_NEAR(script->calcs[0].tuning->notes[2], .33f, 0.000001f);
}

TEST(TimeSeqJsonScriptTuning, ParseTuningShouldFailOnInlineTuningWithId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc" }, { "quantize", { { "id", "not-allowed" }, { "notes", json::array({ 1.f, 5.25f, 3.33f }) } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Id_NotAllowed, "/component-pool/calcs/0/quantize");
}
