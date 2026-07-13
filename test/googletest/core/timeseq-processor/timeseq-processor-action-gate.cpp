#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorGateAction, GateActionWithRefToUnknownOutputShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "timing", "gate" },
					{ "output", { { "ref", "unknown-ref" } } }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/output");
}

TEST(TimeSeqProcessorGateAction, GlideActionWithInvalidOutputShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "timing", "gate" },
					{ "output", { { "index", "1" } } }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Output_IndexNumber, "/timelines/0/lanes/0/segments/0/actions/0/output");
}

TEST(TimeSeqProcessorGateAction, GateActionWithNoOutputShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "timing", "gate" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_GateOutput, "/timelines/0/lanes/0/segments/0/actions/0");
}

TEST(TimeSeqProcessorGateAction, GateActionWithoutGateHighRatioOrGateHighDurationShouldRemainHighForHalfOfDuration) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{ { "duration", { { "samples", 10 } } }, { "actions", json::array({
					{ { "timing", "gate" }, { "output", { { "index", 1 } } } }
				}) } },
				{ { "duration", { { "samples", 20 } } }, { "actions", json::array({
					{ { "timing", "gate" }, { "output", { { "index", 2 } } } }
				}) } },
				{ { "duration", { { "samples", 11 } } }, { "actions", json::array({
					{ { "timing", "gate" }, { "output", { { "index", 3 } } } }
				}) } }
			}) } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

 	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// The first segment should go high on the first process
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for the next 4 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(4).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low for the second half of the first segment
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// The gate should remain low for the next 4 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(4).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The second segment should go high on its first process
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 10.f)).Times(1);
		// The gate should remain high for the next 9 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low for the second half of the second segment
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 0.f)).Times(1);
		// The gate should remain low for the next 9 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The third segment should go high on its first process
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, 10.f)).Times(1);
		// The gate should remain high for the next 5 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(5).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low for the second half of the third segment
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, 0.f)).Times(1);
		// The gate should remain low for the next 4 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(4).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	}

	for (int i = 0; i < 41; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGateAction, GateActionWithGateHighRatioShouldRemainHighForRatioOfDuration) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{ { "duration", { { "samples", 10 } } }, { "actions", json::array({
					{ { "timing", "gate" }, { "output", { { "index", 1 } } }, { "gate-high-ratio", 0.4f } }
				}) } },
				{ { "duration", { { "samples", 20 } } }, { "actions", json::array({
					{ { "timing", "gate" }, { "output", { { "index", 2 } } }, { "gate-high-ratio", 0.7f } }
				}) } },
				{ { "duration", { { "samples", 11 } } }, { "actions", json::array({ // Uneven sample counts should make the high gate last one sample longer
					{ { "timing", "gate" }, { "output", { { "index", 3 } } }, { "gate-high-ratio", 0.2f } }
				}) } }
			}) } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

 	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// The first segment should go high on the first process
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for the next 3 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(3).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low for the second half of the first segment
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// The gate should remain low for the next 5 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(5).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The second segment should go high on its first process
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 10.f)).Times(1);
		// The gate should remain high for the next 9 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(13).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low for the second half of the second segment
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 0.f)).Times(1);
		// The gate should remain low for the next 9 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(5).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The third segment should go high on its first process
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, 10.f)).Times(1);
		// The gate should remain high for the next 5 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low for the second half of the third segment
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, 0.f)).Times(1);
		// The gate should remain low for the next 4 calls
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(7).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	}

	for (int i = 0; i < 41; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGateAction, GateActionWithGateHighRatioZeroShouldBeHighForOneSample) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{ { "duration", { { "samples", 10 } } }, { "actions", json::array({
					{ { "timing", "gate" }, { "output", { { "index", 1 } } }, { "gate-high-ratio", 0.f } }
				}) } }
			}) } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

 	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// The first segment should go high on the first process
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should go low for for the second process
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// The gate should remain low for the rest of the duration
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(8).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	}

	for (int i = 0; i < 10; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGateAction, GateActionWithGateHighRatioOneShouldStayHighUntilLastSample) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{ { "duration", { { "samples", 10 } } }, { "actions", json::array({
					{ { "timing", "gate" }, { "output", { { "index", 1 } } }, { "gate-high-ratio", 1.f } }
				}) } }
			}) } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

 	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// The first segment should go high on the first process
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high until last sample
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low for the last sample
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
	}

	for (int i = 0; i < 10; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGateAction, GateActionWithGateHighDurationShouldRemainHighForDuration) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_3_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{ { "duration", { { "samples", 100 } } }, { "actions", json::array({
					{ { "timing", "gate" }, { "output", { { "index", 1 } } }, { "gate-high-duration", { { "samples", 30 } } } }
				}) } },
				{ { "duration", { { "samples", 50 } } }, { "actions", json::array({
					{ { "timing", "gate" }, { "output", { { "index", 1 } } }, { "gate-high-duration", { { "samples", 21 } } } }
				}) } }
			}) } }
		}) } }
	});

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(10));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

 	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// The first segment should go high on the first process
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 29 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(29).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(69).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The second segment should go high on the first process
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 20 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(20).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(28).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	}

	for (int i = 0; i < 150; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGateAction, GateActionWithGateHighDurationInSamplesShouldKeepGates1MsFromEdges) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "timelines": [
				{
					"time-scale": { "sample-rate": 10000 },
					"lanes": [
						{
							"segments": [
								{
									"duration": { "samples": 100 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": 3 }
									} ]
								},
								{
									"duration": { "samples": 100 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": 11 }
									} ]
								},
								{
									"duration": { "samples": 100 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": 89 }
									} ]
								},
								{
									"duration": { "samples": 100 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": 96 }
									} ]
								},
								{
									"duration": { "samples": 8 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": 22 }
									} ]
								}
							]
						}
					]
				}
			]
        })"_json;

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(10000)); // 10000Hz means 1 ms = 10 samples

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

 	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// The first segment should have the gate high duration pushed to 1ms (10 samples) iso of the configured 3 samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 9 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(89).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The second segment should have not have the gate high duration changed because it is away from the 1ms boundary
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 10 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(10).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(88).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The third segment should have not have the gate high duration changed because it is still inside the 1ms boundaries
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 88 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(88).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(10).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The fourth segment should be pushed away from the end of the gate, since it is inside the final 1ms boundary
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 89 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(89).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The final segment is to small to keep the change 1ms was from both the start and the end of the gate signal
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for three more sample (which reached the middle of the gate signal)
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(3).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the final three samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(3).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	}

	for (int i = 0; i < 408; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGateAction, GateActionWithGateHighDurationInSamplesShouldKeepGates1MsFromEdgesWithVariableDurations) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	testing::NiceMock<MockVariableHandler> mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "timelines": [
				{
					"time-scale": { "sample-rate": 10000 },
					"lanes": [
						{
							"segments": [
								{
									"duration": { "samples": 100 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": { "variable": "the-duration-variable" } }
									} ]
								},
								{
									"duration": { "samples": 100 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": { "variable": "the-duration-variable" } }
									} ]
								},
								{
									"duration": { "samples": 100 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": { "variable": "the-duration-variable" } }
									} ]
								},
								{
									"duration": { "samples": 100 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": { "variable": "the-duration-variable" } }
									} ]
								}
							]
						}
					]
				}
			]
        })"_json;

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(10000)); // 10000Hz means 1 ms = 10 samples

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

 	string variableName = "the-duration-variable";
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// The first segment should have the gate high duration pushed to 1ms (10 samples) iso of the configured 3 samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, getVariable(variableName)).WillOnce(testing::Return(3));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 9 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(89).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The second segment should have not have the gate high duration changed because it is away from the 1ms boundary
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, getVariable(variableName)).WillOnce(testing::Return(11));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 10 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(10).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(88).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The third segment should have not have the gate high duration changed because it is still inside the 1ms boundaries
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, getVariable(variableName)).WillOnce(testing::Return(89));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 88 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(88).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(10).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The fourth segment should be pushed away from the end of the gate, since it is inside the final 1ms boundary
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, getVariable(variableName)).WillOnce(testing::Return(96));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 89 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(89).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	}

	for (int i = 0; i < 400; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGateAction, GateActionWithGateHighDurationInSamplesShouldKeepGates1MsFromEdgesWithSampleRateRecalculation) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
    json json = R"({
            "type": "not-things_timeseq_script",
            "version": ")" SCRIPT_VERSION_1_3_0 R"(",
            "timelines": [
				{
					"time-scale": { "sample-rate": 100000 },
					"lanes": [
						{
							"segments": [
								{
									"duration": { "samples": 1000 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": 30 }
									} ]
								},
								{
									"duration": { "samples": 1000 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": 110 }
									} ]
								},
								{
									"duration": { "samples": 1000 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": 890 }
									} ]
								},
								{
									"duration": { "samples": 1000 },
									"actions": [ {
										"timing": "gate",
										"output": { "index": 1 },
										"gate-high-duration": { "samples": 960 }
									} ]
								}
							]
						}
					]
				}
			]
        })"_json;

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(10000)); // 10000Hz means 1 ms = 10 samples

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

 	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// The first segment should have the gate high duration pushed to 1ms (10 samples) iso of the configured 3 samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 9 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(89).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The second segment should have not have the gate high duration changed because it is away from the 1ms boundary
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 10 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(10).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(88).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The third segment should have not have the gate high duration changed because it is still inside the 1ms boundaries
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 88 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(88).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(10).WillRepeatedly(testing::ReturnRef(emptyTriggers));

		// The fourth segment should be pushed away from the end of the gate, since it is inside the final 1ms boundary
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
		// The gate should remain high for 89 more samples
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(89).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		// The gate should go low
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		// And remain low for the remainder
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	}

	for (int i = 0; i < 400; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGateAction, GateActionWithConditionShouldRunBasedOnIfCondition) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 10 } } }, { "actions", json::array({
					{
						{ "timing", "gate" }, { "output", { { "index", 1 } } },
						{ "if", { { "eq", json::array({
							{ { "variable", "if-variable" } },
							{ { "voltage", 1.f } }
						}) } } }
					}
				}) } }
			}) } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

 	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		std::string variableName = "if-variable";

		for (int i = 0; i < 3; i++) {
			if (i != 1) {
				// The first segment should go high on the first process
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(1.f)); // The variable should only be requested once at the start of the segment.
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 10.f)).Times(1);
				// The gate should remain high for the next 4 calls
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(4).WillRepeatedly(testing::ReturnRef(emptyTriggers));
				// The gate should go low for the second half of the first segment
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
				// The gate should remain low for the next 4 calls
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(4).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			} else {
				// In the second loop, the condition will be false, so the gate should not activate
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(1.1f));
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			}
		}
	}

	for (int i = 0; i < 30; i++) {
		script.second->process();
	}
}
