#include "timeseq-json-shared.hpp"
#include <rack.hpp>

TEST(TimeSeqJsonScriptCalc, ParseShouldSucceedWithoutCalcs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->calcs.size(), 0u);
}

TEST(TimeSeqJsonScriptCalc, ParseShouldSucceedWithEmptyCalcs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->calcs.size(), 0u);
}

TEST(TimeSeqJsonScriptCalc, ParseCalcsShouldNotAllowRefAndRequireIdOnRoot) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", {
			{ { "ref", "calc-ref" } }
		} }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/component-pool/calcs/0");
	expectError(validationErrors, ValidationErrorCode::Ref_NotAllowed, "/component-pool/calcs/0");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcsShouldFailOnNonArrayCalcs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", "not-an-array" }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_CalcsArray, "/component-pool");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcsShouldFailOnNonObjectCalc) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", { { "voltage", 1 } } } },
			"not-an-object",
			{ { "id", "calc-2" }, { "add", { { "voltage", 1 } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_CalcObject, "/component-pool/calcs/1");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnMissingOperation) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Calc_NoOperation, "/component-pool/calcs/0");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldVerifyFeatureVersion) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
			{ "calcs", json::array({
				{ { "id", "calc-1" }, { "add", { { "voltage", 1 } } } }, // Was already part of 1.0.0
				{ { "id", "calc-2" }, { "sub", { { "voltage", 1 } } } }, // Was already part of 1.0.0
				{ { "id", "calc-3" }, { "mult", { { "voltage", 1 } } } }, // Was already part of 1.0.0
				{ { "id", "calc-4" }, { "div", { { "voltage", 1 } } } }, // Was already part of 1.0.0
				{ { "id", "calc-5" }, { "max", { { "voltage", 1 } } } }, // Requires 1.1.0
				{ { "id", "calc-6" }, { "min", { { "voltage", 1 } } } }, // Requires 1.1.0
				{ { "id", "calc-7" }, { "remain", { { "voltage", 1 } } } }, // Requires 1.1.0
				{ { "id", "calc-8" }, { "trunc", true } }, // Requires 1.1.0
				{ { "id", "calc-9" }, { "frac", true } }, // Requires 1.1.0
				{ { "id", "calc-10" }, { "round", "down" } }, // Requires 1.1.0
				{ { "id", "calc-11" }, { "quantize", "tuning" } }, // Requires 1.1.0
				{ { "id", "calc-12" }, { "sign", "pos" } } // Requires 1.1.0
			}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 8u);
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/component-pool/calcs/4");
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/component-pool/calcs/5");
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/component-pool/calcs/6");
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/component-pool/calcs/7");
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/component-pool/calcs/8");
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/component-pool/calcs/9");
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/component-pool/calcs/10");
	expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/component-pool/calcs/11");

	for (vector<ValidationError>::iterator it = validationErrors.begin(); it != validationErrors.end(); it++) {
		EXPECT_NE(it->message.find("requires version 1.1.0"), std::string::npos);
		EXPECT_NE(it->message.find("version set to 1.0.0"), std::string::npos);
	}
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailWithMultipleOperations) {
	vector<json> operations = {
		{ "add", { { "voltage", 1 } } },
		{ "sub", { { "voltage", 1 } } },
		{ "mult", { { "voltage", 1 } } },
		{ "div", { { "voltage", 1 } } },
		{ "max", { { "voltage", 1 } } },
		{ "min", { { "voltage", 1 } } },
		{ "remain", { { "voltage", 1 } } },
		{ "trunc", true },
		{ "frac", true },
		{ "round", "down" },
		{ "quantize", "tuning" },
		{ "sign", "pos" }
	};

	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);

	json["component-pool"] = {
		{ "calcs", json::array({}) }
	};

	unsigned int count = 0;
	for (unsigned int i = 0; i < operations.size(); i++) {
		for (unsigned int j = i + 1; j < operations.size(); j++) {
			json["component-pool"]["calcs"].push_back({
				{ "id", rack::string::f("calc-%d-%d", i, j).c_str() },
				operations[i],
				operations[j]
			});
			count++;
		}
	}

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), count) << json.dump();

	for (unsigned int i = 0; i < count; i++) {
		expectError(validationErrors, ValidationErrorCode::Calc_MultipleOperations, rack::string::f("/component-pool/calcs/%d", i).c_str());
	}
}

