#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithNonBooleanAutoStart) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "auto-start", "not-a-boolean" }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_AutoStartBoolean, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldDefaultAutoStartToTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_TRUE(script->timelines[0].lanes[0].autoStart);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParseAutoStartTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "auto-start", true }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_TRUE(script->timelines[0].lanes[0].autoStart);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParseAutoStartFalse) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "auto-start", false }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_FALSE(script->timelines[0].lanes[0].autoStart);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithNonBooleanLoop) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "loop", "not-a-boolean" }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_LoopBoolean, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldDefaultLoopToTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_FALSE(script->timelines[0].lanes[0].loop);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParseLoopTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "loop", true }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_TRUE(script->timelines[0].lanes[0].loop);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParseLoopFalse) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "loop", false }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_FALSE(script->timelines[0].lanes[0].loop);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithNonNumberRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "repeat", "not-a-number" }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_RepeatNumber, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithNegativeRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "repeat", -1 }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_RepeatNumber, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithNonIntegerRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "repeat", 1.1 }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_RepeatNumber, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldDefaultRepeatToZero) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes[0].repeat, 0);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParseZeroRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "repeat", 0 }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes[0].repeat, 0);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParsePositiveRepeat) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "repeat", 69 }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes[0].repeat, 69);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldDefaultStartTriggerToEmpty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes[0].startTrigger, "");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithNonStringStartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "start-trigger", 1.1 }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_StartTriggerString, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithEmptyStartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "start-trigger", "" }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_StartTriggerLength, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParseStartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "start-trigger", "a-start-trigger" }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes[0].startTrigger, "a-start-trigger");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldDefaultRestartTriggerToEmpty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes[0].restartTrigger, "");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithNonStringRestartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "restart-trigger", 1.1 }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_RestartTriggerString, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithEmptyRestartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "restart-trigger", "" }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_RestartTriggerLength, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParseRestartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "restart-trigger", "a-restart-trigger" }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes[0].restartTrigger, "a-restart-trigger");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldDefaultStopTriggerToEmpty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes[0].stopTrigger, "");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithNonStringStopTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "stop-trigger", 1.1 }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_StopTriggerString, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithEmptyStopTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "stop-trigger", "" }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_StopTriggerLength, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParseStopTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "stop-trigger", "a-stop-trigger" }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes[0].stopTrigger, "a-stop-trigger");
}




TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailWithNonBooleanDisableUi) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "disable-ui", "not-a-boolean" }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_DisableUiBoolean, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldDefaultDisableUiToTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_FALSE(script->timelines[0].lanes[0].disableUi);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParseDisableUiTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "disable-ui", true }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_TRUE(script->timelines[0].lanes[0].disableUi);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParseDisableUiFalse) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "disable-ui", false }, { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_FALSE(script->timelines[0].lanes[0].disableUi);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailOnMissingSegments) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({})
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_SegmentsMissing, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailOnNonArraySegments) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "segments", "not-an-array" } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_SegmentsMissing, "/timelines/0/lanes/0");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldAcceptEmptySegments) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "segments", json::array() } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	EXPECT_EQ(script->timelines[0].lanes[0].segments.size(), 0u);
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldFailOnNonObjectSegment) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "segments", json::array({
					{ { "ref", "segment-1" } },
					"not-an-object",
					{ { "ref", "segment-2" } }
				}) } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Lane_SegmentObject, "/timelines/0/lanes/0/segments/1");
}

TEST(TimeSeqJsonScriptLane, ParseScriptShouldParseSegments) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array({
				json::object({ { "segments", json::array({
					{ { "ref", "segment-1" } },
					{ { "ref", "segment-2" } },
					{ { "ref", "segment-3" } }
				}) } })
			}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 1u);
	ASSERT_EQ(script->timelines[0].lanes[0].segments.size(), 3u);
	EXPECT_EQ(script->timelines[0].lanes[0].segments[0].ref, "segment-1");
	EXPECT_EQ(script->timelines[0].lanes[0].segments[1].ref, "segment-2");
	EXPECT_EQ(script->timelines[0].lanes[0].segments[2].ref, "segment-3");
}
