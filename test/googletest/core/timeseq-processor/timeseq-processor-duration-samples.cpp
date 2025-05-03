#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorDuration, DurationInSamplesWithNoSampleScaleRateShouldUseProcessExactSamples) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-2" } }, { { "ref", "segment-3" } } }) } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 240 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "samples", 540 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "samples", 100 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 3u);

	{
		testing::InSequence inSequence;
		vector<string> emptyTriggers = {};

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(240).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(540).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}

	for (int i = 0; i < 240 + 540 + 100; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInSamplesWithTimeScaleWithoutSampleRateShouldProcessExactSamples) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "bpm", 120 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-2" } }, { { "ref", "segment-3" } } }) } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 240 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "samples", 540 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "samples", 100 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(48000));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 3u);

	{
		testing::InSequence inSequence;
		vector<string> emptyTriggers = {};

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(240).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(540).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}

	for (int i = 0; i < 240 + 540 + 100; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInSamplesWithScaleSampleRateSameAsRealSampleRateShouldProcessExactSamples) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "sample-rate", 48000 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-2" } }, { { "ref", "segment-3" } } }) } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 240 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "samples", 540 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "samples", 100 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(48000));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 3u);

	{
		testing::InSequence inSequence;
		vector<string> emptyTriggers = {};

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(240).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(540).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}

	for (int i = 0; i < 240 + 540 + 100; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInSamplesWithScaleSampleRateDoubleOfRealSampleRateShouldScaleSamples) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "sample-rate", 48000 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-2" } }, { { "ref", "segment-3" } } }) } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 240 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "samples", 540 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "samples", 100 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(24000));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 3u);

	{
		testing::InSequence inSequence;
		vector<string> emptyTriggers = {};

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(240 / 2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(540 / 2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100 / 2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}

	for (int i = 0; i < (240 + 540 + 100) / 2; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInSamplesWithScaledSampleRateShouldLimitDurationToOne) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "sample-rate", 48000 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-2" } }, { { "ref", "segment-3" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 24 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "samples", 12 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(1000));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 3u);

	{
		testing::InSequence inSequence;
		vector<string> emptyTriggers = {};

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1);
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInSamplesWithScaledSampleRateShouldHandleDrift) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "sample-rate", 48000 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", 150 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "samples", 125 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "samples", 233 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(480));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(3495).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(2330);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(2796);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1500);

	for (int i = 0; i < 3495; i++) { // Use the least common multiple of the scaled sample rates (1.5, 1.25 and 2.33)
		script.second->process();
	}
}

