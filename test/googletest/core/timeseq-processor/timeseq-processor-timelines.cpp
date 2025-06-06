#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorTimelines, ScriptWithEmptyTimelinesShouldSucceed) {
	ProcessorLoader processorLoader(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
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
	EXPECT_NO_ERRORS(validationErrors);
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
	EXPECT_NO_ERRORS(validationErrors);
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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	// The first process should trigger the start actions of the segments all three lanes
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
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
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
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
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
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
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	// The first process should trigger the start actions of the segments all three lanes
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The second process should trigger the only the first lane action:
	// the second lane will move to its second (and thus last) sample, the third lane will move to its second sample
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The third process should trigger the first lane and the second lane (which also looped now)
	// The third lane will move to its third (and last) sample
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The fourth process should trigger the first lane and the third lane (which also looped now)
	// The second lane will move to its second sample (for the second time)
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
	}
	script.second->process();
}

TEST(TimeSeqProcessorTimelines, ScriptWithSingleTimelineWithLanesAndLoopLockTrueShouldLoopLanesTogether) {
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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	// The first process should trigger the end action of the first lane
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
	}
	EXPECT_CALL(mockEventListener, laneLooped()).Times(0);
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The second process should trigger to end action of the second lane.
	// The first lane should not have looped yet
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
	}
	EXPECT_CALL(mockEventListener, laneLooped()).Times(0);
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// The third process should trigger to end action of the third lane.
	// The other two lanes should not have looped yet
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
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
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockEventListener, laneLooped()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// Complete another loop to verify that the actions get re-triggered
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
	}
	EXPECT_CALL(mockEventListener, laneLooped()).Times(0);
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 4u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	// The first process should trigger the end action of the first lane
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// The second process should trigger to end action of the second and fourth lane.
	// The first lane should not have looped yet
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger4Name)).Times(1);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// The third process should trigger to end action of the third lane.
	// The other two lanes should not have looped yet
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger4Name)).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// The fourth process should have looped all lanes.
	// The first lane should end again, the second and third should have looped, and the fourth should not loop
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger4Name)).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// Complete another loop to verify that the actions get re-triggered, but the fourth lane should not loop
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger4Name)).Times(0);
	}
	script.second->process();
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger4Name)).Times(0);
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
		{ { "lanes", json::array({
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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 5u);

	// Nothing should happen since no start triggers were activated yet
	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	EXPECT_CALL(mockEventListener, segmentStarted).Times(0);
	EXPECT_CALL(mockEventListener, triggerTriggered).Times(0);
	EXPECT_CALL(mockTriggerHandler, setTrigger).Times(0);
	for (int i = 0; i < 4; i++) {
		script.second->process();
	}
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);

	// Trigger the start of the first lane and let complete a full loop
	vector<string> emptyTriggers = {};
	vector<string> triggers1 = { "start-1" };
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers1));
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, laneLooped).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
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

	// Consecutive process calls:
	// 1. Start the first (complete on 3, looping on 4) and fourth (not-looping, complete on 3) lanes
	// 2. Start the second (complete on 4, looping on 5) lane
	// 3. Start the third (complete on 5, looping on 6) lane; first and fourth lane should complete
	// 4. Start the fifth (not-looping, should complete on 6) lane; first lane should loop (complete on 6, loop on 7), fourth lane should not loop, second lane should complete
	// 5. Stop the third lane (i.e. should not complete or loop anymore); trigger; second lane should loop (complete on 7, loop on 8)
	// 6. Trigger a new start on the second lane (already in progress, so should do nothing and still complete on 7, loop on 8); first lane should complete and loop on 7; fifth lane should complete and not loop
	// 7. first lane should loop (complete on 9, loop on 10); second lane should complete (and loop on 8)
	// 8. second lane loops (complete on 10, loop on 11)
	// 9. Trigger a restart of third (not running, so should just start, complete on 11, loop on 12); first lane completes (loop on 10)
	// 10. first lane loops (completes on 12, loops on 13); second lane completes (loops on 11)
	// 11. Restart first lane (so it loops on 13, completes on 14); second lane loops (completes on 13, loops on 14); third lane completes (loops on 12)
	// 12. Third lane restarts (completes on 14, loops on 15)
	// 13. First lane completes (loops on 14), Second lane completes (loops on 14)
	// 14. First lane loops, Second loops, Third lane completes
	triggers1 = { "start-1", "start-4" };
	vector<string> triggers2 = { "start-2" };
	vector<string> triggers3 = { "start-3" };
	vector<string> triggers4 = { "start-5" };
	vector<string> triggers5 = { "stop-3" };
	vector<string> triggers6 = { "start-2" };
	vector<string> triggers7 = {};
	vector<string> triggers8 = { "restart-3" };
	vector<string> triggers9 = { "restart-1" };
	{
		testing::InSequence inSequence;

		// 1.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers1));
		EXPECT_CALL(mockEventListener, segmentStarted).Times(2);

		// 2.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers2));
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);

		// 3.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers3));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger4Name)).Times(1);

		// 4.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers4));
		EXPECT_CALL(mockEventListener, laneLooped).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);

		// 5.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers5));
		EXPECT_CALL(mockEventListener, laneLooped).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);

		// 6.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers6));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger5Name)).Times(1);

		// 7.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers7));
		EXPECT_CALL(mockEventListener, laneLooped).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);

		// 8.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers7));
		EXPECT_CALL(mockEventListener, laneLooped).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);

		// 9.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers8));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);

		// 10.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers7));
		EXPECT_CALL(mockEventListener, laneLooped).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);

		// 11.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers9));
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1); // Restart of first lane
		EXPECT_CALL(mockEventListener, laneLooped).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1); // Loop of second lane
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);

		// 12.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers7));
		EXPECT_CALL(mockEventListener, laneLooped).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);

		// 13.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers7));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);

		// 14.
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(1).WillOnce(testing::ReturnRef(triggers7));
		EXPECT_CALL(mockEventListener, laneLooped).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);
		EXPECT_CALL(mockEventListener, laneLooped).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
	}
	for (int i = 0; i < 14; i++) {
		script.second->process();
	}
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	testing::Mock::VerifyAndClearExpectations(&mockEventListener);
}

