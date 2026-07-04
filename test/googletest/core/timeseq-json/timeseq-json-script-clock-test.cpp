#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptClock, ParseScriptShouldFailWithClockPreVersion130) {
    vector<ValidationError> validationErrors;
    JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_2_0 R"(",
            "clocks": []
        })"_json;

    shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
    ASSERT_EQ(validationErrors.size(), 1u) << validationErrors[0].location;
    expectError(validationErrors, ValidationErrorCode::Feature_Not_In_Version, "/");
}

TEST(TimeSeqJsonScriptClock, ParseScriptShouldFailWithNonArrayClocks) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": { "not": "an array" }
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_ClocksArray, "/");
}

TEST(TimeSeqJsonScriptClock, ParseScriptShouldFailWithMixOfClocksAndNonObjectClockEntries) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [] },
				"not-a-clock"
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Script_ClockObject, "/clocks/1");
}

TEST(TimeSeqJsonScriptClock, ParseScriptShouldParseMultipleClocks) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": 1 } },
				{ "lanes": [], "time-scale": { "sample-rate": 2 } },
				{ "lanes": [], "time-scale": { "sample-rate": 3 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 3u);
	ASSERT_EQ(*script->clocks[0].timeScale.get()->sampleRate.get(), 1);
	ASSERT_EQ(*script->clocks[1].timeScale.get()->sampleRate.get(), 2);
	ASSERT_EQ(*script->clocks[2].timeScale.get()->sampleRate.get(), 3);
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnNonObjectClock) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": "not-an-object" }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Clock_TimeScaleObject, "/clocks/0");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithNoKnownProperties) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": {} }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithBpbButNoBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": 44000, "bpb": 4 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpbRequiresBpm, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithNonNumericSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": "44000" } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_SampleRateNumber, "/clocks/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithNegativeSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": -44000 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_SampleRateNumber, "/clocks/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithZeroSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": 0 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_SampleRateNumber, "/clocks/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithFloatSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": 44000.1 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_SampleRateNumber, "/clocks/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldParseTimeScaleWithSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": 44000 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_TRUE(script->clocks[0].timeScale);
	ASSERT_TRUE(script->clocks[0].timeScale->sampleRate);
	EXPECT_EQ(*script->clocks[0].timeScale->sampleRate.get(), 44000);
	EXPECT_FALSE(script->clocks[0].timeScale->bpm);
	EXPECT_FALSE(script->clocks[0].timeScale->bpb);
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithNonNumericBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "bpm": "4" } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpmNumber, "/clocks/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithNegativeBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "bpm": -4 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpmNumber, "/clocks/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithZeroBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "bpm": 0 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpmNumber, "/clocks/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithFloatBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "bpm": 4.1 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpmNumber, "/clocks/0/time-scale");
	expectError(validationErrors, ValidationErrorCode::TimeScale_Empty, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldParseTimeScaleWithBpm) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "bpm": 4 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_TRUE(script->clocks[0].timeScale);
	ASSERT_TRUE(script->clocks[0].timeScale->bpm);
	EXPECT_EQ(*script->clocks[0].timeScale->bpm.get(), 4);
	EXPECT_FALSE(script->clocks[0].timeScale->sampleRate);
	EXPECT_FALSE(script->clocks[0].timeScale->bpb);
}




TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithNonNumericBpb) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "bpm": 120, "bpb": "4" } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpbNumber, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithNegativeBpb) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "bpm": 120, "bpb": -4 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpbNumber, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithZeroBpb) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "bpm": 120, "bpb": 0 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpbNumber, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnTimeScaleWithFloatBpb) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "bpm": 120, "bpb": 4.0 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::TimeScale_BpbNumber, "/clocks/0/time-scale");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldParseTimeScaleWithBpb) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "bpm": 120, "bpb": 4 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_TRUE(script->clocks[0].timeScale);
	EXPECT_FALSE(script->clocks[0].timeScale->sampleRate);
	ASSERT_TRUE(script->clocks[0].timeScale->bpm);
	EXPECT_EQ(*script->clocks[0].timeScale->bpm.get(), 120);
	ASSERT_TRUE(script->clocks[0].timeScale->bpb);
	EXPECT_EQ(*script->clocks[0].timeScale->bpb.get(), 4);
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldParseTimeScaleWithBpmAndSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": 44100, "bpm": 120 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_TRUE(script->clocks[0].timeScale);
	ASSERT_TRUE(script->clocks[0].timeScale->sampleRate);
	EXPECT_EQ(*script->clocks[0].timeScale->sampleRate.get(), 44100);
	ASSERT_TRUE(script->clocks[0].timeScale->bpm);
	EXPECT_EQ(*script->clocks[0].timeScale->bpm.get(), 120);
	EXPECT_FALSE(script->clocks[0].timeScale->bpb);
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldParseTimeScaleWithBpmBpbAndSampleRate) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": 44100, "bpm": 120, "bpb": 4 } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_TRUE(script->clocks[0].timeScale);
	ASSERT_TRUE(script->clocks[0].timeScale->sampleRate);
	EXPECT_EQ(*script->clocks[0].timeScale->sampleRate.get(), 44100);
	ASSERT_TRUE(script->clocks[0].timeScale->bpm);
	EXPECT_EQ(*script->clocks[0].timeScale->bpm.get(), 120);
	ASSERT_TRUE(script->clocks[0].timeScale->bpb);
	EXPECT_EQ(*script->clocks[0].timeScale->bpb.get(), 4);
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailOnNonObjectLane) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
				 	{ "durations": [], "output": 1 },
					 "not-a-lane",
				 	{ "durations": [], "output": 2 }
 				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u) << printValidationErrors(validationErrors);
	expectError(validationErrors, ValidationErrorCode::Clock_LaneObject, "/clocks/0/lanes/1");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldParseMultipleLanes) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
				 	{ "durations": [], "output": 1, "start-trigger": "sat" },
				 	{ "durations": [], "output": 2, "restart-trigger": "rst" },
				 	{ "durations": [], "output": 3, "stop-trigger": "sot" }
 				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks[0].lanes.size(), 3u);
	ASSERT_EQ(script->clocks[0].lanes[0].output.index, 1);
	ASSERT_EQ(script->clocks[0].lanes[0].startTrigger, "sat");
	ASSERT_EQ(script->clocks[0].lanes[1].output.index, 2);
	ASSERT_EQ(script->clocks[0].lanes[1].restartTrigger, "rst");
	ASSERT_EQ(script->clocks[0].lanes[2].output.index, 3);
	ASSERT_EQ(script->clocks[0].lanes[2].stopTrigger, "sot");
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldFailWithoutLanes) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{}
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Clock_LanesMissing, "/clocks/0");
}

TEST(TimeSeqJsonScriptClock, ParseClockWithUnknownPropertyShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
				 	{ "durations": [], "output": 1, "unknown-prop": "value" }
 				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/clocks/0/lanes/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptClock, ParseClockWithUnknownPropertiesShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
				 	{ "durations": [], "output": 1, "unknown-prop-1": "value", "unknown-prop-2": { "child": "object" } }
 				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/clocks/0/lanes/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldAllowUnknownPropertyWithXPrefix) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
				 	{ "durations": [], "output": 1, "x-unknown-prop-1": "value", "x-unknown-prop-2": { "child": "object" } }
 				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	expectNoErrors(validationErrors);
}



TEST(TimeSeqJsonScriptClock, ParseClockWithUnknownPropertyOnTimeScaleShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": 1, "unknown-prop": "valuse" } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/clocks/0/time-scale");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptClock, ParseClockWithUnknownPropertiesOnTimeScaleShouldFail) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": 1, "unknown-prop-1": "value", "unknown-prop-2": { "child": "object" } } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Unknown_Property, "/clocks/0/time-scale");
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-1'"), std::string::npos) << validationErrors[0].message;
	EXPECT_NE(validationErrors[0].message.find("'unknown-prop-2'"), std::string::npos) << validationErrors[0].message;
}

TEST(TimeSeqJsonScriptClock, ParseClockShouldAllowUnknownPropertyWithXPrefixOnTimeScale) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [], "time-scale": { "sample-rate": 1, "x-unknown-prop-1": "value", "x-unknown-prop-2": { "child": "object" } } }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	expectNoErrors(validationErrors);
}
