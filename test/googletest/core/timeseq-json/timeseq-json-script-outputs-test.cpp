#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptOutput, ParseShouldSucceedWithouOutputs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->outputs.size(), 0u);
}

TEST(TimeSeqJsonScriptOutput, ParseShouldSucceedWithEmptyOutputs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->outputs.size(), 0u);
}

TEST(TimeSeqJsonScriptOutput, ParseOutputsShouldNotAllowRefAndRequireIdOnRoot) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "ref", "output-ref" } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_GT(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/component-pool/outputs/0");
	expectError(validationErrors, ValidationErrorCode::Ref_NotAllowed, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputsShouldFailOnNonArrayOutputs) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", "not-an-array" }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_OutputsArray, "/component-pool");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputsShouldFailOnNonObjectOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 1 } },
			"not-an-object",
			{ { "id", "output-2" }, { "index", 2 } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_OutputObject, "/component-pool/outputs/1");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailWithoutIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexNumber, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailWithNonNumericIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", "not-a-number" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexNumber, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailWithANegativeIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", -1 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexNumber, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailWithAZeroIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 0 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexRange, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailWithANonIntegerIndex) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 1.1f } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexNumber, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldParseIndexInRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 1 } },
			{ { "id", "output-2" }, { "index", 2 } },
			{ { "id", "output-3" }, { "index", 3 } },
			{ { "id", "output-4" }, { "index", 4 } },
			{ { "id", "output-5" }, { "index", 5 } },
			{ { "id", "output-6" }, { "index", 6 } },
			{ { "id", "output-7" }, { "index", 7 } },
			{ { "id", "output-8" }, { "index", 8 } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->outputs.size(), 8u);
	EXPECT_EQ(script->outputs[0].id, "output-1");
	EXPECT_EQ(script->outputs[0].index, 1);
	EXPECT_FALSE(script->outputs[0].channel);
	EXPECT_EQ(script->outputs[1].id, "output-2");
	EXPECT_EQ(script->outputs[1].index, 2);
	EXPECT_FALSE(script->outputs[1].channel);
	EXPECT_EQ(script->outputs[2].id, "output-3");
	EXPECT_EQ(script->outputs[2].index, 3);
	EXPECT_FALSE(script->outputs[2].channel);
	EXPECT_EQ(script->outputs[3].id, "output-4");
	EXPECT_EQ(script->outputs[3].index, 4);
	EXPECT_FALSE(script->outputs[3].channel);
	EXPECT_EQ(script->outputs[4].id, "output-5");
	EXPECT_EQ(script->outputs[4].index, 5);
	EXPECT_FALSE(script->outputs[4].channel);
	EXPECT_EQ(script->outputs[5].id, "output-6");
	EXPECT_EQ(script->outputs[5].index, 6);
	EXPECT_FALSE(script->outputs[5].channel);
	EXPECT_EQ(script->outputs[6].id, "output-7");
	EXPECT_EQ(script->outputs[6].index, 7);
	EXPECT_FALSE(script->outputs[6].channel);
	EXPECT_EQ(script->outputs[7].id, "output-8");
	EXPECT_EQ(script->outputs[7].index, 8);
	EXPECT_FALSE(script->outputs[7].channel);
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailOnDuplicateIds) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 1 } },
			{ { "id", "output-2" }, { "index", 1 } },
			{ { "id", "output-1" }, { "index", 1 } },
			{ { "id", "output-2" }, { "index", 1 } },
			{ { "id", "output-1" }, { "index", 1 } },
			{ { "id", "output-3" }, { "index", 1 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/outputs/2");
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/outputs/3");
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/outputs/4");
	EXPECT_NE(validationErrors[0].message.find("'output-1'"), std::string::npos);
	EXPECT_NE(validationErrors[1].message.find("'output-2'"), std::string::npos);
	EXPECT_NE(validationErrors[2].message.find("'output-1'"), std::string::npos);
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailWithIndexAboveRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 9 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexRange, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailWithNonNumericChannel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 4 }, { "channel", "not-a-number"} },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_ChannelNumber, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailWithNegativeChannel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 4 }, { "channel", -1} },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_ChannelNumber, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailWithZeroChannel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 4 }, { "channel", 0} },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_ChannelRange, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailWithFloatChannel) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 4 }, { "channel", 0} },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_ChannelRange, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldParseChannelInRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 3 }, { "channel", 1 } },
			{ { "id", "output-2" }, { "index", 4 }, { "channel", 3 } },
			{ { "id", "output-3" }, { "index", 5 }, { "channel", 5 } },
			{ { "id", "output-4" }, { "index", 6 }, { "channel", 7 } },
			{ { "id", "output-5" }, { "index", 7 }, { "channel", 9 } },
			{ { "id", "output-6" }, { "index", 8 }, { "channel", 11 } },
			{ { "id", "output-7" }, { "index", 1 }, { "channel", 13 } },
			{ { "id", "output-8" }, { "index", 2 }, { "channel", 15 } },
			{ { "id", "output-9" }, { "index", 3 }, { "channel", 16 } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->outputs.size(), 9u);
	EXPECT_EQ(script->outputs[0].id, "output-1");
	EXPECT_EQ(script->outputs[0].index, 3);
	ASSERT_TRUE(script->outputs[0].channel);
	EXPECT_EQ(*script->outputs[0].channel.get(), 1);
	EXPECT_EQ(script->outputs[1].id, "output-2");
	EXPECT_EQ(script->outputs[1].index, 4);
	ASSERT_TRUE(script->outputs[1].channel);
	EXPECT_EQ(*script->outputs[1].channel.get(), 3);
	EXPECT_EQ(script->outputs[2].id, "output-3");
	EXPECT_EQ(script->outputs[2].index, 5);
	ASSERT_TRUE(script->outputs[2].channel);
	EXPECT_EQ(*script->outputs[2].channel.get(), 5);
	EXPECT_EQ(script->outputs[3].id, "output-4");
	EXPECT_EQ(script->outputs[3].index, 6);
	ASSERT_TRUE(script->outputs[3].channel);
	EXPECT_EQ(*script->outputs[3].channel.get(), 7);
	EXPECT_EQ(script->outputs[4].id, "output-5");
	EXPECT_EQ(script->outputs[4].index, 7);
	ASSERT_TRUE(script->outputs[4].channel);
	EXPECT_EQ(*script->outputs[4].channel.get(), 9);
	EXPECT_EQ(script->outputs[5].id, "output-6");
	EXPECT_EQ(script->outputs[5].index, 8);
	ASSERT_TRUE(script->outputs[5].channel);
	EXPECT_EQ(*script->outputs[5].channel.get(), 11);
	EXPECT_EQ(script->outputs[6].id, "output-7");
	EXPECT_EQ(script->outputs[6].index, 1);
	ASSERT_TRUE(script->outputs[6].channel);
	EXPECT_EQ(*script->outputs[6].channel.get(), 13);
	EXPECT_EQ(script->outputs[7].id, "output-8");
	EXPECT_EQ(script->outputs[7].index, 2);
	ASSERT_TRUE(script->outputs[7].channel);
	EXPECT_EQ(*script->outputs[7].channel.get(), 15);
	EXPECT_EQ(script->outputs[8].id, "output-9");
	EXPECT_EQ(script->outputs[8].index, 3);
	ASSERT_TRUE(script->outputs[8].channel);
	EXPECT_EQ(*script->outputs[8].channel.get(), 16);
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldFailWithChannelAboveRange) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 2 }, { "channel", 17 } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_ChannelRange, "/component-pool/outputs/0");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldNotAllowIdOnNonRootOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "timing", "start" },
				{ "set-value", {
					{ "value", { { "voltage", 1 } } },
					{ "output", {
						{ "id", "output-id" },
						{ "index", 1 }
					} }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Id_NotAllowed, "/component-pool/actions/0/set-value/output");
}

