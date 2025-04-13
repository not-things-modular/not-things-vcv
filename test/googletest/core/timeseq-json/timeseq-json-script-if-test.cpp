#include "timeseq-json-shared.hpp"

json singleValueArray = json::array({ { { "ref", "value-ref-1" } } });
json doubleValueArray = json::array({ { { "ref", "value-ref-1" } }, { { "ref", "value-ref-2" } } });
json doubleValueArray2 = json::array({ { { "ref", "value-ref-3" } }, { { "ref", "value-ref-4" } } });

TEST(TimeSeqJsonScriptIf, ParseActionWithoutIfShouldWork) {
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
	EXPECT_FALSE(script->actions[0].condition);
}

TEST(TimeSeqJsonScriptIf, ParseActionWithNonObjectIfShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", "not-an-object" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_IfObject, "/component-pool/actions/0");
}

TEST(TimeSeqJsonScriptIf, ParseIfShouldFailForMissingOperation) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", json::object() } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::If_NoOperation, "/component-pool/actions/0/if");
}

void testIfPairWithNonArray(std::string compareOperator, ValidationErrorCode errorCode) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { compareOperator, "not-an-array" } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, errorCode, "/component-pool/actions/0/if");
}

void testIfPairWithEmptyArray(std::string compareOperator) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { compareOperator, json::array() } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::If_TwoValues, "/component-pool/actions/0/if/" + compareOperator);
}

void testIfPairWithSingleValue(std::string compareOperator) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { compareOperator, singleValueArray } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::If_TwoValues, "/component-pool/actions/0/if/" + compareOperator);
}

void testIfPairWithTwoValue(std::string compareOperator, ScriptIf::IfOperator ifOperator) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { compareOperator, doubleValueArray } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 0u);

	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].condition);
	EXPECT_EQ(script->actions[0].condition->ifOperator, ifOperator);
	EXPECT_TRUE(script->actions[0].condition->values);
	EXPECT_EQ(script->actions[0].condition->values->first.ref, "value-ref-1");
	EXPECT_EQ(script->actions[0].condition->values->second.ref, "value-ref-2");
}

void testIfPairWithTwoInvalidValue(std::string compareOperator) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { compareOperator, json::array({ { { "id", "value-ref-1" } }, { { "id", "value-ref-2" } } }) } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Id_NotAllowed, "/component-pool/actions/0/if/" + compareOperator + "/0");
	expectError(validationErrors, ValidationErrorCode::Id_NotAllowed, "/component-pool/actions/0/if/" + compareOperator + "/1");
}

void testIfPairWithThreeValue(std::string compareOperator) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { compareOperator, json::array({ { { "ref", "value-ref-1" } }, { { "ref", "value-ref-2" } }, { { "ref", "value-ref-3" } } }) } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::If_TwoValues, "/component-pool/actions/0/if/" + compareOperator);
}

TEST(TimeSeqJsonScriptIf, ParseIfForEq) {
	shared_ptr<Script> script;

	testIfPairWithNonArray("eq", ValidationErrorCode::If_EqArray);
	testIfPairWithEmptyArray("eq");
	testIfPairWithSingleValue("eq");
	testIfPairWithTwoValue("eq", ScriptIf::IfOperator::EQ);
	testIfPairWithThreeValue("eq");
}

TEST(TimeSeqJsonScriptIf, ParseIfForNe) {
	shared_ptr<Script> script;

	testIfPairWithNonArray("ne", ValidationErrorCode::If_NeArray);
	testIfPairWithEmptyArray("ne");
	testIfPairWithSingleValue("ne");
	testIfPairWithTwoValue("ne", ScriptIf::IfOperator::NE);
	testIfPairWithThreeValue("ne");
}

TEST(TimeSeqJsonScriptIf, ParseIfForLt) {
	shared_ptr<Script> script;

	testIfPairWithNonArray("lt", ValidationErrorCode::If_LtArray);
	testIfPairWithEmptyArray("lt");
	testIfPairWithSingleValue("lt");
	testIfPairWithTwoValue("lt", ScriptIf::IfOperator::LT);
	testIfPairWithThreeValue("lt");
}

