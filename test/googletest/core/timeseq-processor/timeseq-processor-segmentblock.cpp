#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorSegmentBlock, ScriptWithSegmentBlocksShouldExecuteSegmentsInOrder) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "segment-block", "segment-block-1" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "ref", "segment-1.2" } }, { { "ref", "segment-1.3" } } })} }
	});
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1.1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{
			{ "id", "segment-1.2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			})}
		},
		{
			{ "id", "segment-1.3" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			})}
		}
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 3u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithTwoLanesWithSegmentBlocksShouldExecuteSegmentsInOrder) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "segment-block", "segment-block-1" } } }) } },
			{ { "segments", json::array({ { { "segment-block", "segment-block-2" } } }) } }
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "ref", "segment-1.2" } }, { { "ref", "segment-1.3" } } })} },
		{ { "id", "segment-block-2" }, { "segments", json::array({ { { "ref", "segment-2.1" } }, { { "ref", "segment-2.2" } }, { { "ref", "segment-2.3" } } })} }
	});
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1.1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1.1" } }
			})}
		},
		{
			{ "id", "segment-1.2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1.2" } }
			})}
		},
		{
			{ "id", "segment-1.3" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1.3" } }
			})}
		},
		{
			{ "id", "segment-2.1" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2.1" } }
			})}
		},
		{
			{ "id", "segment-2.2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2.2" } }
			})}
		},
		{
			{ "id", "segment-2.3" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2.3" } }
			})}
		}
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 2u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 3u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[1]->m_segments.size(), 3u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1.1")).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1.2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2.1")).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2.2")).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1.3")).Times(1);
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2.3")).Times(1);
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithMixOfSegmentBlocksAndSingleSegmentsShouldExecuteSegmentsInOrder) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-0.1" } }, { { "segment-block", "segment-block-1" } }, { { "ref", "segment-0.2" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "ref", "segment-1.2" } }, { { "ref", "segment-1.3" } } })} }
	});
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1.1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{
			{ "id", "segment-1.2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			})}
		},
		{
			{ "id", "segment-1.3" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			})}
		},
		{
			{ "id", "segment-0.1" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-0.1" } }
			})}
		},
		{
			{ "id", "segment-0.2" }, { "duration", { { "samples", 4 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-0.2" } }
			})}
		}
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 5u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-0.1")).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(3).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-0.2")).Times(1);
	}

	for (int i = 0; i < 12; i++) {
		script.second->process();
	}
}
