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
