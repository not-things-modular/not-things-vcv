#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptWithNoSegmentBlocksShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	EXPECT_EQ(script->segmentBlocks.size(), 0);
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptWithEmptySegmentBlocksShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = json::array();

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	EXPECT_EQ(script->segmentBlocks.size(), 0);
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptWithNonArraySegmentBlocksShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = "not-an-array";

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Script_SegmentBlocksArray, "/");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptWithNonObjectSegmentBlockShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array() } },
		"not-an-object",
		{ { "id", "segment-block-2" }, { "segments", json::array() } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Script_SegmentBlockObject, "/segment-blocks/1");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldSegmentBlocks) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array() } },
		{ { "id", "segment-block-2" }, { "segments", json::array() } },
		{ { "id", "segment-block-3" }, { "segments", json::array() } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segmentBlocks.size(), 3);
	EXPECT_EQ(script->segmentBlocks[0].id, "segment-block-1");
	EXPECT_EQ(script->segmentBlocks[1].id, "segment-block-2");
	EXPECT_EQ(script->segmentBlocks[2].id, "segment-block-3");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldNotAllowRefSegmentBlocks) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = json::array({
		{ { "ref", "segment-block-1" } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 3);
	expectError(validationErrors, ValidationErrorCode::Ref_NotAllowed, "/segment-blocks/0");
	expectError(validationErrors, ValidationErrorCode::Id_String, "/segment-blocks/0");
	expectError(validationErrors, ValidationErrorCode::SegmentBlock_SegmentsArray, "/segment-blocks/0");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldShouldFailWithoutId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = json::array({
		{ { "segments", json::array() } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/segment-blocks/0");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldAllowEmptySegmentsArrayAndNoRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array() } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segmentBlocks.size(), 1);
	EXPECT_EQ(script->segmentBlocks[0].segments.size(), 0);
	EXPECT_FALSE(script->segmentBlocks[0].repeat);
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldFailOnNonNumericRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "repeat", "not-a-number" }, { "segments", json::array() } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::SegmentBlock_RepeatNumber, "/segment-blocks/0");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldFailOnNegativeRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "repeat", -1 }, { "segments", json::array() } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::SegmentBlock_RepeatNumber, "/segment-blocks/0");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldFailOnFloatRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "repeat", 1.1 }, { "segments", json::array() } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::SegmentBlock_RepeatNumber, "/segment-blocks/0");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldSucceedOnPositiveIntegerRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "repeat", 420 }, { "segments", json::array() } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segmentBlocks.size(), 1);
	EXPECT_EQ(script->segmentBlocks[0].segments.size(), 0);
	ASSERT_TRUE(script->segmentBlocks[0].repeat);
	EXPECT_EQ(*script->segmentBlocks[0].repeat.get(), 420);
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldFailOnNonObjectSegment) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	// Both ref and inline segments should be accepted
	json["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array({
			{ { "ref", "segment-id-1" } },
			"not-an-object",
			{ { "ref", "segment-id-2" } }
		}) } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::SegmentBlock_SegmentObject, "/segment-blocks/0/segments/1");
}

TEST(TimeSeqJsonScriptSegmentBlocks, ParseScriptShouldParseSegments) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	// Both ref and inline segments should be accepted
	json["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array({
			{ { "ref", "segment-id" } },
			{ { "duration", { { "samples", 1 } } }, { "actions", json::array() } }
		}) } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segmentBlocks.size(), 1);
	ASSERT_EQ(script->segmentBlocks[0].segments.size(), 2);
	EXPECT_EQ(script->segmentBlocks[0].segments[0].ref, "segment-id");
	EXPECT_EQ(script->segmentBlocks[0].segments[1].ref, "");
	EXPECT_TRUE(script->segmentBlocks[0].segments[1].duration.samples);
}