TEST(TimeSeqJsonScriptCalc, ParseValueCalcShouldFailOnNonObjectOperator) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", json::array() } },
			{ { "id", "calc-2" }, { "sub", json::array() } },
			{ { "id", "calc-3" }, { "div", json::array() } },
			{ { "id", "calc-4" }, { "mult", json::array() } },
			{ { "id", "calc-5" }, { "max", json::array() } },
			{ { "id", "calc-6" }, { "min", json::array() } },
			{ { "id", "calc-7" }, { "remain", json::array() } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 7u);
	expectError(validationErrors, ValidationErrorCode::Calc_AddObject, "/component-pool/calcs/0");
	expectError(validationErrors, ValidationErrorCode::Calc_SubObject, "/component-pool/calcs/1");
	expectError(validationErrors, ValidationErrorCode::Calc_DivObject, "/component-pool/calcs/2");
	expectError(validationErrors, ValidationErrorCode::Calc_MultObject, "/component-pool/calcs/3");
	expectError(validationErrors, ValidationErrorCode::Calc_MaxObject, "/component-pool/calcs/4");
	expectError(validationErrors, ValidationErrorCode::Calc_MinObject, "/component-pool/calcs/5");
	expectError(validationErrors, ValidationErrorCode::Calc_RemainObject, "/component-pool/calcs/6");
}

TEST(TimeSeqJsonScriptCalc, ParseValueCalcShouldFailOnInvalidCalcValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", json::object() } },
			{ { "id", "calc-2" }, { "sub", json::object() } },
			{ { "id", "calc-3" }, { "div", json::object() } },
			{ { "id", "calc-4" }, { "mult", json::object() } },
			{ { "id", "calc-5" }, { "max", json::object() } },
			{ { "id", "calc-6" }, { "min", json::object() } },
			{ { "id", "calc-7" }, { "remain", json::object() } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 7u);
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/calcs/0/add");
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/calcs/1/sub");
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/calcs/2/div");
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/calcs/3/mult");
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/calcs/4/max");
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/calcs/5/min");
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/calcs/6/remain");
}

