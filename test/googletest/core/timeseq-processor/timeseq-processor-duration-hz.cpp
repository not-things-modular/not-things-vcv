#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorDuration, DurationInHzWithIntegerHzAndNoPartialSamplesToHzShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "hz", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "hz", 240 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "hz", 48000 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(48000));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(48000).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(240);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(48000);

	for (int i = 0; i < 48000; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInHzWithIntegerHzAndPartialSamplesToHzShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "hz", 669 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "hz", 420 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "hz", 123 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(48000));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(48000).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(669);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(420);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(123);

	for (int i = 0; i < 48000; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInHzWithFractionalHzAndNoPartialSamplesToHzShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "hz", 44.1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "hz", 22.05 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "hz", 88.2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(44100));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(44100 * 20).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(441 * 2);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(220.5 * 2);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(882 * 2);

	for (int i = 0; i < 44100 * 20; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInHzWithFractionalHzAndPartialSamplesToHzShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-2" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "segment-3" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "hz", 12.3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "hz", 6.9 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "hz", 4.2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(4410));

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(441000).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1230);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(690);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-3")).Times(420);

	for (int i = 0; i < 441000; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInHzShouldNotGoBelowOneSample) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "hz", 45000 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(44100));

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
