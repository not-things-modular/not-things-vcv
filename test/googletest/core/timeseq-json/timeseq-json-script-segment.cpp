#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptSegment, ParseScriptWithNoSegmentsShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	EXPECT_EQ(script->segments.size(), 0);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptWithEmptySegmentsShouldSucceed) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array();

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	EXPECT_EQ(script->segments.size(), 0);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptWithNonArrayEmptySegmentsShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = "not-an-array";

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Script_SegmentsArray, "/");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptSegmentsChildrenCanNotBeRefsAndMustHaveIf) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "ref", "segment-ref" } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptRefSegmentsCanNotHaveOtherProperties) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	// A ref segment is not possible under the root segments array, so test this inside a segment-block instead.
	json["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array( {
			{ { "ref", "segment-1" }, { "duration", "some-duration" } },
			{ { "ref", "segment-1" }, { "actions", "some-actions" } },
			{ { "ref", "segment-1" }, { "segment-block", "some-segment-block" } },
			{ { "ref", "segment-1" }, { "disable-ui", "some-disable-ui" } }
		} ) } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 4);
	expectError(validationErrors, ValidationErrorCode::Segment_RefOrInstance, "/segment-blocks/0/segments/0");
	expectError(validationErrors, ValidationErrorCode::Segment_RefOrInstance, "/segment-blocks/0/segments/1");
	expectError(validationErrors, ValidationErrorCode::Segment_RefOrInstance, "/segment-blocks/0/segments/2");
	expectError(validationErrors, ValidationErrorCode::Segment_RefOrInstance, "/segment-blocks/0/segments/3");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptIdShouldNotBeAllowedOutsideOfRootSegments) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array( {
			{ { "id", "segment-1" }, { "duration", { { "millis", 1 } } } }
		} ) } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Id_NotAllowed, "/segment-blocks/0/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptWithNonObjectSegmentShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", 1 } } } },
		"not-an-object",
		{ { "id", "segment-3" }, { "duration", { { "millis", 1 } } } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Script_SegmentObject, "/segments/1");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseSegments) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", 1 } } } },
		{ { "id", "segment-2" }, { "duration", { { "millis", 1 } } } },
		{ { "id", "segment-3" }, { "duration", { { "millis", 1 } } } },
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 3);
	EXPECT_EQ(script->segments[0].id, "segment-1");
	EXPECT_EQ(script->segments[1].id, "segment-2");
	EXPECT_EQ(script->segments[2].id, "segment-3");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithoutSegmentId) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "duration", { { "millis", 1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailSegmentDuration) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "actions", json::array() } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Segment_DurationObject, "/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithNonObjectSegmentDuration) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", "not-an-object" } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Segment_DurationObject, "/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNoChildren) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", json::object() } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_NoSamplesOrMillisOrBeatsOrHz, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithSamplesAndMillis) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 }, { "millis", 1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_EitherSamplesOrMillisOrBeatsOrHz, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithSamplesAndBeats) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 }, { "beats", 1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_EitherSamplesOrMillisOrBeatsOrHz, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithSamplesAndHz) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 }, { "hz", 100 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_EitherSamplesOrMillisOrBeatsOrHz, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithMillisAndBeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", 1 }, { "beats", 1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_EitherSamplesOrMillisOrBeatsOrHz, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithMillisAndHz) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", 1 }, { "hz", 100 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_EitherSamplesOrMillisOrBeatsOrHz, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithBeatsAndHz) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 1 }, { "hz", 100 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_EitherSamplesOrMillisOrBeatsOrHz, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithBarsButNoBeats) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", 1 }, { "bars", 1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_BarsRequiresBeats, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNoBarsButZeroBeats) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 0 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_BeatsNotZero, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNegativeSamples) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", -1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0);
	expectError(validationErrors, ValidationErrorCode::Duration_SamplesNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithZeroSamples) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 0 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0);
	expectError(validationErrors, ValidationErrorCode::Duration_SamplesNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithFloatSamples) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1.1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0);
	expectError(validationErrors, ValidationErrorCode::Duration_SamplesNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNegativeMillis) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", -1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0);
	expectError(validationErrors, ValidationErrorCode::Duration_MillisNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithZeroMillis) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", 0 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_GT(validationErrors.size(), 0);
	expectError(validationErrors, ValidationErrorCode::Duration_MillisNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseDurationWithPositiveSamples) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 6942 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	ASSERT_TRUE(script->segments[0].duration.samples);
	EXPECT_EQ(*script->segments[0].duration.samples.get(), 6942);
	EXPECT_FALSE(script->segments[0].duration.bars);
	EXPECT_FALSE(script->segments[0].duration.beats);
	EXPECT_FALSE(script->segments[0].duration.hz);
	EXPECT_FALSE(script->segments[0].duration.millis);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseDurationWithMillis) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", 4269 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	ASSERT_TRUE(script->segments[0].duration.millis);
	EXPECT_EQ(*script->segments[0].duration.millis.get(), 4269);
	EXPECT_FALSE(script->segments[0].duration.bars);
	EXPECT_FALSE(script->segments[0].duration.beats);
	EXPECT_FALSE(script->segments[0].duration.hz);
	EXPECT_FALSE(script->segments[0].duration.samples);
}


// TODO: Next up -> parse duration section (including missing/non-object duration)


