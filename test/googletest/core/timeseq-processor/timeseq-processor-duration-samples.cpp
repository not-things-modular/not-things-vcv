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
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(540).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
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
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(540).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
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
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(540).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
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
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(540 / 2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100 / 2).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
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
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1);
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
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(2330);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(2796);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1500);

	for (int i = 0; i < 3495; i++) { // Use the least common multiple of the scaled sample rates (1.5, 1.25 and 2.33)
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInSamplesWithVariableSamplesNoRateMappingAndNoPartialSamplesShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockPortHandler mockPortHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", { { "input", 1 } } } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 2; i++) {
			// First let it run for 10 samples
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(10.f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// Then let it run for 5 samples
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(5.f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(4).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// And then for 15 samples
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(15.f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(14).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		}
	}

	for (int i = 0; i < (10 + 5 + 15) * 2; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInSamplesWithVariableSamplesNoRateMappingAndPartialSamplesShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockPortHandler mockPortHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", { { "input", 1 } } } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 2; i++) {
			// First let it run for 10.25 samples
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(10.25f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// Then let it run for 5.5 samples
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(5.5f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(4).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// And then for 15.25 samples (which results in a drift of 1)
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(15.25f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(14 + 1).WillRepeatedly(testing::ReturnRef(emptyTriggers)); // One additional sample due to the drift
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		}
	}

	for (int i = 0; i < (10 + 5 + 15 + 1) * 2; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInSamplesWithVariableSamplesRateMappingAndPartialSamplesShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockPortHandler mockPortHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "sample-rate", 48000 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", { { "input", 1 } } } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(24000)); // The real sample rate is half that of the target, so all sample durations should be halved

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 2; i++) {
			// First let it run for 10.25 * 2 samples
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(10.25f * 2.));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(9).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// Then let it run for 5.5 * 2 samples
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(5.5f * 2.));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(4).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// And then for 15.25 * 2 samples
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(15.25f * 2.));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(14 + 1).WillRepeatedly(testing::ReturnRef(emptyTriggers)); // One additional sample due to the drift
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		}
	}

	for (int i = 0; i < (10 + 5 + 15 + 1) * 2; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInVariableSamplesShouldNotGoBelowOneSample) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", { { "input", 1 } } } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(48));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(100).WillRepeatedly(testing::Return(0.1f));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(100);

	for (int i = 0; i < 100; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInSamplesWithVariableSamplesShouldQueryDurationAfterStartActionsAndBeforeOtherActions) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockPortHandler mockPortHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "samples", { { "input", 1 } } } } }, { "actions", json::array({
				{ { "timing", "start" }, { "set-value", { { "output", 1 }, { "value", 6.9f } } } },
				{ { "timing", "glide" }, { "start-value", 1.f }, { "end-value", 2.f }, { "output", 2 } },
				{ { "timing", "end" }, { "set-value", { { "output", 3 }, { "value", 9.6f } } } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(3));


	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 2; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			// First the start action should be executed
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 6.9f)).Times(1);
			// Then the duration should be queried
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(3.f));
			// Then the glide action should happen
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 1.f)).Times(1);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 1.5f)).Times(1);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 2.f)).Times(1);
			// Finally the end action should happen
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, 9.6f)).Times(1);
		}
	}

	for (int i = 0; i < 3 * 2; i++) {
		script.second->process();
	}
}
