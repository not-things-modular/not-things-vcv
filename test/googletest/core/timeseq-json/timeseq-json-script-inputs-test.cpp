#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptInput, ParseShouldSucceedWithoutInputs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	EXPECT_EQ(script->inputs.size(), 0u);
}

TEST(TimeSeqJsonScriptInput, ParseShouldSucceedWithEmptyInputs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	EXPECT_EQ(script->inputs.size(), 0u);
}

TEST(TimeSeqJsonScriptInput, ParseInputsShouldNotAllowRefAndRequireIdOnRoot) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", {
			{ { "ref", "input-ref" } }
		} }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/component-pool/inputs/0");
	expectError(validationErrors, ValidationErrorCode::Ref_NotAllowed, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputsShouldFailOnNonArrayInputs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", "not-an-array" }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_InputsArray, "/component-pool");
}

TEST(TimeSeqJsonScriptInput, ParseInputsShouldFailOnNonObjectInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 1 } },
			"not-an-object",
			{ { "id", "input-1" }, { "index", 2 } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_InputObject, "/component-pool/inputs/1");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldFailWithoutIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_IndexNumber, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldFailWithNonNumericIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", "not-a-number" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_IndexNumber, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldFailWithANegativeIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", -1 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_IndexNumber, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldFailWithAZeroIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 0 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_IndexRange, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldFailWithANonIntegerIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 1.1f } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_IndexNumber, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldParseIndexInRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 1 } },
			{ { "id", "input-2" }, { "index", 2 } },
			{ { "id", "input-3" }, { "index", 3 } },
			{ { "id", "input-4" }, { "index", 4 } },
			{ { "id", "input-5" }, { "index", 5 } },
			{ { "id", "input-6" }, { "index", 6 } },
			{ { "id", "input-7" }, { "index", 7 } },
			{ { "id", "input-8" }, { "index", 8 } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->inputs.size(), 8u);
	EXPECT_EQ(script->inputs[0].id, "input-1");
	EXPECT_EQ(script->inputs[0].index, 1);
	EXPECT_FALSE(script->inputs[0].channel);
	EXPECT_EQ(script->inputs[1].id, "input-2");
	EXPECT_EQ(script->inputs[1].index, 2);
	EXPECT_FALSE(script->inputs[1].channel);
	EXPECT_EQ(script->inputs[2].id, "input-3");
	EXPECT_EQ(script->inputs[2].index, 3);
	EXPECT_FALSE(script->inputs[2].channel);
	EXPECT_EQ(script->inputs[3].id, "input-4");
	EXPECT_EQ(script->inputs[3].index, 4);
	EXPECT_FALSE(script->inputs[3].channel);
	EXPECT_EQ(script->inputs[4].id, "input-5");
	EXPECT_EQ(script->inputs[4].index, 5);
	EXPECT_FALSE(script->inputs[4].channel);
	EXPECT_EQ(script->inputs[5].id, "input-6");
	EXPECT_EQ(script->inputs[5].index, 6);
	EXPECT_FALSE(script->inputs[5].channel);
	EXPECT_EQ(script->inputs[6].id, "input-7");
	EXPECT_EQ(script->inputs[6].index, 7);
	EXPECT_FALSE(script->inputs[6].channel);
	EXPECT_EQ(script->inputs[7].id, "input-8");
	EXPECT_EQ(script->inputs[7].index, 8);
	EXPECT_FALSE(script->inputs[7].channel);
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldFailWithIndexAboveRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 9 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_IndexRange, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldFailWithNonNumericChannel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 4 }, { "channel", "not-a-number"} },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_ChannelNumber, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldFailWithNegativeChannel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 4 }, { "channel", -1} },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_ChannelNumber, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldFailWithZeroChannel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 4 }, { "channel", 0} },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_ChannelRange, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldFailWithFloatChannel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 4 }, { "channel", 0} },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_ChannelRange, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldParseChannelInRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 3 }, { "channel", 1 } },
			{ { "id", "input-2" }, { "index", 4 }, { "channel", 3 } },
			{ { "id", "input-3" }, { "index", 5 }, { "channel", 5 } },
			{ { "id", "input-4" }, { "index", 6 }, { "channel", 7 } },
			{ { "id", "input-5" }, { "index", 7 }, { "channel", 9 } },
			{ { "id", "input-6" }, { "index", 8 }, { "channel", 11 } },
			{ { "id", "input-7" }, { "index", 1 }, { "channel", 13 } },
			{ { "id", "input-8" }, { "index", 2 }, { "channel", 15 } },
			{ { "id", "input-9" }, { "index", 3 }, { "channel", 16 } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->inputs.size(), 9u);
	EXPECT_EQ(script->inputs[0].id, "input-1");
	EXPECT_EQ(script->inputs[0].index, 3);
	ASSERT_TRUE(script->inputs[0].channel);
	EXPECT_EQ(*script->inputs[0].channel.get(), 1);
	EXPECT_EQ(script->inputs[1].id, "input-2");
	EXPECT_EQ(script->inputs[1].index, 4);
	ASSERT_TRUE(script->inputs[1].channel);
	EXPECT_EQ(*script->inputs[1].channel.get(), 3);
	EXPECT_EQ(script->inputs[2].id, "input-3");
	EXPECT_EQ(script->inputs[2].index, 5);
	ASSERT_TRUE(script->inputs[2].channel);
	EXPECT_EQ(*script->inputs[2].channel.get(), 5);
	EXPECT_EQ(script->inputs[3].id, "input-4");
	EXPECT_EQ(script->inputs[3].index, 6);
	ASSERT_TRUE(script->inputs[3].channel);
	EXPECT_EQ(*script->inputs[3].channel.get(), 7);
	EXPECT_EQ(script->inputs[4].id, "input-5");
	EXPECT_EQ(script->inputs[4].index, 7);
	ASSERT_TRUE(script->inputs[4].channel);
	EXPECT_EQ(*script->inputs[4].channel.get(), 9);
	EXPECT_EQ(script->inputs[5].id, "input-6");
	EXPECT_EQ(script->inputs[5].index, 8);
	ASSERT_TRUE(script->inputs[5].channel);
	EXPECT_EQ(*script->inputs[5].channel.get(), 11);
	EXPECT_EQ(script->inputs[6].id, "input-7");
	EXPECT_EQ(script->inputs[6].index, 1);
	ASSERT_TRUE(script->inputs[6].channel);
	EXPECT_EQ(*script->inputs[6].channel.get(), 13);
	EXPECT_EQ(script->inputs[7].id, "input-8");
	EXPECT_EQ(script->inputs[7].index, 2);
	ASSERT_TRUE(script->inputs[7].channel);
	EXPECT_EQ(*script->inputs[7].channel.get(), 15);
	EXPECT_EQ(script->inputs[8].id, "input-9");
	EXPECT_EQ(script->inputs[8].index, 3);
	ASSERT_TRUE(script->inputs[8].channel);
	EXPECT_EQ(*script->inputs[8].channel.get(), 16);
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldFailWithChannelAboveRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 2 }, { "channel", 17 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_ChannelRange, "/component-pool/inputs/0");
}

TEST(TimeSeqJsonScriptInput, ParseInputShouldNotAllowIdOnNonRootInput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = json::array({
		{
			{ "id", "input-trigger-1" },
			{ "input", {
				{ "id", "input-id" },
				{ "index", 1 } }
			}
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Id_NotAllowed, "/input-triggers/0/input");
}

TEST(TimeSeqJsonScriptInput, ParseRefInputShouldNotAllowIndexProperty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = json::array({
		{
			{ "id", "input-trigger-1" },
			{ "input", {
				{ "ref", "input-ref" },
				{ "index", 1 } }
			}
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_RefOrInstance, "/input-triggers/0/input");
}

TEST(TimeSeqJsonScriptInput, ParseRefInputShouldNotAllowChannelProperty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["input-triggers"] = json::array({
		{
			{ "id", "input-trigger-1" },
			{ "input", {
				{ "ref", "input-ref" },
				{ "channel", 1 } }
			}
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Input_RefOrInstance, "/input-triggers/0/input");
}