TEST(TimeSeqJsonScriptOutput, ParseRefInputShouldNotAllowIndexProperty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "timing", "start" },
				{ "set-value", {
					{ "value", { { "voltage", 1 } } },
					{ "output", {
						{ "ref", "output-id" },
						{ "index", 1 }
					} }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_RefOrInstance, "/component-pool/actions/0/set-value/output");
}

TEST(TimeSeqJsonScriptOutput, ParseRefInputShouldNotAllowChannelProperty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{
				{ "id", "action-1" },
				{ "timing", "start" },
				{ "set-value", {
					{ "value", { { "voltage", 1 } } },
					{ "output", {
						{ "ref", "output-id" },
						{ "channel", 1 }
					} }
				} }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_RefOrInstance, "/component-pool/actions/0/set-value/output");
}

TEST(TimeSeqJsonScriptOutput, ParseOutputWithUnknownPropertyShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{
				{ "id", "output-1" },
				{ "index", 1 },
				{ "unknown-prop", "value" }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/outputs/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptOutput, ParseOutputWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{
				{ "id", "output-1" },
				{ "index", 1 },
				{ "unknown-prop-1", "value" },
				{ "unknown-prop-2", { { "child", "object" } } }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/component-pool/outputs/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptOutput, ParseOutputShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "outputs", json::array({
			{
				{ "id", "output-1" },
				{ "index", 1 },
				{ "x-unknown-prop-1", "value" },
				{ "x-unknown-prop-2", { { "child", "object" } } }
			}
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
}
