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

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNonNumberSamples) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", "1" } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_SamplesNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNegativeSamples) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", -1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
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
	ASSERT_EQ(validationErrors.size(), 1);
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
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_SamplesNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNonNumberMillis) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", "1" } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_MillisNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNegativeMillis) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", -1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
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
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_MillisNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNonNumberBars) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", {  { "beats", 1 }, { "bars", "1" } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_BarsNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNegativeBars) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 1 }, { "bars", -1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_BarsNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithZeroBars) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 1 }, { "bars", 0 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_BarsNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithFloatBars) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 1 }, { "bars", 1.1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_BarsNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNonNumberBeats) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", "1" } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_BeatsNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNegativeBeats) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", -1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_BeatsNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithZeroBeatsAndNoBars) {
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

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNonNumberHz) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "hz", "1" } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_HzNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithNegativeHz) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "hz", -1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_HzNumber, "/segments/0/duration");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithDurationWithZeroHz) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "hz", 0 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Duration_HzNumber, "/segments/0/duration");
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

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseDurationWithIntegerMillis) {
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

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseDurationWithFloatMillis) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", 42.69f } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	ASSERT_TRUE(script->segments[0].duration.millis);
	EXPECT_EQ(*script->segments[0].duration.millis.get(), 42.69f);
	EXPECT_FALSE(script->segments[0].duration.bars);
	EXPECT_FALSE(script->segments[0].duration.beats);
	EXPECT_FALSE(script->segments[0].duration.hz);
	EXPECT_FALSE(script->segments[0].duration.samples);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseBarsWithNonZeroBeats) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "bars", 4 }, { "beats", 2 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	ASSERT_TRUE(script->segments[0].duration.bars);
	ASSERT_TRUE(script->segments[0].duration.beats);
	EXPECT_EQ(*script->segments[0].duration.bars.get(), 4);
	EXPECT_EQ(*script->segments[0].duration.beats.get(), 2);
	EXPECT_FALSE(script->segments[0].duration.millis);
	EXPECT_FALSE(script->segments[0].duration.hz);
	EXPECT_FALSE(script->segments[0].duration.samples);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseBarsWithZeroBeats) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "bars", 4 }, { "beats", 0 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	ASSERT_TRUE(script->segments[0].duration.bars);
	ASSERT_TRUE(script->segments[0].duration.beats);
	EXPECT_EQ(*script->segments[0].duration.bars.get(), 4);
	EXPECT_EQ(*script->segments[0].duration.beats.get(), 0);
	EXPECT_FALSE(script->segments[0].duration.millis);
	EXPECT_FALSE(script->segments[0].duration.hz);
	EXPECT_FALSE(script->segments[0].duration.samples);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseFloatBeats) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 2.5f } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	ASSERT_TRUE(script->segments[0].duration.beats);
	EXPECT_EQ(*script->segments[0].duration.beats.get(), 2.5f);
	EXPECT_FALSE(script->segments[0].duration.bars);
	EXPECT_FALSE(script->segments[0].duration.millis);
	EXPECT_FALSE(script->segments[0].duration.hz);
	EXPECT_FALSE(script->segments[0].duration.samples);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseIntegerHz) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "hz", 420 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	ASSERT_TRUE(script->segments[0].duration.hz);
	EXPECT_EQ(*script->segments[0].duration.hz.get(), 420);
	EXPECT_FALSE(script->segments[0].duration.bars);
	EXPECT_FALSE(script->segments[0].duration.millis);
	EXPECT_FALSE(script->segments[0].duration.beats);
	EXPECT_FALSE(script->segments[0].duration.samples);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseFloatHz) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "hz", 4.2f } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	ASSERT_TRUE(script->segments[0].duration.hz);
	EXPECT_EQ(*script->segments[0].duration.hz.get(), 4.2f);
	EXPECT_FALSE(script->segments[0].duration.bars);
	EXPECT_FALSE(script->segments[0].duration.millis);
	EXPECT_FALSE(script->segments[0].duration.beats);
	EXPECT_FALSE(script->segments[0].duration.samples);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldSucceedWithoutActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	EXPECT_EQ(script->segments[0].actions.size(), 0);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithNonArrayActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", "not-an-array" } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Segment_ActionsArray, "/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailWithMixOfActionsAndNonObjectActionEntries) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
			{ { "ref", "action-1" } },
			"not-an-action",
			{ { "ref", "action-2" } },
		}) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Segment_ActionObject, "/segments/0/actions/1");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseMultipleActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
			{ { "ref", "action-1" } },
			{ { "ref", "action-2" } },
			{ { "ref", "action-3" } },
		}) } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	ASSERT_EQ(script->segments[0].actions.size(), 3);
	EXPECT_EQ(script->segments[0].actions[0].ref, "action-1");
	EXPECT_EQ(script->segments[0].actions[1].ref, "action-2");
	EXPECT_EQ(script->segments[0].actions[2].ref, "action-3");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailOnNonBooleanDisableUi) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "disable-ui", "not-a-boolean" } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Segment_DisableUiBoolean, "/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseWithoutDisableUi) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	EXPECT_FALSE(script->segments[0].disableUi);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseFalseDisableUi) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "disable-ui", false } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	EXPECT_FALSE(script->segments[0].disableUi);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseTrueDisableUi) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "disable-ui", true } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	EXPECT_TRUE(script->segments[0].disableUi);
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailOnSegmentBlockCombinedWithDuration) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-block-1" }, { "segment-block", "segment-block-ref" }, { "duration", { { "samples", 1 } } } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Segment_BlockOrSegment, "/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailOnSegmentBlockCombinedWithActions) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-block-1" }, { "segment-block", "segment-block-ref" }, { "actions", json::array() } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Segment_BlockOrSegment, "/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailOnSegmentBlockCombinedWithDisableUi) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-block-1" }, { "segment-block", "segment-block-ref" }, { "disable-ui", false } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Segment_BlockOrSegment, "/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailOnNonStringSegmentBlock) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-block-1" }, { "segment-block", json::object() } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Segment_SegmentBlockString, "/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldFailOnEmptySegmentBlock) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-block-1" }, { "segment-block", "" } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1);
	expectError(validationErrors, ValidationErrorCode::Segment_SegmentBlockLength, "/segments/0");
}

TEST(TimeSeqJsonScriptSegment, ParseScriptShouldParseSegmentBlock) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["segments"] = json::array({
		{ { "id", "segment-block-1" }, { "segment-block", "segment-block-ref" } }
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, true, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0);
	ASSERT_EQ(script->segments.size(), 1);
	EXPECT_EQ(script->segments[0].id, "segment-block-1");
	EXPECT_TRUE(script->segments[0].segmentBlock);
	EXPECT_EQ(script->segments[0].segmentBlock.get()->ref, "segment-block-ref");
}