TEST(TimeSeqJsonScriptIf, ParseIfForLte) {
	shared_ptr<Script> script;

	testIfPairWithNonArray("lte", ValidationErrorCode::If_LteArray);
	testIfPairWithEmptyArray("lte");
	testIfPairWithSingleValue("lte");
	testIfPairWithTwoValue("lte", ScriptIf::IfOperator::LTE);
	testIfPairWithThreeValue("lte");
}

TEST(TimeSeqJsonScriptIf, ParseIfForGt) {
	shared_ptr<Script> script;

	testIfPairWithNonArray("gt", ValidationErrorCode::If_GtArray);
	testIfPairWithEmptyArray("gt");
	testIfPairWithSingleValue("gt");
	testIfPairWithTwoValue("gt", ScriptIf::IfOperator::GT);
	testIfPairWithThreeValue("gt");
}

TEST(TimeSeqJsonScriptIf, ParseIfForGte) {
	shared_ptr<Script> script;

	testIfPairWithNonArray("gte", ValidationErrorCode::If_GteArray);
	testIfPairWithEmptyArray("gte");
	testIfPairWithSingleValue("gte");
	testIfPairWithTwoValue("gte", ScriptIf::IfOperator::GTE);
	testIfPairWithThreeValue("gte");
}

TEST(TimeSeqJsonScriptIf, ParseIfForAnd) {
	shared_ptr<Script> script;

	testIfPairWithNonArray("and", ValidationErrorCode::If_AndArray);
	testIfPairWithEmptyArray("and");
}

TEST(TimeSeqJsonScriptIf, ParseIfForOr) {
	shared_ptr<Script> script;

	testIfPairWithNonArray("or", ValidationErrorCode::If_OrArray);
	testIfPairWithEmptyArray("or");
}

TEST(TimeSeqJsonScriptIf, ParseAndWithSingleChildShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "and", json::array({
				{ { "eq", doubleValueArray } }
			}) } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::If_TwoValues, "/component-pool/actions/0/if/and");
}

TEST(TimeSeqJsonScriptIf, ParseAndWithTwoChildrenShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "and", json::array({
				{ { "eq", doubleValueArray } },
				{ { "ne", doubleValueArray2 } }
			}) } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 0u);

	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].condition);
	EXPECT_EQ(script->actions[0].condition->ifOperator, ScriptIf::IfOperator::AND);
	EXPECT_FALSE(script->actions[0].condition->values);
	EXPECT_TRUE(script->actions[0].condition->ifs);
	EXPECT_EQ(script->actions[0].condition->ifs->first.ifOperator, ScriptIf::IfOperator::EQ);
	ASSERT_TRUE(script->actions[0].condition->ifs->first.values);
	EXPECT_EQ(script->actions[0].condition->ifs->first.values->first.ref, "value-ref-1");
	EXPECT_EQ(script->actions[0].condition->ifs->first.values->second.ref, "value-ref-2");
	EXPECT_EQ(script->actions[0].condition->ifs->second.ifOperator, ScriptIf::IfOperator::NE);
	ASSERT_TRUE(script->actions[0].condition->ifs->second.values);
	EXPECT_EQ(script->actions[0].condition->ifs->second.values->first.ref, "value-ref-3");
	EXPECT_EQ(script->actions[0].condition->ifs->second.values->second.ref, "value-ref-4");
}

TEST(TimeSeqJsonScriptIf, ParseAndWithThreeChildShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "and", json::array({
				{ { "eq", doubleValueArray } },
				{ { "ne", doubleValueArray2 } },
				{ { "ne", doubleValueArray } }
			}) } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::If_TwoValues, "/component-pool/actions/0/if/and");
}

TEST(TimeSeqJsonScriptIf, ParseOrWithSingleChildShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "or", json::array({
				{ { "eq", doubleValueArray } }
			}) } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::If_TwoValues, "/component-pool/actions/0/if/or");
}

