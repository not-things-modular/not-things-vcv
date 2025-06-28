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

TEST(TimeSeqProcessorDuration, DurationInBeatsValueWithNoTimeScaleOnTimelineShouldFail) {
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
		{ { "id", "segment-1" }, { "duration", { { "beats", { { "input", 1 } } } } }, { "actions", json::array({
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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(480 * 60).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(120);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(30);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(10);

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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(69 * 6000).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(8800);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(4400);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(400);

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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(567 * 1800).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(400 * 2);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1800 * 2);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(1000 * 2);

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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(480 * 60).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(30);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(20);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(10);

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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(100);

	for (int i = 0; i < 100; i++) {
		script.second->process();
	}
}










TEST(TimeSeqProcessorDuration, DurationInBeatsWithVariableBeatsAndNoPartialSamplesToBeatsShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockPortHandler mockPortHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "bpm", 60 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", { { "input", 1 } } } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(480)); // 480 samples per second, so 480 samples per beat


	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 1; i++) {
			// First let it run for 3 beats
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(3.f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times((48 * 3 * 10) - 1).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// Then let it run for 5 beats
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(5.f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times((48 * 5 * 10) - 1).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// And then for 0.5 beats
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(.5f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times((48 * .5f * 10) - 1).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		}
	}

	for (int i = 0; i < (3 + 5 + .5f) * 48 * 10 * 1; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInBeatsWithVariableBeatsAndPartialSamplesToBeatsShouldWork) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockPortHandler mockPortHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "bpm", 60 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", { { "input", 1 } } } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(441)); // 441 results in a partial 441 samples per second, so per beat


	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 2; i++) {
			// First let it run for 1 beats => 441 samples in total
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(1.f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(440).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// Then let it run for .25 beats => 110 samples and 0.25 drift
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(.25f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(109).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// Then for .5 beats => 220 samples and 0.5 drift
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(.5f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(219).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// And finally for another .25 beats => another 110 samples and 0.25 drift (resulting in an overall drift of 1, thus one more cycle)
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(.25f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(110).WillRepeatedly(testing::ReturnRef(emptyTriggers)); // 1 trigger to start, 109 more triggers to get to 2.5 millis and 1 additional trigger for the drift
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		}
	}

	for (int i = 0; i < (441 + 110 + 220 + 110 + 1) * 2; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInVariableBeatsShouldNotGoBelowOneSample) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "bpm", 60 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", { { "input", 1 } } } } }, { "actions", json::array({
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
	EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(100).WillRepeatedly(testing::Return(0.0001f));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(100);

	for (int i = 0; i < 100; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInBeatsWithVariableBeatsShouldQueryDurationAfterStartActionsAndBeforeOtherActions) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockPortHandler mockPortHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "time-scale", { { "bpm", 60000 } } }, { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } }
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "beats", { { "input", 1 } } } } }, { "actions", json::array({
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
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(1000.f));
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
