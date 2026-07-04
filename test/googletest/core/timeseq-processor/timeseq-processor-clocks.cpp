#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorClocks, ScriptWithNoClocksShouldResultInNoTimeline) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": []
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 0u);
}

TEST(TimeSeqProcessorClocks, ScriptWithEmptyClockShouldResultInEmptyTimeline) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [] }
			]
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 1u);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes.size(), 0u);
	EXPECT_FALSE(processor->m_timelines[0]->m_loopLock);
}

TEST(TimeSeqProcessorClocks, ScriptWithMultipleClocksShouldResultInMultipleTimelines) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [ { "durations": [ { "samples": 1 } ], "output": 2, "start-trigger": "start-1" } ] },
				{ "lanes": [ { "durations": [ { "samples": 3 } ], "output": 4, "start-trigger": "start-2" } ] }
			]
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 2u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_startTrigger, "start-1");
	ASSERT_EQ(processor->m_timelines[1]->m_lanes.size(), 1u);
	EXPECT_EQ(processor->m_timelines[1]->m_lanes[0]->m_startTrigger, "start-2");
}

TEST(TimeSeqProcessorClocks, ScripShouldCombineClocksAndTimelines) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [ { "durations": [ { "samples": 1 } ], "output": 2, "start-trigger": "start-1" } ] },
				{ "lanes": [ { "durations": [ { "samples": 3 } ], "output": 4, "start-trigger": "start-2" } ] }
			],
			"timelines": [
				{ "lanes": [ { "segments": [], "start-trigger": "start-3" } ] },
				{ "lanes": [ { "segments": [], "start-trigger": "start-4" } ] }
			]
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 4u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_startTrigger, "start-1");
	ASSERT_EQ(processor->m_timelines[1]->m_lanes.size(), 1u);
	EXPECT_EQ(processor->m_timelines[1]->m_lanes[0]->m_startTrigger, "start-2");
	ASSERT_EQ(processor->m_timelines[2]->m_lanes.size(), 1u);
	EXPECT_EQ(processor->m_timelines[2]->m_lanes[0]->m_startTrigger, "start-3");
	ASSERT_EQ(processor->m_timelines[3]->m_lanes.size(), 1u);
	EXPECT_EQ(processor->m_timelines[3]->m_lanes[0]->m_startTrigger, "start-4");
}

TEST(TimeSeqProcessorClocks, ScriptShouldParseClockLaneTriggers) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [ { "durations": [ { "samples": 1 } ], "output": 2, "start-trigger": "start-1", "stop-trigger": "stop-1", "restart-trigger": "restart-1" } ] },
				{ "lanes": [ { "durations": [ { "samples": 3 } ], "output": 4, "start-trigger": "start-2", "stop-trigger": "stop-2", "restart-trigger": "restart-2" } ] }
			]
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 2u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_startTrigger, "start-1");
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_stopTrigger, "stop-1");
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_restartTrigger, "restart-1");
	ASSERT_EQ(processor->m_timelines[1]->m_lanes.size(), 1u);
	EXPECT_EQ(processor->m_timelines[1]->m_lanes[0]->m_startTrigger, "start-2");
	EXPECT_EQ(processor->m_timelines[1]->m_lanes[0]->m_stopTrigger, "stop-2");
	EXPECT_EQ(processor->m_timelines[1]->m_lanes[0]->m_restartTrigger, "restart-2");
}

TEST(TimeSeqProcessorClocks, ClockTimelineShouldNotLoopLock) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [ { "samples": 1 } ], "output": 2 },
					{ "durations": [ { "samples": 3 } ], "output": 4 }
				] }
			]
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes.size(), 2u);
	ASSERT_FALSE(processor->m_timelines[0]->m_loopLock);
}

TEST(TimeSeqProcessorClocks, ClockLanesShouldHaveCorrectAutoStartAndLoopValues) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [ { "samples": 3 } ], "output": 1 },
					{ "durations": [ { "samples": 3 } ], "output": 2, "auto-start": true },
					{ "durations": [ { "samples": 3 } ], "output": 3, "auto-start": false }
				] }
			]
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes.size(), 3u);

	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_autoStart, true);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_loop, true);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_repeat, 0);

	EXPECT_EQ(processor->m_timelines[0]->m_lanes[1]->m_autoStart, true);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[1]->m_loop, true);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[1]->m_repeat, 0);

	EXPECT_EQ(processor->m_timelines[0]->m_lanes[2]->m_autoStart, false);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[2]->m_loop, true);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[2]->m_repeat, 0);
}

