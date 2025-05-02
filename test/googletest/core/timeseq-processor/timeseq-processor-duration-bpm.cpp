#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorDuration, DurationInBeatsWithNoTimeScaleOnTimelineShouldFail) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Duration_BeatsButNoBmp, "/component-pool/segments/0/duration");
}

TEST(TimeSeqProcessorDuration, DurationInBeatsWithNoBpmOnTimelineShouldFail) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "sample-rate", 44100 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Duration_BeatsButNoBmp, "/component-pool/segments/0/duration");
}

TEST(TimeSeqProcessorDuration, DurationWithBeatsAndBarsWithNoBpbOnTimelineShouldFail) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "bpm", 120 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 1 }, { "bars", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Duration_BarsButNoBpb, "/component-pool/segments/0/duration");
}

TEST(TimeSeqProcessorDuration, DurationInBeatsWithIntegerBeatsAndNoPartialSamplesToBeatsShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "bpm", 120 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "beats", 4 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "beats", 12 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(480));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(480 * 60).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(120);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(30);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(10);

	for (int i = 0; i < 480 * 60; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInBeatsWithIntegerBeatsAndPartialSamplesToBeatsShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "bpm", 88 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "beats", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "beats", 22 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(69));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(69 * 6000).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(8800);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(4400);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(400);

	for (int i = 0; i < 69 * 6000; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInBeatsWithPartialBeatsAndPartialSamplesToBeatsShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "bpm", 40 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 1.5 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "beats", 1.f / 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "beats", 0.6 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(567));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(567 * 1800).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(400 * 2);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1800 * 2);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(1000 * 2);

	for (int i = 0; i < 567 * 1800; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInBeatsWithBeatsAndBarsShouldAddBarsToBeats) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "bpm", 120 }, { "bpb", 4 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 0 }, { "bars", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "beats", 2 }, { "bars", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "beats", 4 }, { "bars", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(480));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(480 * 60).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(30);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(20);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(10);

	for (int i = 0; i < 480 * 60; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInBpmShouldNotGoBelowOneSample) {
	testing::NiceMock<MockEventListener> mockEventListener;
	testing::NiceMock<MockTriggerHandler> mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "bpm", 120 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", 0.002 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(480));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(100);

	for (int i = 0; i < 100; i++) {
		script.second->process();
	}
}