// TEST(TimeSeqJsonScriptSegment, ParseScriptShouldShouldFailWithoutId) {
// 	vector<ValidationError> validationErrors;
// 	JsonLoader jsonLoader;
// 	json json = getMinimalJson();
// 	json["segment-blocks"] = json::array({
// 		{ { "segments", json::array() } },
// 	});

// 	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
// 	ASSERT_EQ(validationErrors.size(), 1);
// 	expectError(validationErrors, ValidationErrorCode::Id_String, "/segment-blocks/0");
// }

// TEST(TimeSeqJsonScriptSegment, ParseScriptShouldAllowEmptySegmentsArrayAndNoRepeat) {
// 	vector<ValidationError> validationErrors;
// 	JsonLoader jsonLoader;
// 	json json = getMinimalJson();
// 	json["segment-blocks"] = json::array({
// 		{ { "id", "segment-block-1" }, { "segments", json::array() } },
// 	});

// 	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
// 	ASSERT_EQ(validationErrors.size(), 0);
// 	ASSERT_EQ(script->segmentBlocks.size(), 1);
// 	EXPECT_EQ(script->segmentBlocks[0].segments.size(), 0);
// 	EXPECT_FALSE(script->segmentBlocks[0].repeat);
// }

// TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailOnNonNumericRepeat) {
// 	vector<ValidationError> validationErrors;
// 	JsonLoader jsonLoader;
// 	json json = getMinimalJson();
// 	json["segment-blocks"] = json::array({
// 		{ { "id", "segment-block-1" }, { "repeat", "not-a-number" }, { "segments", json::array() } },
// 	});

// 	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
// 	ASSERT_EQ(validationErrors.size(), 1);
// 	expectError(validationErrors, ValidationErrorCode::SegmentBlock_RepeatNumber, "/segment-blocks/0");
// }

// TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailOnNegativeRepeat) {
// 	vector<ValidationError> validationErrors;
// 	JsonLoader jsonLoader;
// 	json json = getMinimalJson();
// 	json["segment-blocks"] = json::array({
// 		{ { "id", "segment-block-1" }, { "repeat", -1 }, { "segments", json::array() } },
// 	});

// 	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
// 	ASSERT_EQ(validationErrors.size(), 1);
// 	expectError(validationErrors, ValidationErrorCode::SegmentBlock_RepeatNumber, "/segment-blocks/0");
// }

// TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailOnFloatRepeat) {
// 	vector<ValidationError> validationErrors;
// 	JsonLoader jsonLoader;
// 	json json = getMinimalJson();
// 	json["segment-blocks"] = json::array({
// 		{ { "id", "segment-block-1" }, { "repeat", 1.1 }, { "segments", json::array() } },
// 	});

// 	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
// 	ASSERT_EQ(validationErrors.size(), 1);
// 	expectError(validationErrors, ValidationErrorCode::SegmentBlock_RepeatNumber, "/segment-blocks/0");
// }

// TEST(TimeSeqJsonScriptSegment, ParseScriptShouldSucceedOnPositiveIntegerRepeat) {
// 	vector<ValidationError> validationErrors;
// 	JsonLoader jsonLoader;
// 	json json = getMinimalJson();
// 	json["segment-blocks"] = json::array({
// 		{ { "id", "segment-block-1" }, { "repeat", 420 }, { "segments", json::array() } },
// 	});

// 	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
// 	ASSERT_EQ(validationErrors.size(), 0);
// 	ASSERT_EQ(script->segmentBlocks.size(), 1);
// 	EXPECT_EQ(script->segmentBlocks[0].segments.size(), 0);
// 	ASSERT_TRUE(script->segmentBlocks[0].repeat);
// 	EXPECT_EQ(*script->segmentBlocks[0].repeat.get(), 420);
// }

// TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailOnNonObjectSegment) {
// 	vector<ValidationError> validationErrors;
// 	JsonLoader jsonLoader;
// 	json json = getMinimalJson();
// 	// Both ref and inline segments should be accepted
// 	json["segment-blocks"] = json::array({
// 		{ { "id", "segment-block-1" }, { "segments", json::array({
// 			{ { "ref", "segment-id-1" } },
// 			"not-an-object",
// 			{ { "ref", "segment-id-2" } }
// 		}) } },
// 	});

// 	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
// 	ASSERT_EQ(validationErrors.size(), 1);
// 	expectError(validationErrors, ValidationErrorCode::SegmentBlock_SegmentObject, "/segment-blocks/0/segments/1");
// }

// TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseSegments) {
// 	vector<ValidationError> validationErrors;
// 	JsonLoader jsonLoader;
// 	json json = getMinimalJson();
// 	// Both ref and inline segments should be accepted
// 	json["segment-blocks"] = json::array({
// 		{ { "id", "segment-block-1" }, { "segments", json::array({
// 			{ { "ref", "segment-id" } },
// 			{ { "duration", { { "samples", 1 } } }, { "actions", json::array() } }
// 		}) } },
// 	});

// 	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
// 	ASSERT_EQ(validationErrors.size(), 0);
// 	ASSERT_EQ(script->segmentBlocks.size(), 1);
// 	ASSERT_EQ(script->segmentBlocks[0].segments.size(), 2);
// 	EXPECT_EQ(script->segmentBlocks[0].segments[0].ref, "segment-id");
// 	EXPECT_EQ(script->segmentBlocks[0].segments[1].ref, "");
// 	EXPECT_TRUE(script->segmentBlocks[0].segments[1].duration.samples);
// }
