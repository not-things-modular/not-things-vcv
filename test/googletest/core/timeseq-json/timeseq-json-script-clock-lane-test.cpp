#include "timeseq-json-shared.hpp"

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailWithNonBooleanAutoStart) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "auto-start": "not-a-boolean", "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u) << printValidationErrors(validationErrors);
	expectError(validationErrors, ValidationErrorCode::ClockLane_AutoStartBoolean, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldDefaultAutoStartToTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_TRUE(script->clocks[0].lanes[0].autoStart);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldParseAutoStartTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "auto-start": true, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_TRUE(script->clocks[0].lanes[0].autoStart);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldParseAutoStartFalse) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "auto-start": false, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_FALSE(script->clocks[0].lanes[0].autoStart);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldDefaultStartTriggerToEmpty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes[0].startTrigger, "");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailWithNonStringStartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "start-trigger": 1.1, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_StartTriggerString, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailWithEmptyStartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "start-trigger": "", "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_StartTriggerLength, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldParseStartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "start-trigger": "a-start-trigger", "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes[0].startTrigger, "a-start-trigger");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldDefaultRestartTriggerToEmpty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes[0].restartTrigger, "");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailWithNonStringRestartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "restart-trigger": 1.1, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_RestartTriggerString, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailWithEmptyRestartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "restart-trigger": "", "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_RestartTriggerLength, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldParseRestartTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "restart-trigger": "a-restart-trigger", "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes[0].restartTrigger, "a-restart-trigger");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldDefaultStopTriggerToEmpty) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes[0].stopTrigger, "");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailWithNonStringStopTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "stop-trigger": 1.1, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_StopTriggerString, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailWithEmptyStopTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "stop-trigger": "", "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_StopTriggerLength, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldParseStopTrigger) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "stop-trigger": "a-stop-trigger", "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes[0].stopTrigger, "a-stop-trigger");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailWithNonBooleanDisableUi) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "disable-ui": "not-a-boolean", "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_DisableUiBoolean, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldDefaultDisableUiToFalse) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_FALSE(script->clocks[0].lanes[0].disableUi);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldParseDisableUiTrue) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "disable-ui": true, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_TRUE(script->clocks[0].lanes[0].disableUi);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldParseDisableUiFalse) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "disable-ui": false, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_FALSE(script->clocks[0].lanes[0].disableUi);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailWithNonNumericGateHighRatio) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "gate-high-ratio": "not-a-number", "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_GateHighRatioFloat, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldHaveEmtpyGateHighRatioIfNotSet) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_FALSE(script->clocks[0].lanes[0].gateHighRatio);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldParseValidFloatGateHighRatio) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "gate-high-ratio": 0.69, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_TRUE(script->clocks[0].lanes[0].gateHighRatio);
	ASSERT_EQ(*script->clocks[0].lanes[0].gateHighRatio, .69f);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldParseValidIntegerGateHighRatio) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "gate-high-ratio": 1, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_TRUE(script->clocks[0].lanes[0].gateHighRatio);
	ASSERT_EQ(*script->clocks[0].lanes[0].gateHighRatio, 1.f);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldAcceptZeroGateHighRatio) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "gate-high-ratio": 0, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_TRUE(script->clocks[0].lanes[0].gateHighRatio);
	ASSERT_EQ(*script->clocks[0].lanes[0].gateHighRatio, .0f);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailWithNegativeGateHighRatio) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "gate-high-ratio": -0.001, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_GateHighRatioRange, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailWitGateHighRatioAboveOne) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "gate-high-ratio": 1.001, "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_GateHighRatioRange, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailOnMissingOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [] }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_OutputObject, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailOnInvalidOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [], "output": "yes" }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_OutputObject, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldAcceptShortFormatOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [], "output": 2 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes[0].output.index, 2);
	ASSERT_FALSE(script->clocks[0].lanes[0].output.channel);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldAcceptFullFormatOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [], "output": { "index": 3, "channel": 5 } }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes[0].output.index, 3);
	ASSERT_TRUE(script->clocks[0].lanes[0].output.channel);
	ASSERT_EQ(*script->clocks[0].lanes[0].output.channel, 5);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldAcceptRefOutput) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [], "output": { "ref": "the-output" } }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes[0].output.ref, "the-output");
}





TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailOnMissingDurations) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_DurationsMissing, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailOnNonArrayDurations) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": "not-an-array", "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_DurationsMissing, "/clocks/0/lanes/0");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldAcceptEmptyDurations) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	EXPECT_EQ(script->clocks[0].lanes[0].durations.size(), 0u);
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldFailOnNonObjectDuration) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [
						{ "samples": 1 },
						 "not-an-object",
						{ "samples": 3 }
					], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClockLane_DurationObject, "/clocks/0/lanes/0/durations/1");
}

TEST(TimeSeqJsonScriptClockLane, ParseScriptShouldParseDuration) {
	vector<ValidationError> validationErrors;
	JsonLoader jsonLoader;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [
						{ "samples": 1 },
						{ "samples": 2 },
						{ "samples": 3 }
					], "output": 1 }
				] }
			]
        })"_json;

	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script->clocks.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes.size(), 1u);
	ASSERT_EQ(script->clocks[0].lanes[0].durations.size(), 3u);
	EXPECT_EQ(*script->clocks[0].lanes[0].durations[0].samples, 1u);
	EXPECT_EQ(*script->clocks[0].lanes[0].durations[1].samples, 2u);
	EXPECT_EQ(*script->clocks[0].lanes[0].durations[2].samples, 3u);
}

TEST(TimeSeqJsonScriptClockLane, ParseClockLaneWithUnknownPropertyShouldFail) {
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

TEST(TimeSeqJsonScriptClockLane, ParseClockLaneWithUnknownPropertiesShouldFail) {
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

TEST(TimeSeqJsonScriptClockLane, ParseLaneShouldAllowUnknownPropertyWithXPrefix) {
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
