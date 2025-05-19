#include "timeseq-json-shared.hpp"

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

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailWithMultipleOperations) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", { { "voltage", 1 } } }, { "sub", { { "voltage", 1 } } } },
			{ { "id", "calc-2" }, { "add", { { "voltage", 1 } } }, { "div", { { "voltage", 1 } } } },
			{ { "id", "calc-3" }, { "add", { { "voltage", 1 } } }, { "mult", { { "voltage", 1 } } } },
			{ { "id", "calc-4" }, { "sub", { { "voltage", 1 } } }, { "div", { { "voltage", 1 } } } },
			{ { "id", "calc-5" }, { "sub", { { "voltage", 1 } } }, { "mult", { { "voltage", 1 } } } },
			{ { "id", "calc-6" }, { "div", { { "voltage", 1 } } }, { "mult", { { "voltage", 1 } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 6u);
	expectError(validationErrors, ValidationErrorCode::Calc_MultpleOperations, "/component-pool/calcs/0");
	expectError(validationErrors, ValidationErrorCode::Calc_MultpleOperations, "/component-pool/calcs/1");
	expectError(validationErrors, ValidationErrorCode::Calc_MultpleOperations, "/component-pool/calcs/2");
	expectError(validationErrors, ValidationErrorCode::Calc_MultpleOperations, "/component-pool/calcs/3");
	expectError(validationErrors, ValidationErrorCode::Calc_MultpleOperations, "/component-pool/calcs/4");
	expectError(validationErrors, ValidationErrorCode::Calc_MultpleOperations, "/component-pool/calcs/5");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnNonObjectOperator) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", json::array() } },
			{ { "id", "calc-2" }, { "sub", json::array() } },
			{ { "id", "calc-3" }, { "div", json::array() } },
			{ { "id", "calc-4" }, { "mult", json::array() } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 4u);
	expectError(validationErrors, ValidationErrorCode::Calc_AddObject, "/component-pool/calcs/0");
	expectError(validationErrors, ValidationErrorCode::Calc_SubObject, "/component-pool/calcs/1");
	expectError(validationErrors, ValidationErrorCode::Calc_DivObject, "/component-pool/calcs/2");
	expectError(validationErrors, ValidationErrorCode::Calc_MultObject, "/component-pool/calcs/3");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldFailOnInvalidCalcValue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", json::object() } },
			{ { "id", "calc-2" }, { "sub", json::object() } },
			{ { "id", "calc-3" }, { "div", json::object() } },
			{ { "id", "calc-4" }, { "mult", json::object() } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 4u);
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/calcs/0/add");
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/calcs/1/sub");
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/calcs/2/div");
	expectError(validationErrors, ValidationErrorCode::Value_NoActualValue, "/component-pool/calcs/3/mult");
}

TEST(TimeSeqJsonScriptCalc, ParseCalcShouldParseCalcs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "calcs", json::array({
			{ { "id", "calc-1" }, { "add", { { "ref", "value-1" }} } },
			{ { "id", "calc-2" }, { "sub", { { "ref", "value-2" }} } },
			{ { "id", "calc-3" }, { "div", { { "ref", "value-3" }} } },
			{ { "id", "calc-4" }, { "mult", { { "ref", "value-4" }} } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->calcs.size(), 4u);
	EXPECT_EQ(script->calcs[0].id, "calc-1");
	EXPECT_EQ(script->calcs[0].operation, ScriptCalc::CalcOperation::ADD);
	EXPECT_EQ(script->calcs[1].id, "calc-2");
	EXPECT_EQ(script->calcs[1].operation, ScriptCalc::CalcOperation::SUB);
	EXPECT_EQ(script->calcs[2].id, "calc-3");
	EXPECT_EQ(script->calcs[2].operation, ScriptCalc::CalcOperation::DIV);
	EXPECT_EQ(script->calcs[3].id, "calc-4");
	EXPECT_EQ(script->calcs[3].operation, ScriptCalc::CalcOperation::MULT);

	ASSERT_TRUE(script->calcs[0].value);
	EXPECT_EQ(script->calcs[0].value.get()->ref, "value-1");
	ASSERT_TRUE(script->calcs[1].value);
	EXPECT_EQ(script->calcs[1].value.get()->ref, "value-2");
	ASSERT_TRUE(script->calcs[2].value);
	EXPECT_EQ(script->calcs[2].value.get()->ref, "value-3");
	ASSERT_TRUE(script->calcs[3].value);
	EXPECT_EQ(script->calcs[3].value.get()->ref, "value-4");
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
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "values", json::array({
			{ { "id", "value-1" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-1" }, { "add", json::object() }}) })} },
			{ { "id", "value-2" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-2" }, { "sub", json::object() }}) })} },
			{ { "id", "value-3" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-3" }, { "div", json::object() }}) })} },
			{ { "id", "value-4" }, { "voltage", 1 }, { "calc", json::array({ json::object({{ "ref", "calc-ref-4" }}), json::object({{ "ref", "calc-ref-4" }, { "mult", json::object() }}) })} }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 4u);
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/0/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/1/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/2/calc/0");
	expectError(validationErrors, ValidationErrorCode::Calc_RefOrInstance, "/component-pool/values/3/calc/1");
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