TEST(TimeSeqJsonScriptCalc, ParseValueCalcShouldParseCalcs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", { { "ref", "value-1" }} } },
			{ { "id", "calc-2" }, { "sub", { { "ref", "value-2" }} } },
			{ { "id", "calc-3" }, { "div", { { "ref", "value-3" }} } },
			{ { "id", "calc-4" }, { "mult", { { "ref", "value-4" }} } },
			{ { "id", "calc-5" }, { "max", { { "ref", "value-5" }} } },
			{ { "id", "calc-6" }, { "min", { { "ref", "value-6" }} } },
			{ { "id", "calc-7" }, { "remain", { { "ref", "value-7" }} } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->calcs.size(), 7u);
	EXPECT_EQ(script->calcs[0].id, "calc-1");
	EXPECT_EQ(script->calcs[0].operation, ScriptCalc::CalcOperation::ADD);
	EXPECT_EQ(script->calcs[1].id, "calc-2");
	EXPECT_EQ(script->calcs[1].operation, ScriptCalc::CalcOperation::SUB);
	EXPECT_EQ(script->calcs[2].id, "calc-3");
	EXPECT_EQ(script->calcs[2].operation, ScriptCalc::CalcOperation::DIV);
	EXPECT_EQ(script->calcs[3].id, "calc-4");
	EXPECT_EQ(script->calcs[3].operation, ScriptCalc::CalcOperation::MULT);
	EXPECT_EQ(script->calcs[4].id, "calc-5");
	EXPECT_EQ(script->calcs[4].operation, ScriptCalc::CalcOperation::MAX);
	EXPECT_EQ(script->calcs[5].id, "calc-6");
	EXPECT_EQ(script->calcs[5].operation, ScriptCalc::CalcOperation::MIN);
	EXPECT_EQ(script->calcs[6].id, "calc-7");
	EXPECT_EQ(script->calcs[6].operation, ScriptCalc::CalcOperation::REMAIN);

	ASSERT_TRUE(script->calcs[0].value);
	EXPECT_EQ(script->calcs[0].value.get()->ref, "value-1");
	ASSERT_TRUE(script->calcs[1].value);
	EXPECT_EQ(script->calcs[1].value.get()->ref, "value-2");
	ASSERT_TRUE(script->calcs[2].value);
	EXPECT_EQ(script->calcs[2].value.get()->ref, "value-3");
	ASSERT_TRUE(script->calcs[3].value);
	EXPECT_EQ(script->calcs[3].value.get()->ref, "value-4");
	ASSERT_TRUE(script->calcs[4].value);
	EXPECT_EQ(script->calcs[4].value.get()->ref, "value-5");
	ASSERT_TRUE(script->calcs[5].value);
	EXPECT_EQ(script->calcs[5].value.get()->ref, "value-6");
	ASSERT_TRUE(script->calcs[6].value);
	EXPECT_EQ(script->calcs[6].value.get()->ref, "value-7");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnDuplicateIds) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", { { "ref", "value-1" }} } },
			{ { "id", "calc-1" }, { "add", { { "ref", "value-1" }} } },
			{ { "id", "calc-2" }, { "add", { { "ref", "value-1" }} } },
			{ { "id", "calc-3" }, { "add", { { "ref", "value-1" }} } },
			{ { "id", "calc-2" }, { "add", { { "ref", "value-1" }} } },
			{ { "id", "calc-1" }, { "add", { { "ref", "value-1" }} } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/calcs/1");
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/calcs/4");
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/calcs/5");
	EXPECT_NE(validationErrors[0].message.find("'calc-1'"), std::string::npos);
	EXPECT_NE(validationErrors[1].message.find("'calc-2'"), std::string::npos);
	EXPECT_NE(validationErrors[2].message.find("'calc-1'"), std::string::npos);
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnRefWithOtherCalcProperties) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-1" }, { "add", json::object() }}) })} },
			{ { "id", "value-2" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-2" }, { "sub", json::object() }}) })} },
			{ { "id", "value-3" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-3" }, { "div", json::object() }}) })} },
			{ { "id", "value-4" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-4" }}), json::object({{ "ref", "calc-ref-4" }, { "mult", json::object() }}) })} },
			{ { "id", "value-5" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-5" }, { "max", json::object() }}) })} },
			{ { "id", "value-6" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-6" }, { "min", json::object() }}) })} },
			{ { "id", "value-7" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-7" }, { "remain", json::object() }}) })} },
			{ { "id", "value-8" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-8" }, { "trunc", true }}) })} },
			{ { "id", "value-9" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-9" }, { "frac", true }}) })} },
			{ { "id", "value-10" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-10" }, { "round", "up" }}) })} },
			{ { "id", "value-11" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-11" }, { "quantize", "tuning" }}) })} },
			{ { "id", "value-12" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-12" }, { "sign", "pos" }}) })} },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 12u);
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/0/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/1/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/2/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/3/calc/1");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/4/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/5/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/6/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/7/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/8/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/9/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/10/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/11/calc/0");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcWithUnknownPropertyShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array({
			{
				{ "id", "calc-1" },
				{ "add", { { "ref", "value-1" }} },
				{ "unknown-prop", "value" }
			},
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/calcs/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptCalc, ParseCalcWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array({
			{
				{ "id", "calc-1" },
				{ "add", { { "ref", "value-1" }} },
				{ "unknown-prop-1", "value" },
				{ "unknown-prop-2", { { "child", "object" } } }
			},
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/calcs/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnNonBooleanTrunc) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "trunc", { { "ref", "value-1" } } } },
			{ { "id", "calc-2" }, { "trunc", "true" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Calc_TruncBoolean, "/component-pool/calcs/0");
	expectError(validationErrors, ValidationErrorCode::Calc_TruncBoolean, "/component-pool/calcs/1");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnFalseTrunc) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "trunc", false } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Calc_TruncBoolean, "/component-pool/calcs/0");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldParseTrueTrunc) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "trunc", true } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);

	EXPECT_EQ(script->calcs.size(), 1u);
	EXPECT_EQ(script->calcs[0].operation, ScriptCalc::CalcOperation::TRUNC);
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnNonBooleanFrac) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "frac", { { "ref", "value-1" } } } },
			{ { "id", "calc-2" }, { "frac", "true" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Calc_FracBoolean, "/component-pool/calcs/0");
	expectError(validationErrors, ValidationErrorCode::Calc_FracBoolean, "/component-pool/calcs/1");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnFalseFrac) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "frac", false } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Calc_FracBoolean, "/component-pool/calcs/0");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldParseTrueFrac) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "frac", true } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);

	EXPECT_EQ(script->calcs.size(), 1u);
	EXPECT_EQ(script->calcs[0].operation, ScriptCalc::CalcOperation::FRAC);
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnNonStringRound) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "round", { { "ref", "value-1" } } } },
			{ { "id", "calc-2" }, { "round", 1.0f } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Calc_RoundString, "/component-pool/calcs/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RoundString, "/component-pool/calcs/1");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnInvalidRound) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "round", "yes please" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Calc_RoundEnum, "/component-pool/calcs/0");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldParseRoundValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "round", "up" } },
			{ { "id", "calc-2" }, { "round", "down" } },
			{ { "id", "calc-3" }, { "round", "near" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);

	EXPECT_EQ(script->calcs.size(), 3u);
	EXPECT_EQ(script->calcs[0].operation, ScriptCalc::CalcOperation::ROUND);
	EXPECT_TRUE(script->calcs[0].roundType);
	EXPECT_EQ(*script->calcs[0].roundType.get(), ScriptCalc::RoundType::UP);
	EXPECT_EQ(script->calcs[1].operation, ScriptCalc::CalcOperation::ROUND);
	EXPECT_TRUE(script->calcs[1].roundType);
	EXPECT_EQ(*script->calcs[1].roundType.get(), ScriptCalc::RoundType::DOWN);
	EXPECT_EQ(script->calcs[2].operation, ScriptCalc::CalcOperation::ROUND);
	EXPECT_TRUE(script->calcs[2].roundType);
	EXPECT_EQ(*script->calcs[2].roundType.get(), ScriptCalc::RoundType::NEAR);
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnNonStringQuantize) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "quantize", { { "ref", "value-1" } } } },
			{ { "id", "calc-2" }, { "quantize", 1.0f } },
			{ { "id", "calc-3" }, { "quantize", nullptr } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Calc_QuantizeString, "/component-pool/calcs/0");
	expectError(validationErrors, ValidationErrorCode::Calc_QuantizeString, "/component-pool/calcs/1");
	expectError(validationErrors, ValidationErrorCode::Calc_QuantizeString, "/component-pool/calcs/2");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnEmptyQuantize) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "quantize", "" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Calc_QuantizeString, "/component-pool/calcs/0");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldParseQuantize) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "quantize", "my-tuning" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);

	EXPECT_EQ(script->calcs.size(), 1u);
	EXPECT_EQ(script->calcs[0].operation, ScriptCalc::CalcOperation::QUANTIZE);
	EXPECT_EQ(script->calcs[0].tuning, "my-tuning");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnNonStringSign) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "sign", { { "ref", "value-1" } } } },
			{ { "id", "calc-2" }, { "sign", 1.0f } },
			{ { "id", "calc-3" }, { "sign", nullptr } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Calc_SignString, "/component-pool/calcs/0");
	expectError(validationErrors, ValidationErrorCode::Calc_SignString, "/component-pool/calcs/1");
	expectError(validationErrors, ValidationErrorCode::Calc_SignString, "/component-pool/calcs/2");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnInvalidSign) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "sign", "on the dotted line" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Calc_SignEnum, "/component-pool/calcs/0");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldParseSignValues) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "sign", "pos" } },
			{ { "id", "calc-2" }, { "sign", "neg" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);

	EXPECT_EQ(script->calcs.size(), 2u);
	EXPECT_EQ(script->calcs[0].operation, ScriptCalc::CalcOperation::SIGN);
	EXPECT_TRUE(script->calcs[0].signType);
	EXPECT_EQ(*script->calcs[0].signType.get(), ScriptCalc::SignType::POS);
	EXPECT_EQ(script->calcs[1].operation, ScriptCalc::CalcOperation::SIGN);
	EXPECT_TRUE(script->calcs[1].signType);
	EXPECT_EQ(*script->calcs[1].signType.get(), ScriptCalc::SignType::NEG);
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array({
			{
				{ "id", "calc-1" },
				{ "add", { { "ref", "value-1" }} },
				{ "x-unknown-prop-1", "value" },
				{ "x-unknown-prop-2", { { "child", "object" } } }
			},
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}

TEST(TimeSeqJsonScriptCalc, ParseValueCalcShouldFailOnShorthandValueOutOfRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", 11.f } },
			{ { "id", "calc-2" }, { "sub", 10.1f } },
			{ { "id", "calc-3" }, { "mult", -10.1f } },
			{ { "id", "calc-4" }, { "div", -11.f } },
			{ { "id", "calc-5" }, { "max", 10.1f } },
			{ { "id", "calc-6" }, { "min", -10.1f } },
			{ { "id", "calc-7" }, { "remain", -11.f } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 7u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/calcs/0/add");
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/calcs/1/sub");
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/calcs/2/mult");
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/calcs/3/div");
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/calcs/4/max");
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/calcs/5/min");
	expectError(validationErrors, ValidationErrorCode::Value_VoltageRange, "/component-pool/calcs/6/remain");
}

TEST(TimeSeqJsonScriptCalc, ParseValueCalcShouldParseFloatShorthandValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", 9.9f } },
			{ { "id", "calc-2" }, { "sub", 3.4f } },
			{ { "id", "calc-3" }, { "mult", -3.4f } },
			{ { "id", "calc-4" }, { "div", -8.4f } },
			{ { "id", "calc-5" }, { "max", 5.6f } },
			{ { "id", "calc-6" }, { "min", -5.6f } },
			{ { "id", "calc-7" }, { "remain", -7.8f } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	EXPECT_EQ(script->calcs.size(), 7u);
	EXPECT_TRUE(script->calcs[0].value->voltage);
	EXPECT_EQ(*script->calcs[0].value->voltage, 9.9f);
	EXPECT_TRUE(script->calcs[1].value->voltage);
	EXPECT_EQ(*script->calcs[1].value->voltage, 3.4f);
	EXPECT_TRUE(script->calcs[2].value->voltage);
	EXPECT_EQ(*script->calcs[2].value->voltage, -3.4f);
	EXPECT_TRUE(script->calcs[3].value->voltage);
	EXPECT_EQ(*script->calcs[3].value->voltage, -8.4f);
	EXPECT_TRUE(script->calcs[4].value->voltage);
	EXPECT_EQ(*script->calcs[4].value->voltage, 5.6f);
	EXPECT_TRUE(script->calcs[5].value->voltage);
	EXPECT_EQ(*script->calcs[5].value->voltage, -5.6f);
	EXPECT_TRUE(script->calcs[6].value->voltage);
	EXPECT_EQ(*script->calcs[6].value->voltage, -7.8f);
}

TEST(TimeSeqJsonScriptCalc, ParseValueCalcShouldFailOnNonNoteStringShorthandValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", "AA" } },
			{ { "id", "calc-2" }, { "sub", "BB" } },
			{ { "id", "calc-3" }, { "mult", "CC" } },
			{ { "id", "calc-4" }, { "div", "DD" } },
			{ { "id", "calc-5" }, { "max", "EE" } },
			{ { "id", "calc-6" }, { "min", "FF" } },
			{ { "id", "calc-7" }, { "remain", "GG" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 7u);
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/calcs/0/add");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/calcs/1/sub");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/calcs/2/mult");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/calcs/3/div");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/calcs/4/max");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/calcs/5/min");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/calcs/6/remain");
}

TEST(TimeSeqJsonScriptCalc, ParseValueCalcShouldParseNoteShorthandValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", "C4-" } },
			{ { "id", "calc-2" }, { "sub", "D5+" } },
			{ { "id", "calc-3" }, { "mult", "E3-" } },
			{ { "id", "calc-4" }, { "div", "F6+" } },
			{ { "id", "calc-5" }, { "max", "G1+" } },
			{ { "id", "calc-6" }, { "min", "A2-" } },
			{ { "id", "calc-7" }, { "remain", "B7+" } }
		} ) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	EXPECT_EQ(script->calcs.size(), 7u);
	EXPECT_TRUE(script->calcs[0].value->note);
	EXPECT_EQ(*script->calcs[0].value->note, "C4-");
	EXPECT_TRUE(script->calcs[1].value->note);
	EXPECT_EQ(*script->calcs[1].value->note, "D5+");
	EXPECT_TRUE(script->calcs[2].value->note);
	EXPECT_EQ(*script->calcs[2].value->note, "E3-");
	EXPECT_TRUE(script->calcs[3].value->note);
	EXPECT_EQ(*script->calcs[3].value->note, "F6+");
	EXPECT_TRUE(script->calcs[4].value->note);
	EXPECT_EQ(*script->calcs[4].value->note, "G1+");
	EXPECT_TRUE(script->calcs[5].value->note);
	EXPECT_EQ(*script->calcs[5].value->note, "A2-");
	EXPECT_TRUE(script->calcs[6].value->note);
	EXPECT_EQ(*script->calcs[6].value->note, "B7+");
}
