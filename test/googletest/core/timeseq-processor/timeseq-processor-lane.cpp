#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorLane, LaneWithNoSegmentsShouldNotFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({/* { { "ref", "segment-1" } } */}) }, { "loop", true } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 0u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);

	for (int i = 0; i < 69; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorLane, LaneWithNoDisableUiShouldTriggerLaneLoop) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({ { { "id", "segment-1" }, { "duration", { { "samples", 1 } } } } }) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);
	EXPECT_FALSE(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->disableUi);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	EXPECT_CALL(mockEventListener, segmentStarted()).Times(2);
	EXPECT_CALL(mockEventListener, laneLooped()).Times(1);

	script.second->process();
	script.second->process();
}

TEST(TimeSeqProcessorLane, LaneWithDisableUiFalseShouldTriggerLaneLoop) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true }, { "disable-ui", false } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({ { { "id", "segment-1" }, { "duration", { { "samples", 1 } } } } }) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);
	EXPECT_FALSE(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->disableUi);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	EXPECT_CALL(mockEventListener, segmentStarted()).Times(2);
	EXPECT_CALL(mockEventListener, laneLooped()).Times(1);

	script.second->process();
	script.second->process();
}

TEST(TimeSeqProcessorLane, LaneWithDisableUiTrueShouldNotTriggerLaneLoop) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true }, { "disable-ui", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({ { { "id", "segment-1" }, { "duration", { { "samples", 1 } } } } }) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);
	EXPECT_TRUE(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->disableUi);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	EXPECT_CALL(mockEventListener, segmentStarted()).Times(2);
	EXPECT_CALL(mockEventListener, laneLooped()).Times(0);

	script.second->process();
	script.second->process();
}

TEST(TimeSeqProcessorLane, LaneWithNoAutoStartShouldAutoStart) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true }, { "start-trigger", "start1" } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	EXPECT_CALL(mockEventListener, segmentStarted()).Times(2);
	EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(2);

	script.second->process();
	script.second->process();
}

TEST(TimeSeqProcessorLane, LaneWithAutoStartTrueShouldAutoStart) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true }, { "auto-start", true}, { "start-trigger", "start1" } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	EXPECT_CALL(mockEventListener, segmentStarted()).Times(2);
	EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(2);

	script.second->process();
	script.second->process();
}

TEST(TimeSeqProcessorLane, LaneWithNoAutoStartFalseShouldNotStartUntilStartTriggerIsReceived) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true }, { "auto-start", false}, { "start-trigger", "start1" } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	EXPECT_CALL(mockEventListener, segmentStarted()).Times(0);
	EXPECT_CALL(mockEventListener, laneLooped()).Times(0);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);

	script.second->process();
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	vector<string> startTrigger = { "start1" };
	vector<string> emptyTrigger = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(2).WillOnce(testing::ReturnRef(startTrigger)).WillOnce(testing::ReturnRef(emptyTrigger));
	EXPECT_CALL(mockEventListener, segmentStarted()).Times(2);
	EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(2);

	script.second->process();
	script.second->process();
}

TEST(TimeSeqProcessorLane, LaneShouldHandleResetInMiddleOfSegment) {
	MockPortHandler mockPortHandler;
	MockEventListener mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "auto-start", true}, { "segments", json::array({ { { "ref", "segment-1" } } }) } },
			{ { "loop", true }, { "auto-start", true}, { "segments", json::array({ { { "ref", "segment-2" } }, { { "ref", "segment-3" } }, { { "ref", "segment-4" } }, { { "ref", "segment-5" } } }) } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 15 } } }, { "actions", json::array({
				{ { "timing", "glide" }, { "start-value", 1 }, { "end-value", 8 }, { "output", 1 } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "glide" }, { "start-value", 1 }, { "end-value", 3 }, { "output", 2 } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "samples", 5 } } }, { "actions", json::array({
				{ { "timing", "glide" }, { "start-value", 1 }, { "end-value", 5 }, { "output", 3 } }
			}) }
		},
		{ { "id", "segment-4" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "glide" }, { "start-value", 1 }, { "end-value", 2 }, { "output", 4 } }
			}) }
		},
		{ { "id", "segment-5" }, { "duration", { { "samples", 4 } } }, { "actions", json::array({
				{ { "timing", "glide" }, { "start-value", 1 }, { "end-value", 4 }, { "output", 5 } }
			}) }
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 2u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[1]->m_segments.size(), 4u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);

	{
		testing::InSequence inSequence;
		float segment1Value = 1.f;

		// First loop for 2 steps, and reset within segment-2
		for (int i = 0; i < 2; i++) {
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}

		// Then loop for 6 steps, and reset within segment-3
		segment1Value = 1.f;
		for (int i = 0; i < 3; i++) {
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}
		for (int i = 0; i < 3; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}

		// Then loop for 9 steps, and reset within segment-4
		segment1Value = 1.f;
		for (int i = 0; i < 3; i++) {
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}
		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}
		for (int i = 0; i < 1; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(3, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}

		// Then loop for 12 steps, and reset within segment-5
		segment1Value = 1.f;
		for (int i = 0; i < 3; i++) {
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}
		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}
		for (int i = 0; i < 2; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(3, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}
		for (int i = 0; i < 2; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(4, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}

		// Finally do a full loop
		segment1Value = 1.f;
		for (int i = 0; i < 3; i++) {
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}
		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}
		for (int i = 0; i < 2; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(3, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}
		for (int i = 0; i < 4; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, segment1Value)).Times(1);
			if (i == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(4, 0, i + 1)).Times(1);
			segment1Value += .5f;
		}
	}

	int loopCount[] = { 2, 6, 9, 12, 14 };
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < loopCount[i]; j++) {
			script.second->process();
		}
		script.second->reset();
	}
}