TEST(TimeSeqJsonScriptIf, ParseOrWithTwoChildrenShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "or", json::array({
				{ { "eq", doubleValueArray } },
				{ { "ne", doubleValueArray2 } }
			}) } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 0u);

	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].condition);
	EXPECT_EQ(script->actions[0].condition->ifOperator, ScriptIf::IfOperator::OR);
	EXPECT_FALSE(script->actions[0].condition->values);
	EXPECT_TRUE(script->actions[0].condition->ifs);
	EXPECT_EQ(script->actions[0].condition->ifs->first.ifOperator, ScriptIf::IfOperator::EQ);
	ASSERT_TRUE(script->actions[0].condition->ifs->first.values);
	EXPECT_EQ(script->actions[0].condition->ifs->first.values->first.ref, "value-ref-1");
	EXPECT_EQ(script->actions[0].condition->ifs->first.values->second.ref, "value-ref-2");
	EXPECT_EQ(script->actions[0].condition->ifs->second.ifOperator, ScriptIf::IfOperator::NE);
	ASSERT_TRUE(script->actions[0].condition->ifs->second.values);
	EXPECT_EQ(script->actions[0].condition->ifs->second.values->first.ref, "value-ref-3");
	EXPECT_EQ(script->actions[0].condition->ifs->second.values->second.ref, "value-ref-4");
}

TEST(TimeSeqJsonScriptIf, ParseOrWithThreeChildShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "or", json::array({
				{ { "eq", doubleValueArray } },
				{ { "ne", doubleValueArray2 } },
				{ { "ne", doubleValueArray } }
			}) } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::If_TwoValues, "/component-pool/actions/0/if/or");
}

TEST(TimeSeqJsonScriptIf, ParseWithoutToleranceShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "eq", doubleValueArray } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 0u);

	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].condition);
	EXPECT_FALSE(script->actions[0].condition->tolerance);
}

TEST(TimeSeqJsonScriptIf, ParseWithNonNumericToleranceShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "tolerance", "not-a-number", }, { "eq", doubleValueArray } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::If_ToleranceNumber, "/component-pool/actions/0/if");
}

TEST(TimeSeqJsonScriptIf, ParseWithNegativeToleranceShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "tolerance", -0.1 }, { "eq", doubleValueArray } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::If_ToleranceNumber, "/component-pool/actions/0/if");
}

TEST(TimeSeqJsonScriptIf, ParseWithZeroToleranceShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "tolerance", 0 }, { "eq", doubleValueArray } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 0u);

	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].condition);
	ASSERT_TRUE(script->actions[0].condition->tolerance);
	EXPECT_EQ(*script->actions[0].condition->tolerance.get(), 0);
}

TEST(TimeSeqJsonScriptIf, ParseWithPositiveToleranceShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "tolerance", 0.1f }, { "eq", doubleValueArray } } } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 0u);

	ASSERT_EQ(script->actions.size(), 1u);
	ASSERT_TRUE(script->actions[0].condition);
	ASSERT_TRUE(script->actions[0].condition->tolerance);
	EXPECT_EQ(*script->actions[0].condition->tolerance.get(), 0.1f);
}

