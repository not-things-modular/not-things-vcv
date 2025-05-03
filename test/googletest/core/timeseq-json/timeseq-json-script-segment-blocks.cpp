#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptWithNoSegmentBlocksShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->segmentBlocks.size(), 0u);
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptWithEmptySegmentBlocksShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", json::array() }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script->segmentBlocks.size(), 0u);
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptWithNonArraySegmentBlocksShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", "not-an-array" }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_SegmentBlocksArray, "/component-pool");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptWithNonObjectSegmentBlockShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "id", "segment-block-1" }, { "segments", json::array() } },
			"not-an-object",
			{ { "id", "segment-block-2" }, { "segments", json::array() } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_SegmentBlockObject, "/component-pool/segment-blocks/1");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldParseSegmentBlocks) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "id", "segment-block-1" }, { "segments", json::array() } },
			{ { "id", "segment-block-2" }, { "segments", json::array() } },
			{ { "id", "segment-block-3" }, { "segments", json::array() } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->segmentBlocks.size(), 3u);
	EXPECT_EQ(script->segmentBlocks[0].id, "segment-block-1");
	EXPECT_EQ(script->segmentBlocks[1].id, "segment-block-2");
	EXPECT_EQ(script->segmentBlocks[2].id, "segment-block-3");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldFailOnDuplicateSegmentBlockIds) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "id", "segment-block-1" }, { "segments", json::array() } },
			{ { "id", "segment-block-2" }, { "segments", json::array() } },
			{ { "id", "segment-block-1" }, { "segments", json::array() } },
			{ { "id", "segment-block-4" }, { "segments", json::array() } },
			{ { "id", "segment-block-1" }, { "segments", json::array() } },
			{ { "id", "segment-block-3" }, { "segments", json::array() } },
			{ { "id", "segment-block-2" }, { "segments", json::array() } }
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/segment-blocks/2");
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/segment-blocks/4");
	expectError(validationErrors, ValidationErrorCode::Id_Duplicate, "/component-pool/segment-blocks/6");

	EXPECT_NE(validationErrors[0].message.find("'segment-block-1'"), std::string::npos);
	EXPECT_NE(validationErrors[1].message.find("'segment-block-1'"), std::string::npos);
	EXPECT_NE(validationErrors[2].message.find("'segment-block-2'"), std::string::npos);
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldNotAllowRefSegmentBlocks) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "ref", "segment-block-1" } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotAllowed, "/component-pool/segment-blocks/0");
	expectError(validationErrors, ValidationErrorCode::Id_String, "/component-pool/segment-blocks/0");
	expectError(validationErrors, ValidationErrorCode::SegmentBlock_SegmentsArray, "/component-pool/segment-blocks/0");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldShouldFailWithoutId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "segments", json::array() } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/component-pool/segment-blocks/0");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldAllowEmptySegmentsArrayAndNoRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "id", "segment-block-1" }, { "segments", json::array() } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->segmentBlocks.size(), 1u);
	EXPECT_EQ(script->segmentBlocks[0].segments.size(), 0u);
	EXPECT_FALSE(script->segmentBlocks[0].repeat);
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldFailOnNonNumericRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "id", "segment-block-1" }, { "repeat", "not-a-number" }, { "segments", json::array() } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SegmentBlock_RepeatNumber, "/component-pool/segment-blocks/0");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldFailOnNegativeRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "id", "segment-block-1" }, { "repeat", -1 }, { "segments", json::array() } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SegmentBlock_RepeatNumber, "/component-pool/segment-blocks/0");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldFailOnFloatRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "id", "segment-block-1" }, { "repeat", 1.1 }, { "segments", json::array() } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SegmentBlock_RepeatNumber, "/component-pool/segment-blocks/0");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldSucceedOnPositiveIntegerRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "id", "segment-block-1" }, { "repeat", 420 }, { "segments", json::array() } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->segmentBlocks.size(), 1u);
	EXPECT_EQ(script->segmentBlocks[0].segments.size(), 0u);
	ASSERT_TRUE(script->segmentBlocks[0].repeat);
	EXPECT_EQ(*script->segmentBlocks[0].repeat.get(), 420);
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldFailOnNonObjectSegment) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	// Both ref and inline segments should be accepted
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "id", "segment-block-1" }, { "segments", json::array({
				{ { "ref", "segment-id-1" } },
				"not-an-object",
				{ { "ref", "segment-id-2" } }
			}) } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SegmentBlock_SegmentObject, "/component-pool/segment-blocks/0/segments/1");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldParseSegments) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	// Both ref and inline segments should be accepted
	json["component-pool"] = {
		{ "segment-blocks", json::array({
			{ { "id", "segment-block-1" }, { "segments", json::array({
				{ { "ref", "segment-id" } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array() } }
			}) } },
		}) }
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->segmentBlocks.size(), 1u);
	ASSERT_EQ(script->segmentBlocks[0].segments.size(), 2u);
	EXPECT_EQ(script->segmentBlocks[0].segments[0].ref, "segment-id");
	EXPECT_EQ(script->segmentBlocks[0].segments[1].ref, "");
	EXPECT_TRUE(script->segmentBlocks[0].segments[1].duration.samples);
}
