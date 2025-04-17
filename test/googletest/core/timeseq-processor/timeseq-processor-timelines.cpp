#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorTimelines, ScriptWithEmptyTimelinesShouldSucceed) {
	ProcessorLoader processorLoader(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	EXPECT_EQ(script.second->m_timelines.size(), 0u);
	
	// Calling process should not give an error
	script.second->process();
}

TEST(TimeSeqProcessorTimelines, ScriptWithSingleTimelineWithoutLanesShouldSucceed) {
	MockTriggerHandler mockTriggerHandler;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array() } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 0u);

	// Calling process should not give an error
	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	script.second->process();
}

TEST(TimeSeqProcessorTimelines, ScriptWithSingleTimelineWithLanesShouldSucceed) {
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" }} }) } },
			{ { "segments", json::array({ { { "ref", "segment-2" }} }) } },
			{ { "segments", json::array({ { { "ref", "segment-3" }} }) } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 1 } } } },
		{ { "id", "segment-2" }, { "duration", { { "samples", 2 } } } },
		{ { "id", "segment-3" }, { "duration", { { "samples", 3 } } } }
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments[0]->m_duration->m_duration, 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[1]->m_segments.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[1]->m_segments[0]->m_duration->m_duration, 2u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[2]->m_segments.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[2]->m_segments[0]->m_duration->m_duration, 3u);
}

TEST(TimeSeqProcessorTimelines, ScriptWithSingleTimelineWithLanesAndNoLoopLockShouldLoopLanesSeparately) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" }} }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{
			{ "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "start" }, { "trigger", "trigger-1" } }
			})}
		},
		{
			{ "id", "segment-2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "start" }, { "trigger", "trigger-2" } }
			})}
		},
		{
			{ "id", "segment-3" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "start" }, { "trigger", "trigger-3" } }
			})}
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	// The first process should trigger the start actions of the segments all three lanes
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// The second process should trigger the only the first lane action:
	// the second lane will move to its second (and thus last) sample, the third lane will move to its second sample
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// The third process should trigger the first lane and the second lane (which also looped now)
	// The third lane will move to its third (and last) sample
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// The fourth process should trigger the first lane and the third lane (which also looped now)
	// The second lane will move to its second sample (for the second time)
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
}
