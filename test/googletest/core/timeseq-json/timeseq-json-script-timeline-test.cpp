#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptTimeLine, ParseScriptShouldFailWithoutTimelines) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = {
		{ "type", "not-things_timeseq_script" },
		{ "version", SCRIPT_VERSION },
	};

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_TimelinesMissing, "/");
}

TEST(TimeSeqJsonScriptTimeLine, ParseScriptShouldFailWithNonArrayTimelines) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::object({ { "not", "an array " } });

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_TimelinesMissing, "/");
}

TEST(TimeSeqJsonScriptTimeLine, ParseScriptShouldFailWithMixOfTimelinesAndNonObjectTimelineEntries) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() }
		},
		"not-a-timeline"
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_TimelineObject, "/timelines/1");
}

TEST(TimeSeqJsonScriptTimeLine, ParseScriptShouldParseMultipleTimelines) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "sample-rate", 1 } } }
		},
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "sample-rate", 2 } } }
		},
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "sample-rate", 3 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->timelines.size(), 3u);
	ASSERT_EQ(*script->timelines[0].timeScale.get()->sampleRate.get(), 1);
	ASSERT_EQ(*script->timelines[1].timeScale.get()->sampleRate.get(), 2);
	ASSERT_EQ(*script->timelines[2].timeScale.get()->sampleRate.get(), 3);
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnNonObjectTimeScale) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", "not-an-object" }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Timeline_TimeScaleObject, "/timelines/0");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithNoKnownProperties) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", json::object({ { "dummy", "value" }}) }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithBpbButNoBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "sample-rate", 44000 }, { "bpb", 4 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpbRequiresBpm, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithNonNumericSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "sample-rate", "44000" } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_SampleRateNumber, "/timelines/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithNegativeSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "sample-rate", -44000 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_SampleRateNumber, "/timelines/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithZeroSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "sample-rate", 0 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_SampleRateNumber, "/timelines/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithFloatSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "sample-rate", 44000.1 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_SampleRateNumber, "/timelines/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldParseTimeScaleWithSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "sample-rate", 44000 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_TRUE(script->timelines[0].timeScale);
	ASSERT_TRUE(script->timelines[0].timeScale->sampleRate);
	EXPECT_EQ(*script->timelines[0].timeScale->sampleRate.get(), 44000);
	EXPECT_FALSE(script->timelines[0].timeScale->bpm);
	EXPECT_FALSE(script->timelines[0].timeScale->bpb);
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithNonNumericBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "bpm", "4" } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpmNumber, "/timelines/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithNegativeBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "bpm", -4 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpmNumber, "/timelines/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithZeroBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "bpm", 0 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpmNumber, "/timelines/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithFloatBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "bpm", 4.1 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpmNumber, "/timelines/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldParseTimeScaleWithBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "bpm", 4 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_TRUE(script->timelines[0].timeScale);
	ASSERT_TRUE(script->timelines[0].timeScale->bpm);
	EXPECT_EQ(*script->timelines[0].timeScale->bpm.get(), 4);
	EXPECT_FALSE(script->timelines[0].timeScale->sampleRate);
	EXPECT_FALSE(script->timelines[0].timeScale->bpb);
}




TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithNonNumericBpb) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "bpm", 120 }, { "bpb", "4" } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpbNumber, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithNegativeBpb) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "bpm", 120 }, { "bpb", -4 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpbNumber, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithZeroBpb) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "bpm", 120 }, { "bpb", 0 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpbNumber, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnTimeScaleWithFloatBpb) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "bpm", 120 }, { "bpb", 4.0 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpbNumber, "/timelines/0/time-scale");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldParseTimeScaleWithBpb) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "bpm", 120 }, { "bpb", 4 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_TRUE(script->timelines[0].timeScale);
	EXPECT_FALSE(script->timelines[0].timeScale->sampleRate);
	ASSERT_TRUE(script->timelines[0].timeScale->bpm);
	EXPECT_EQ(*script->timelines[0].timeScale->bpm.get(), 120);
	ASSERT_TRUE(script->timelines[0].timeScale->bpb);
	EXPECT_EQ(*script->timelines[0].timeScale->bpb.get(), 4);
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldParseTimeScaleWithBpmAndSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "sample-rate", 44100 }, { "bpm", 120 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_TRUE(script->timelines[0].timeScale);
	ASSERT_TRUE(script->timelines[0].timeScale->sampleRate);
	EXPECT_EQ(*script->timelines[0].timeScale->sampleRate.get(), 44100);
	ASSERT_TRUE(script->timelines[0].timeScale->bpm);
	EXPECT_EQ(*script->timelines[0].timeScale->bpm.get(), 120);
	EXPECT_FALSE(script->timelines[0].timeScale->bpb);
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldParseTimeScaleWithBpmBpbAndSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
			{ "time-scale", { { "sample-rate", 44100 }, { "bpm", 120 }, { "bpb", 4 } } }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_TRUE(script->timelines[0].timeScale);
	ASSERT_TRUE(script->timelines[0].timeScale->sampleRate);
	EXPECT_EQ(*script->timelines[0].timeScale->sampleRate.get(), 44100);
	ASSERT_TRUE(script->timelines[0].timeScale->bpm);
	EXPECT_EQ(*script->timelines[0].timeScale->bpm.get(), 120);
	ASSERT_TRUE(script->timelines[0].timeScale->bpb);
	EXPECT_EQ(*script->timelines[0].timeScale->bpb.get(), 4);
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldDefaultToFalseLoopLock) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "lanes", json::array() },
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_FALSE(script->timelines[0].loopLock);
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnNonBooleanLoopLock) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "loop-lock", "not-a-boolean" },
			{ "lanes", json::array() }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Timeline_LoopLockBoolean, "/timelines/0");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldParseLoopLockTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{ "loop-lock", true },
			{ "lanes", json::array() }
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->timelines.size(), 1u);
	ASSERT_TRUE(script->timelines[0].loopLock);
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailOnNonObjectLane) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{
				"lanes", json::array({
					json::object({ { "segments", json::array() } }),
					"not-a-lane",
					json::object({ { "segments", json::array() } }),
			})
			}
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Timeline_LaneObject, "/timelines/0/lanes/1");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldParseMultipleLanes) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{
			{
				"lanes", json::array({
					json::object({ { "start-trigger", "sat" }, { "segments", json::array() } }),
					json::object({ { "restart-trigger", "rst" }, { "segments", json::array() } }),
					json::object({ { "stop-trigger", "sot" }, { "segments", json::array() } }),
			})
			}
		}
	});

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script->timelines[0].lanes.size(), 3u);
	ASSERT_EQ(script->timelines[0].lanes[0].startTrigger, "sat");
	ASSERT_EQ(script->timelines[0].lanes[1].restartTrigger, "rst");
	ASSERT_EQ(script->timelines[0].lanes[2].stopTrigger, "sot");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldFailWithoutLanes) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
	json["timelines"] = json::array({ json::object() });

	shared_ptr<Script> script = loadScript(jsonLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Timeline_LanesMissing, "/timelines/0");
}

TEST(TimeSeqJsonScriptTimeLine, ParseTimelineShouldParseLoopLockFalse) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
	json json = getMinimalJson();
}