TEST(TimeSeqProcessorClocks, ScriptShouldParseClockLaneOutputs) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [ { "samples": 1 } ], "output": 1 },
					{ "durations": [ { "samples": 2 } ], "output": { "index": 2, "channel": 3 } },
					{ "durations": [ { "samples": 3 } ], "output": { "ref": "the-output" } }
				] }
			],
			"component-pool": {
				"outputs": [
					{
						"id": "the-output",
						"index": 4,
						"channel": 5
					}
				]
			}
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes.size(), 3u);

	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_ongoingActions[0].get())->m_outputPort, 0);
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_ongoingActions[0].get())->m_outputChannel, 0);

	ASSERT_EQ(processor->m_timelines[0]->m_lanes[1]->m_segments.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[1]->m_segments[0]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[1]->m_segments[0]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[1]->m_segments[0]->m_ongoingActions[0].get())->m_outputPort, 1);
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[1]->m_segments[0]->m_ongoingActions[0].get())->m_outputChannel, 2);

	ASSERT_EQ(processor->m_timelines[0]->m_lanes[2]->m_segments.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[2]->m_segments[0]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[2]->m_segments[0]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[2]->m_segments[0]->m_ongoingActions[0].get())->m_outputPort, 3);
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[2]->m_segments[0]->m_ongoingActions[0].get())->m_outputChannel, 4);
}

TEST(TimeSeqProcessorClocks, ScriptShouldFailOnUnknownOutputRef) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [ { "samples": 3 } ], "output": { "ref": "the-output" } }
				] }
			],
			"component-pool": {
				"outputs": [
					{
						"id": "not-the-output",
						"index": 4,
						"channel": 5
					}
				]
			}
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/clocks/0/lanes/0/output");
}

TEST(TimeSeqProcessorClocks, ScriptShouldParseMultipleDurations) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{
						"durations": [
							{ "samples": 1 },
							{ "samples": 2 },
							{ "samples": 3 }
						],
						"output": { "index": 5, "channel": 3 }
					}
				] }
			],
			"component-pool": {
				"outputs": [
					{
						"id": "the-output",
						"index": 4,
						"channel": 5
					}
				]
			}
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments.size(), 3u);

	EXPECT_NE(nullptr, dynamic_cast<DurationConstantProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_duration.get()));
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_duration->m_duration, 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_ongoingActions[0].get())->m_outputPort, 4);
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_ongoingActions[0].get())->m_outputChannel, 2);

	EXPECT_NE(nullptr, dynamic_cast<DurationConstantProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[1]->m_duration.get()));
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[1]->m_duration->m_duration, 2u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[1]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[1]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[1]->m_ongoingActions[0].get())->m_outputPort, 4);
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[1]->m_ongoingActions[0].get())->m_outputChannel, 2);

	EXPECT_NE(nullptr, dynamic_cast<DurationConstantProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[2]->m_duration.get()));
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[2]->m_duration->m_duration, 3u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[2]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[2]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[2]->m_ongoingActions[0].get())->m_outputPort, 4);
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[2]->m_ongoingActions[0].get())->m_outputChannel, 2);
}

TEST(TimeSeqProcessorClocks, ScriptShouldParseVariableDuration) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{
						"durations": [
							{ "samples": { "variable": "the-duration" } }
						],
						"output": { "index": 5, "channel": 3 }
					}
				] }
			],
			"component-pool": {
				"outputs": [
					{
						"id": "the-output",
						"index": 4,
						"channel": 5
					}
				]
			}
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	// dynamic_cast<VariableValueProcessor*>(dynamic_cast<DurationVariableFactorProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_duration.get())->m_value)

	// We're expecting a variable duration processor
	ASSERT_NE(nullptr, dynamic_cast<DurationVariableFactorProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_duration.get()));
	DurationVariableFactorProcessor* durationProcessor = dynamic_cast<DurationVariableFactorProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_duration.get());
	// That has a value
	ASSERT_TRUE(durationProcessor->m_value);
	// The value should ba a variable value
	ASSERT_NE(nullptr, dynamic_cast<VariableValueProcessor*>(durationProcessor->m_value.get()));
	// That points to the "the-duration" value
	EXPECT_EQ(dynamic_cast<VariableValueProcessor*>(durationProcessor->m_value.get())->m_name, "the-duration");
}