TEST(TimeSeqProcessorTimelines, ScriptWithMutlipleTimelinesWithLanesAndLoopLockMixedShouldHandleLanesSeparately) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "loop-lock", true }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1.1" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-1.2" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-1.3" }} }) }, { "loop", true } }
		}) } },
		{ { "loop-lock", false }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-2.1" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2.2" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2.3" }} }) }, { "loop", true } }
		}) } },
		{ { "loop-lock", true }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-3.1" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3.2" }} }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3.3" }} }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{
			{ "id", "segment-1.1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({	// Loop-locked, so re-triggers with lane: will trigger on the first step every 3 steps
				{ { "timing", "end" }, { "trigger", "trigger-1.1" } }									// Triggers on 0, 3, 6, 9
			})}
		},
		{
			{ "id", "segment-1.2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({	// Loop-locked, so re-triggers with lane: will trigger on the second step every 3 steps
				{ { "timing", "end" }, { "trigger", "trigger-1.2" } }									// Triggers on 1, 4, 7, 10
			})}
		},
		{
			{ "id", "segment-1.3" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({ 	// Loop-locked, so re-triggers with lane: will trigger on the third step every 3 steps
				{ { "timing", "end" }, { "trigger", "trigger-1.3" } }									// Triggers on 2, 5, 8, 11
			})}
		},
		{
			{ "id", "segment-2.1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({	// Not loop-locked, so triggers every step
				{ { "timing", "end" }, { "trigger", "trigger-2.1" } }									// Triggers on every step
			})}
		},
		{
			{ "id", "segment-2.2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({	// Not loop-locked, so triggers every two steps
				{ { "timing", "end" }, { "trigger", "trigger-2.2" } }									// Triggers on 1, 3, 5, 7, 9, 11
			})}
		},
		{
			{ "id", "segment-2.3" }, { "duration", { { "samples", 4 } } }, { "actions", json::array({	// Not loop-locked, so triggers every four steps
				{ { "timing", "end" }, { "trigger", "trigger-2.3" } }									// 3, 7, 11
			})}
		},
		{
			{ "id", "segment-3.1" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({	// Loop-locked, so re-triggers with lane: will trigger on the second step every 5 steps
				{ { "timing", "end" }, { "trigger", "trigger-3.1" } }									// Triggers on 1, 6, 11
			})}
		},
		{
			{ "id", "segment-3.2" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({	// Loop-locked, so re-triggers with lane: will trigger on the third step every 5 steps
				{ { "timing", "end" }, { "trigger", "trigger-3.2" } }									// Triggers on 2, 7
			})}
		},
		{
			{ "id", "segment-3.3" }, { "duration", { { "samples", 5 } } }, { "actions", json::array({	// Loop-locked, so re-triggers with lane: will trigger on the fifth step every 5 steps
				{ { "timing", "end" }, { "trigger", "trigger-3.3" } }									// Triggers on 4, 9
			})}
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 3u);
	ASSERT_EQ(script.second->m_timelines[0]->m_scriptTimeline->loopLock, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[1]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[2]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[1]->m_scriptTimeline->loopLock, false);
	ASSERT_EQ(script.second->m_timelines[1]->m_lanes.size(), 3u);
	ASSERT_EQ(script.second->m_timelines[1]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[1]->m_lanes[1]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[1]->m_lanes[2]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[2]->m_scriptTimeline->loopLock, true);
	ASSERT_EQ(script.second->m_timelines[2]->m_lanes.size(), 3u);
	ASSERT_EQ(script.second->m_timelines[2]->m_lanes[0]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[2]->m_lanes[1]->m_scriptLane->loop, true);
	ASSERT_EQ(script.second->m_timelines[2]->m_lanes[2]->m_scriptLane->loop, true);

	std::string trigger11 = "trigger-1.1";
	std::string trigger12 = "trigger-1.2";
	std::string trigger13 = "trigger-1.3";
	std::string trigger21 = "trigger-2.1";
	std::string trigger22 = "trigger-2.2";
	std::string trigger23 = "trigger-2.3";
	std::string trigger31 = "trigger-3.1";
	std::string trigger32 = "trigger-3.2";
	std::string trigger33 = "trigger-3.3";

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger11)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger12)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger22)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger31));
 		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger13)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger32));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger11)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger22)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger23));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger12)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger33));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger13)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger22));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger11)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger31));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger12)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger22)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger23)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger32));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger13)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger11)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger22)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger33));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger12)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger13)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger21)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger22)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger23)); EXPECT_CALL(mockTriggerHandler, setTrigger(trigger31));
	}

	for (int i = 0; i < 12; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorTimelines, ScriptWithLoopingLaneAndRepeatingLaneShouldWork) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" }} }) }, { "repeat", 3 }, { "start-trigger", "start-1" } },
			{ { "segments", json::array({ { { "ref", "segment-2" }} }) }, { "loop", true } }
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
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 2u);

	vector<string> emptyTrigger = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(15).WillRepeatedly(testing::ReturnRef(emptyTrigger));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(3); // Should stop repeating after 3 times
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(5); // Should keep repeating

	for (int i = 0; i < 15; i++) {
		script.second->process();
	}
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// After a start trigger, the first lane should repeat the same amount again
	vector<string> startTrigger = { "start-1" };
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(15).WillOnce(testing::ReturnRef(startTrigger)).WillRepeatedly(testing::ReturnRef(emptyTrigger));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(3); // Should stop repeating after 3 times
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(5); // Should keep repeating

	for (int i = 0; i < 15; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorTimelines, ScriptWithTwoRepeatingLanesShouldLoopSeparate) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" }} }) }, { "repeat", 3 }, { "start-trigger", "start-1" } },
			{ { "segments", json::array({ { { "ref", "segment-2" }} }) }, { "repeat", 3 }, { "start-trigger", "start-2" } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{
			{ "id", "segment-1" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{
			{ "id", "segment-2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			})}
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u) << validationErrors[0].location << " " << validationErrors[0].message;
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 2u);

	vector<string> emptyTrigger = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(15).WillRepeatedly(testing::ReturnRef(emptyTrigger));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(3);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(3);

	for (int i = 0; i < 15; i++) {
		script.second->process();
	}
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// After a start trigger, the first lane should repeat the same amount again
	vector<string> startTrigger1 = { "start-1" };
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(15).WillOnce(testing::ReturnRef(startTrigger1)).WillRepeatedly(testing::ReturnRef(emptyTrigger));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(3); // Should stop repeating after 3 times

	for (int i = 0; i < 15; i++) {
		script.second->process();
	}

	// After a start trigger, the second lane should repeat the same amount again
	vector<string> startTrigger2 = { "start-2" };
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(15).WillOnce(testing::ReturnRef(startTrigger2)).WillRepeatedly(testing::ReturnRef(emptyTrigger));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(3); // Should stop repeating after 3 times

	for (int i = 0; i < 15; i++) {
		script.second->process();
	}

	// After a start trigger for both, the both lanes should repeat the same amount again
	vector<string> startTrigger3 = { "start-1", "start-2" };
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(15).WillOnce(testing::ReturnRef(startTrigger3)).WillRepeatedly(testing::ReturnRef(emptyTrigger));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(3); // Should stop repeating after 3 times
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(3); // Should stop repeating after 3 times

	for (int i = 0; i < 15; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorTimelines, ScriptWithTwoLoopAndRepeatShouldKeepLooping) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" }} }) }, { "loop", true }, { "repeat", 3 } },
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{
			{ "id", "segment-1" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u) << validationErrors[0].location << " " << validationErrors[0].message;
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);

	MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(10);

	for (int i = 0; i < 20; i++) {
		script.second->process();
	}
}