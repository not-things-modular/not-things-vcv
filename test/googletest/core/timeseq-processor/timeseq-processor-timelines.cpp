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
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}
	EXPECT_CALL(mockEventListener, laneLooped()).Times(0);
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The second process should trigger the only the first lane action:
	// the second lane will move to its second (and thus last) sample, the third lane will move to its second sample
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The third process should trigger the first lane and the second lane (which also looped now)
	// The third lane will move to its third (and last) sample
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The fourth process should trigger the first lane and the third lane (which also looped now)
	// The second lane will move to its second sample (for the second time)
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}
	script.second->process();
}

TEST(TimeSeqProcessorTimelines, ScriptWithSingleTimelineWithLanesAndLoopLockFalseShouldLoopLanesSeparately) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "loop-lock", false }, { "lanes", json::array({
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
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

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
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

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
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The fourth process should trigger the first lane and the third lane (which also looped now)
	// The second lane will move to its second sample (for the second time)
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}
	script.second->process();
}

TEST(TimeSeqProcessorTimelines, ScriptWithSingleTimelineWithLanesAndLoopLockTrueShouldLoopLanesSeparately) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "loop-lock", true }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" }} }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{
			{ "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{
			{ "id", "segment-2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			})}
		},
		{
			{ "id", "segment-3" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			})}
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	// The first process should trigger the end action of the first lane
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
	}
	EXPECT_CALL(mockEventListener, laneLooped()).Times(0);
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The second process should trigger to end action of the second lane.
	// The first lane should not have looped yet
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
	}
	EXPECT_CALL(mockEventListener, laneLooped()).Times(0);
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The third process should trigger to end action of the third lane.
	// The other two lanes should not have looped yet
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}
	EXPECT_CALL(mockEventListener, laneLooped()).Times(0);
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The fourth process should have looped all lanes.
	// The first lane should end again, and the other two should have started, but not completed
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// Complete another loop to verify that the actions get re-triggered
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
	}
	EXPECT_CALL(mockEventListener, laneLooped()).Times(0);
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}
	EXPECT_CALL(mockEventListener, laneLooped()).Times(0);
	script.second->process();
}

TEST(TimeSeqProcessorTimelines, ScriptWithSingleTimelineWithLanesAndLoopLockTrueShouldLoopLanesSeparatelyWithNonLoopingLane) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "loop-lock", true }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-4" }} }) }, { "loop", false } },
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{
			{ "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{
			{ "id", "segment-2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			})}
		},
		{
			{ "id", "segment-3" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			})}
		},
		{
			{ "id", "segment-4" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-4" } }
			})}
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 4u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	// The first process should trigger the end action of the first lane
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// The second process should trigger to end action of the second and fourth lane.
	// The first lane should not have looped yet
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-4")).Times(1);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// The third process should trigger to end action of the third lane.
	// The other two lanes should not have looped yet
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-4")).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// The fourth process should have looped all lanes.
	// The first lane should end again, the second and third should have looped, and the fourth should not loop
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-4")).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// Complete another loop to verify that the actions get re-triggered, but the fourth lane should not loop
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-4")).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-4")).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
}

TEST(TimeSeqProcessorTimelines, ScriptWithSingleTimelineWithLanesWithStartAndStopTriggersShouldWork) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "loop-lock", true }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" }} }) }, { "loop", true }, { "auto-start", false}, { "start-trigger", "start-1" }, { "restart-trigger", "restart-1" }, { "stop-trigger", "stop-1" } },
			{ { "segments", json::array({ { { "ref", "segment-2" }} }) }, { "loop", true }, { "auto-start", false}, { "start-trigger", "start-2" }, { "restart-trigger", "restart-2" }, { "stop-trigger", "stop-2" } },
			{ { "segments", json::array({ { { "ref", "segment-3" }} }) }, { "loop", true }, { "auto-start", false}, { "start-trigger", "start-3" }, { "restart-trigger", "restart-3" }, { "stop-trigger", "stop-3" } },
			{ { "segments", json::array({ { { "ref", "segment-4" }} }) }, { "loop", false }, { "auto-start", false}, { "start-trigger", "start-4" }, { "restart-trigger", "restart-4" }, { "stop-trigger", "stop-4" } },
			{ { "segments", json::array({ { { "ref", "segment-5" }} }) }, { "loop", false }, { "auto-start", false}, { "start-trigger", "start-5" }, { "restart-trigger", "restart-5" }, { "stop-trigger", "stop-5" } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{
			{ "id", "segment-1" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{
			{ "id", "segment-2" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			})}
		},
		{
			{ "id", "segment-3" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			})}
		},
		{
			{ "id", "segment-4" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-4" } }
			})}
		},
		{
			{ "id", "segment-5" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-5" } }
			})}
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 5u);

	// // Nothing should happen since no start triggers were activated yet
	// MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	// EXPECT_CALL(mockEventListener, segmentStarted).Times(0);
	// EXPECT_CALL(mockEventListener, triggerTriggered).Times(0);
	// EXPECT_CALL(mockTriggerHandler, setTrigger).Times(0);
	// for (int i = 0; i < 4; i++) {
	// 	script.second->process();
	// }
	// testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	// testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// Trigger the start of the first lane and let complete a full loop
	vector<string> emptyTriggers = {};
	vector<string> triggers1 = { "start-1" };
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers1));
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, laneLooped).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
	}
	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// Stop the first lane again, and check that it doesn't do anything anymore
	triggers1 = { "stop-1" };
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers1));
		EXPECT_CALL(mockEventListener, segmentStarted).Times(0);
		EXPECT_CALL(mockEventListener, laneLooped).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger).Times(0);
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(5).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	}
	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);
}