TEST(TimeSeqProcessorClocks, ParseScriptShouldApplyClockDisableUiToLanesAndSegments) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [ { "samples": 3 },{ "samples": 2 }, { "samples": 1 } ], "output": 1 },
					{ "durations": [ { "samples": 3 },{ "samples": 2 }, { "samples": 1 } ], "output": 2, "disable-ui": true },
					{ "durations": [ { "samples": 3 },{ "samples": 2 }, { "samples": 1 } ], "output": 3, "disable-ui": false }
				] }
			]
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes.size(), 3u);

	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_disableUi, false);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments.size(), 3u);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_disableUi, false);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[1]->m_disableUi, false);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[2]->m_disableUi, false);

	EXPECT_EQ(processor->m_timelines[0]->m_lanes[1]->m_disableUi, true);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[1]->m_segments.size(), 3u);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[1]->m_segments[0]->m_disableUi, true);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[1]->m_segments[1]->m_disableUi, true);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[1]->m_segments[2]->m_disableUi, true);

	EXPECT_EQ(processor->m_timelines[0]->m_lanes[2]->m_disableUi, false);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[2]->m_segments.size(), 3u);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[2]->m_segments[0]->m_disableUi, false);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[2]->m_segments[1]->m_disableUi, false);
	EXPECT_EQ(processor->m_timelines[0]->m_lanes[2]->m_segments[2]->m_disableUi, false);
}

TEST(TimeSeqProcessorClocks, ParseScriptShouldApplyGateHighRatioToActions) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "clocks": [
				{ "lanes": [
					{ "durations": [ { "samples": 3 },{ "samples": 2 }, { "samples": 1 } ], "output": 1 },
					{ "durations": [ { "samples": 3 },{ "samples": 2 }, { "samples": 1 } ], "output": 2, "gate-high-ratio": 0.25 },
					{ "durations": [ { "samples": 3 },{ "samples": 2 }, { "samples": 1 } ], "output": 3, "gate-high-ratio": 0.75 }
				] }
			]
        })"_json;

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	shared_ptr<Processor> processor = script.second;
	ASSERT_EQ(processor->m_timelines.size(), 1u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes.size(), 3u);

	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments.size(), 3u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[0]->m_ongoingActions[0].get())->m_gateHighRatio, 0.5);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[1]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[1]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[1]->m_ongoingActions[0].get())->m_gateHighRatio, 0.5);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[0]->m_segments[2]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[2]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[0]->m_segments[2]->m_ongoingActions[0].get())->m_gateHighRatio, 0.5);

	ASSERT_EQ(processor->m_timelines[0]->m_lanes[1]->m_segments.size(), 3u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[1]->m_segments[0]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[1]->m_segments[0]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[1]->m_segments[0]->m_ongoingActions[0].get())->m_gateHighRatio, 0.25);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[1]->m_segments[1]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[1]->m_segments[1]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[1]->m_segments[1]->m_ongoingActions[0].get())->m_gateHighRatio, 0.25);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[1]->m_segments[2]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[1]->m_segments[2]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[1]->m_segments[2]->m_ongoingActions[0].get())->m_gateHighRatio, 0.25);

	ASSERT_EQ(processor->m_timelines[0]->m_lanes[2]->m_segments.size(), 3u);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[2]->m_segments[0]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[2]->m_segments[0]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[2]->m_segments[0]->m_ongoingActions[0].get())->m_gateHighRatio, 0.75);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[2]->m_segments[1]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[2]->m_segments[1]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[2]->m_segments[1]->m_ongoingActions[0].get())->m_gateHighRatio, 0.75);
	ASSERT_EQ(processor->m_timelines[0]->m_lanes[2]->m_segments[2]->m_ongoingActions.size(), 1u);
	EXPECT_NE(nullptr, dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[2]->m_segments[2]->m_ongoingActions[0].get()));
	EXPECT_EQ(dynamic_cast<ActionGateProcessor*>(processor->m_timelines[0]->m_lanes[2]->m_segments[2]->m_ongoingActions[0].get())->m_gateHighRatio, 0.75);
}
