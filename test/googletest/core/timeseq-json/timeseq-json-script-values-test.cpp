#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptValue, ParseShouldSucceedWithoutValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	EXPECT_EQ(script->values.size(), 0u);
}

TEST(TimeSeqJsonScriptValue, ParseShouldSucceedWithEmptyValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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
			{ { "id", "value-1" }, { "voltage", 2 } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
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
			{ { "id", "value-2" }, { "voltage", 0 } },
			{ { "id", "value-2" }, { "voltage", 6 } },
			{ { "id", "value-2" }, { "voltage", 10 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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

TEST(TimeSeqJsonScriptValue, ParseValueShouldSucceedWithFloatVoltages) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", -10.0f } },
			{ { "id", "value-2" }, { "voltage", -9.999f } },
			{ { "id", "value-2" }, { "voltage", 0.1f } },
			{ { "id", "value-2" }, { "voltage", 4.99f } },
			{ { "id", "value-2" }, { "voltage", 10.0f } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
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
