#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorDuration, DurationInMillisWithIntegerMillisAndNoPartialSamplesToMillisShouldWork) {
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
		{ { "id", "segment-1" }, { "duration", { { "millis", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "millis", 10 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "millis", 69 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(48000)); // 48000 results in exactly 48 samples for each milli

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(69 * 480).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(690);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(69);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(10);

	for (int i = 0; i < 690 * 48; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInMillisWithIntegerMillisAndPartialSamplesToMillisShouldWork) {
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
		{ { "id", "segment-1" }, { "duration", { { "millis", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "millis", 9 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "millis", 69 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(44100)); // 44100 results in a fractional 44.1 samples per milli

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(91287).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(2070);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(230);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(30);

	for (int i = 0; i < 91287; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInMillisWithFractionalMillisAndNoPartialSamplesToMillisShouldWork) {
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
		{ { "id", "segment-1" }, { "duration", { { "millis", 0.7 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "millis", 10.1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "millis", 6.6 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(48000)); // 48000 results in exactly 48 samples for each milli

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(46662 * 48).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(66660);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(4620);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(7070);

	for (int i = 0; i < 46662 * 48; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInMillisWithFractionalMillisAndPartialSamplesToMillisShouldWork) {
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
		{ { "id", "segment-1" }, { "duration", { { "millis", .5 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		},
		{ { "id", "segment-2" }, { "duration", { { "millis", 10.1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			}) }
		},
		{ { "id", "segment-3" }, { "duration", { { "millis", 6.6 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(44100)); // 44100 results in a fractional 44.1 samples per milli

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 3u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(3333 * 441).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(66660);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(3300);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger3Name)).Times(5050);

	for (int i = 0; i < 3333 * 441; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInMillisShouldNotGoBelowOneSample) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) }, { "loop", true } },
		}) } }
	});
	json["component-pool"] = { { "segments", json::array({
		{ { "id", "segment-1" }, { "duration", { { "millis", 0.0001 } } }, { "actions", json::array({
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
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(100);

	for (int i = 0; i < 100; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInMillisWithVariableMillisAndNoPartialSamplesToMillisShouldWork) {
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
		{ { "id", "segment-1" }, { "duration", { { "millis", { { "input", 1 } } } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(48000)); // 48000 results in exactly 48 samples for each milli


	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 2; i++) {
			// First let it run for 3 millis
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(3.f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times((48 * 3) - 1).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// Then let it run for 5 millis
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(5.f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times((48 * 5) - 1).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// And then for 0.5 millis
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(.5f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times((48 * .5f) - 1).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		}
	}

	for (int i = 0; i < (3 + 5 + .5f) * 48 * 2; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInMillisWithVariableMillisAndPartialSamplesToMillisShouldWork) {
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
		{ { "id", "segment-1" }, { "duration", { { "millis", { { "input", 1 } } } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) }
		}
	}) } };

	ON_CALL(mockSampleRateReader, getSampleRate()).WillByDefault(testing::Return(44100)); // 44100 results in a partial 44.1 samples per milli


	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	EXPECT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 2; i++) {
			// First let it run for 10 millis => 441 samples in total
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(10.f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(440).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// Then let it run for 2.5 millis => 110 samples and 0.25 drift
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(2.5f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(109).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// Then for 5 millis => 220 samples and 0.5 drift
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(5.f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(219).WillRepeatedly(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);

			// And finally for another 2.5 millis => another 110 samples and 0.25 drift (resulting in an overall drift of 1, thus one more cycle)
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).WillOnce(testing::Return(2.5f));
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(110).WillRepeatedly(testing::ReturnRef(emptyTriggers)); // 1 trigger to start, 109 more triggers to get to 2.5 millis and 1 additional trigger for the drift
			EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		}
	}

	for (int i = 0; i < (441 + 110 + 220 + 110 + 1) * 2; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorDuration, DurationInVariableMillisShouldNotGoBelowOneSample) {
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
		{ { "id", "segment-1" }, { "duration", { { "millis", { { "input", 1 } } } } }, { "actions", json::array({
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

TEST(TimeSeqProcessorDuration, DurationInMillisWithVariableMillisShouldQueryDurationAfterStartActionsAndBeforeOtherActions) {
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
		{ { "id", "segment-1" }, { "duration", { { "millis", { { "input", 1 } } } } }, { "actions", json::array({
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