TEST(TimeSeqJsonScriptIf, ParseShouldFailWithMultipleOperators) {
	json dualIf = json::array({ 
		{ { "eq", doubleValueArray } },
		{ { "ne", doubleValueArray2 } }
	});

	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "trigger", "trigger-name" }, { "if", { { "eq", doubleValueArray }, { "ne", doubleValueArray } } } },
			{ { "id", "action-2" }, { "trigger", "trigger-name" }, { "if", { { "eq", doubleValueArray }, { "lt", doubleValueArray } } } },
			{ { "id", "action-3" }, { "trigger", "trigger-name" }, { "if", { { "eq", doubleValueArray }, { "lte", doubleValueArray } } } },
			{ { "id", "action-4" }, { "trigger", "trigger-name" }, { "if", { { "eq", doubleValueArray }, { "gt", doubleValueArray } } } },
			{ { "id", "action-5" }, { "trigger", "trigger-name" }, { "if", { { "eq", doubleValueArray }, { "gte", doubleValueArray } } } },
			{ { "id", "action-6" }, { "trigger", "trigger-name" }, { "if", { { "eq", doubleValueArray }, { "and", dualIf } } } },
			{ { "id", "action-7" }, { "trigger", "trigger-name" }, { "if", { { "eq", doubleValueArray }, { "or", dualIf } } } },

			{ { "id", "action-8" }, { "trigger", "trigger-name" }, { "if", { { "ne", doubleValueArray }, { "lt", doubleValueArray } } } },
			{ { "id", "action-9" }, { "trigger", "trigger-name" }, { "if", { { "ne", doubleValueArray }, { "lte", doubleValueArray } } } },
			{ { "id", "action-10" }, { "trigger", "trigger-name" }, { "if", { { "ne", doubleValueArray }, { "gt", doubleValueArray } } } },
			{ { "id", "action-11" }, { "trigger", "trigger-name" }, { "if", { { "ne", doubleValueArray }, { "gte", doubleValueArray } } } },
			{ { "id", "action-12" }, { "trigger", "trigger-name" }, { "if", { { "ne", doubleValueArray }, { "and", dualIf } } } },
			{ { "id", "action-13" }, { "trigger", "trigger-name" }, { "if", { { "ne", doubleValueArray }, { "or", dualIf } } } },

			{ { "id", "action-14" }, { "trigger", "trigger-name" }, { "if", { { "lt", doubleValueArray }, { "lte", doubleValueArray } } } },
			{ { "id", "action-15" }, { "trigger", "trigger-name" }, { "if", { { "lt", doubleValueArray }, { "gt", doubleValueArray } } } },
			{ { "id", "action-16" }, { "trigger", "trigger-name" }, { "if", { { "lt", doubleValueArray }, { "gte", doubleValueArray } } } },
			{ { "id", "action-17" }, { "trigger", "trigger-name" }, { "if", { { "lt", doubleValueArray }, { "and", dualIf } } } },
			{ { "id", "action-18" }, { "trigger", "trigger-name" }, { "if", { { "lt", doubleValueArray }, { "or", dualIf } } } },

			{ { "id", "action-19" }, { "trigger", "trigger-name" }, { "if", { { "lte", doubleValueArray }, { "gt", doubleValueArray } } } },
			{ { "id", "action-20" }, { "trigger", "trigger-name" }, { "if", { { "lte", doubleValueArray }, { "gte", doubleValueArray } } } },
			{ { "id", "action-21" }, { "trigger", "trigger-name" }, { "if", { { "lte", doubleValueArray }, { "and", dualIf } } } },
			{ { "id", "action-22" }, { "trigger", "trigger-name" }, { "if", { { "lte", doubleValueArray }, { "or", dualIf } } } },

			{ { "id", "action-23" }, { "trigger", "trigger-name" }, { "if", { { "gt", doubleValueArray }, { "gte", doubleValueArray } } } },
			{ { "id", "action-24" }, { "trigger", "trigger-name" }, { "if", { { "gt", doubleValueArray }, { "and", dualIf } } } },
			{ { "id", "action-25" }, { "trigger", "trigger-name" }, { "if", { { "gt", doubleValueArray }, { "or", dualIf } } } },

			{ { "id", "action-26" }, { "trigger", "trigger-name" }, { "if", { { "gte", doubleValueArray }, { "and", dualIf } } } },
			{ { "id", "action-27" }, { "trigger", "trigger-name" }, { "if", { { "gte", doubleValueArray }, { "or", dualIf } } } },

			{ { "id", "action-28" }, { "trigger", "trigger-name" }, { "if", { { "and", dualIf }, { "or", dualIf } } } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);

	ASSERT_EQ(validationErrors.size(), 28u);
	for (unsigned int i = 0; i < 28; i++) {
		expectError(validationErrors, ValidationErrorCode::If_MultpleOperations, "/component-pool/actions/" + std::to_string(i) + "/if");
	}
}
