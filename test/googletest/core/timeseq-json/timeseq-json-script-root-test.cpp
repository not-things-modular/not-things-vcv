#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScript, ParseWithoutTypeShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = {
		{ "version", SCRIPT_VERSION },
		{ "timelines", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_TypeMissing, "/");
}

TEST(TimeSeqJsonScript, ParseWithInvalidTypeShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = {
		{ "type", "not_not-things_timeseq_script" },
		{ "version", SCRIPT_VERSION },
		{ "timelines", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_TypeUnsupported, "/");
}

TEST(TimeSeqJsonScript, ParseWithoutVersionShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = {
		{ "type", "not-things_timeseq_script" },
		{ "timelines", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_VersionMissing, "/");
}

TEST(TimeSeqJsonScript, ParseWithUnknownVersionShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = {
		{ "type", "not-things_timeseq_script" },
		{ "version", "-0.0.1" },
		{ "timelines", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_VersionUnsupported, "/");
}

TEST(TimeSeqJsonScript, ParseWithNonObjectComponentPoolShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = {
		{ "type", "not-things_timeseq_script" },
		{ "version", "0.0.1" },
		{ "timelines", json::array() },
		{ "component-pool", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_ComponentPoolObject, "/");
}

TEST(TimeSeqJsonScript, ParseWithEmptyComponentPoolShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = {
		{ "type", "not-things_timeseq_script" },
		{ "version", "0.0.1" },
		{ "timelines", json::array() },
		{ "component-pool", json::object() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}

TEST(TimeSeqJsonScript, ParseRefShouldFailOnNonString) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = {
		{ "type", "not-things_timeseq_script" },
		{ "version", "0.0.1" },
		{ "timelines", json::array() },
		{ "component-pool", { { "calcs", json::array({
			{ { "id", "calc-id" }, { "add", { { "ref", 1 } } } }
		}) } } }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Ref_String, "/component-pool/calcs/0/add");
}

TEST(TimeSeqJsonScript, ParseRefShouldFailOnEmptyString) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = {
		{ "type", "not-things_timeseq_script" },
		{ "version", "0.0.1" },
		{ "timelines", json::array() },
		{ "component-pool", { { "calcs", json::array({
			{ { "id", "calc-id" }, { "add", { { "ref", "" } } } }
		}) } } }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Ref_Length, "/component-pool/calcs/0/add");
}

TEST(TimeSeqJsonScript, ParseRefShouldFailOnEmptyId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = {
		{ "type", "not-things_timeseq_script" },
		{ "version", "0.0.1" },
		{ "timelines", json::array() },
		{ "component-pool", { { "calcs", json::array({
			{ { "id", "" }, { "add", { { "ref", "value-ref" } } } }
		}) } } }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0u);
	expectError(validationErrors, ValidationErrorCode::Id_Length, "/component-pool/calcs/0");
}

TEST(TimeSeqJsonScript, ParseShouldFailOnInvalidJson) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	string jsonString = "{ \"this\": \"json\", is: \"invalid\" }";
	istringstream is(jsonString);

	jsonLoader.loadScript(is, &validationErrors);

	ASSERT_EQ(validationErrors.size(), 1u);
	EXPECT_EQ(validationErrors[0].location, "/");
	EXPECT_GT(validationErrors[0].message.size(), 0u);
}