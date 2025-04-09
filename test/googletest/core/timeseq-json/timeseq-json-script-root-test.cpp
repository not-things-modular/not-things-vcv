#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScript, ParseWithoutTypeShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = {
		{ "version", SCRIPT_VERSION },
		{ "timelines", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
}
