#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorSegmentBlock, ScriptWithUnknownSegmentBlockRefFromOtherSegmentBlockShouldFail) {
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
		{ { "id", "segment-block-1" }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "segment-block", "segment-block-2" } }, { { "ref", "segment-1.3" } } })} },
		{ { "id", "segment-block-2" }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "segment-block", "segment-block-3" } }, { { "ref", "segment-1.3" } } })} },
		{ { "id", "segment-block-3" }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "segment-block", "segment-block-4" } }, { { "ref", "segment-1.3" } } })} }
	});
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1.1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{
			{ "id", "segment-1.3" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			})}
		}
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/component-pool/segment-blocks/2/segments/1/segment-block");
	EXPECT_NE(validationErrors[0].message.find("'segment-block-4'"), std::string::npos);
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithUnknownSegmentBlockRefFromSegmentsShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "segment-block", "segment-block-4" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "segment-block", "segment-block-2" } }, { { "ref", "segment-1.3" } } })} },
		{ { "id", "segment-block-2" }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "ref", "segment-1.3" } } })} },
	});
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1.1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{
			{ "id", "segment-1.3" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			})}
		}
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/segment-block");
	EXPECT_NE(validationErrors[0].message.find("'segment-block-4'"), std::string::npos);
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithSegmentBlocksShouldNotAllowSegmentBlockRecursion) {
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
		{ { "id", "segment-block-1" }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "segment-block", "segment-block-2" } }, { { "ref", "segment-1.3" } } })} },
		{ { "id", "segment-block-2" }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "segment-block", "segment-block-3" } }, { { "ref", "segment-1.3" } } })} },
		{ { "id", "segment-block-3" }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "segment-block", "segment-block-1" } }, { { "ref", "segment-1.3" } } })} }
	});
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1.1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{
			{ "id", "segment-1.3" }, { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-3" } }
			})}
		}
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_CircularFound, "/component-pool/segment-blocks/2/segments/1/segment-block");
	EXPECT_NE(validationErrors[0].message.find("'segment-block-1'"), std::string::npos);
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithSegmentBlockShouldNotAllowNonStartOrEndActions) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ {
				{ "segment-block", "segment-block-1" },
				{ "actions", json::array({
					{ { "timing", "glide" }, { "start-value", { { "voltage", 0.f } } }, { "end-value", { { "voltage", 10.f } } }, { "output", { { "index", 1 } } } },
					{ { "timing", "gate" }, { "output", { { "index", 1 } } } }
				})}
			} }) } },
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
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Segment_SegmentBlockActionTimings, "/timelines/0/lanes/0/segments/0/actions/0");
	expectError(validationErrors, ValidationErrorCode::Segment_SegmentBlockActionTimings, "/timelines/0/lanes/0/segments/0/actions/1");
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithSegmentBlockShouldFailOnUnresolvableSegmentBlockActions) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ {
				{ "segment-block", "segment-block-1" },
				{ "actions", json::array({
					{ { "timing", "start" }, { "trigger", "my-trigger" } },
					{ { "ref", "i-m-not-here" } }
				})}
			} }) } },
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
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/1");
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithSegmentBlocksInlineInLaneShouldExecuteSegmentsInOrder) {
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
	EXPECT_NO_ERRORS(validationErrors);
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

TEST(TimeSeqProcessorSegmentBlock, ScriptWithSegmentBlocksFromSegmentsPoolLaneShouldExecuteSegmentsInOrder) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "blocked-segment" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array({
			{ { "ref", "segment-1.1" } },
			{ { "ref", "segment-1.2" } },
			{
				{ "duration", { { "samples", 3 } } }, { "actions", json::array({
					{ { "timing", "end" }, { "trigger", "trigger-3" } }
				})}
			}
		})} }
	});
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "blocked-segment" }, { "segment-block", "segment-block-1" }
		},
		{
			{ "id", "segment-1.1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{
			{ "id", "segment-1.2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			})}
		}
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
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
	EXPECT_NO_ERRORS(validationErrors);
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
	EXPECT_NO_ERRORS(validationErrors);
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

TEST(TimeSeqProcessorSegmentBlock, ScriptWithRepeatingSegmentBlockShouldRepleatInPlace) {
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
		{ { "id", "segment-block-1" }, { "repeat", 3 }, { "segments", json::array({ { { "ref", "segment-1.1" } }, { { "ref", "segment-1.2" } }, { { "ref", "segment-1.3" } } })} }
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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 11u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-0.1")).Times(1);

		for (int i = 0; i < 3; i++) {
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

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(3).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-0.2")).Times(1);
	}

	for (int i = 0; i < 24; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithCircularSegmentBlockReferenceInSegmentBlockThroughSegmentShouldFail) {
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
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{ { "id", "segment-2" }, { "segment-block", "segment-block-2" } },
		{ { "id", "segment-3" }, { "segment-block", "segment-block-1" } }
	});
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-2" } } })} },
		{ { "id", "segment-block-2" }, { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-3" } } })} },
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_CircularFound, "/component-pool/segments/2/segment-block");
	EXPECT_NE(validationErrors[0].message.find("'segment-block-1'"), std::string::npos);
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithCircularSegmentBlockReferenceInSegmentBlockThroughSegmentBlockShouldFail) {
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
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{ { "id", "segment-2" }, { "segment-block", "segment-block-2" } }
	});
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-2" } } })} },
		{ { "id", "segment-block-2" }, { "segments", json::array({ { { "ref", "segment-1" } }, { { "segment-block", "segment-block-1" } } })} },
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_CircularFound, "/component-pool/segment-blocks/1/segments/1/segment-block");
	EXPECT_NE(validationErrors[0].message.find("'segment-block-1'"), std::string::npos);
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithSegmentAndSegmentBlocksWithSameIdsShouldNotCauseCircularReferenceErrors) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "segment-block", "id-1" } } }) }, { "loop", true } },
			{ { "segments", json::array({ { { "ref", "id-2" } } }) }, { "loop", true } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "id-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{ { "id", "id-2" }, { "segment-block", "id-2" } },
		{ { "id", "id-3" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			})}
		}
	});
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "id-1" }, { "segments", json::array({ { { "ref", "id-1" } } })} },
		{ { "id", "id-2" }, { "segments", json::array({ { { "ref", "id-3" } } })} },
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(6).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(6);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(6);
	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithNonRepeatingSegmentBlockWithSingleSegmentAndStartAndEndActionsShouldTriggerExecuteStartAndEndActions) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{ { "ref", "segment-0.1" } },
				{
					{ "segment-block", "segment-block-1" },
					{ "actions", json::array({
						{ { "timing", "end" }, { "trigger", "block-end-trigger-1" } },
						{ { "timing", "start" }, { "trigger", "block-start-trigger-1" } },
						{ { "timing", "end" }, { "trigger", "block-end-trigger-2" } },
						{ { "timing", "start" }, { "trigger", "block-start-trigger-2" } },
					}) }
				},
				{ { "ref", "segment-0.2" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "repeat", 1 },{ "segments", json::array({ { { "ref", "segment-1.1" } } })} }
	});
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1.1" }, { "duration", { { "samples", 5 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "end-trigger-1.1.1" } },
				{ { "timing", "start" }, { "trigger", "start-trigger-1.1.1" } },
				{ { "timing", "end" }, { "trigger", "end-trigger-1.1.2" } },
				{ { "timing", "start" }, { "trigger", "start-trigger-1.1.2" } },
				{ { "timing", "glide" }, { "start-value", { { "voltage", 0.f } } }, { "end-value", { { "voltage", 4.f } } }, { "output", { { "index", 1 } } } }
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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 3u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		
		// First the 0.1 segment before the segment-block is executed
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-0.1")).Times(1);

		// Then the segment-block gets executed
		// The segment-block segment takes up 5 process calls
		for (int j = 0; j < 5; j++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			if (j == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}

			// The first time the first segment gets executed, the block start actions should get executed, and then the segment start actions
			if (j == 0) {
				EXPECT_CALL(mockTriggerHandler, setTrigger("block-start-trigger-1")).Times(1);
				EXPECT_CALL(mockTriggerHandler, setTrigger("block-start-trigger-2")).Times(1);

				EXPECT_CALL(mockTriggerHandler, setTrigger("start-trigger-1.1.1")).Times(1);
				EXPECT_CALL(mockTriggerHandler, setTrigger("start-trigger-1.1.2")).Times(1);
			}

			// The segment glide actions should get executed
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, j)).Times(1);

			// If this is the last time the segment is executed, the segment end actions should get executed, followed by the block end actions
			if (j == 4) {
				EXPECT_CALL(mockTriggerHandler, setTrigger("end-trigger-1.1.1")).Times(1);
				EXPECT_CALL(mockTriggerHandler, setTrigger("end-trigger-1.1.2")).Times(1);

				EXPECT_CALL(mockTriggerHandler, setTrigger("block-end-trigger-1")).Times(1);
				EXPECT_CALL(mockTriggerHandler, setTrigger("block-end-trigger-2")).Times(1);
			}
		}

		// Finally, the last non-segment-block segment should be executed
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(3).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-0.2")).Times(1);
	}

	int processCount =
		2 + // The first non-block segment
		5 + // 5 for the first segment-block segment
		4;  // The last non-block segment

	for (int i = 0; i < processCount; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithNonRepeatingSegmentBlockWithStartAndEndActionsShouldTriggerExecuteStartAndEndActions) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{ { "ref", "segment-0.1" } },
				{
					{ "segment-block", "segment-block-1" },
					{ "actions", json::array({
						{ { "timing", "end" }, { "trigger", "block-end-trigger-1" } },
						{ { "timing", "start" }, { "trigger", "block-start-trigger-1" } },
						{ { "timing", "end" }, { "trigger", "block-end-trigger-2" } },
						{ { "timing", "start" }, { "trigger", "block-start-trigger-2" } },
					}) }
				},
				{ { "ref", "segment-0.2" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "repeat", 1 },{ "segments", json::array({ { { "ref", "segment-1.1" } }, { { "ref", "segment-1.2" } }, { { "ref", "segment-1.3" } } })} }
	});
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1.1" }, { "duration", { { "samples", 5 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "end-trigger-1.1.1" } },
				{ { "timing", "start" }, { "trigger", "start-trigger-1.1.1" } },
				{ { "timing", "end" }, { "trigger", "end-trigger-1.1.2" } },
				{ { "timing", "start" }, { "trigger", "start-trigger-1.1.2" } },
				{ { "timing", "glide" }, { "start-value", { { "voltage", 0.f } } }, { "end-value", { { "voltage", 4.f } } }, { "output", { { "index", 1 } } } }
			})}
		},
		{
			{ "id", "segment-1.2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			})}
		},
		{
			{ "id", "segment-1.3" }, { "duration", { { "samples", 5 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "end-trigger-1.3.1" } },
				{ { "timing", "start" }, { "trigger", "start-trigger-1.3.1" } },
				{ { "timing", "end" }, { "trigger", "end-trigger-1.3.2" } },
				{ { "timing", "start" }, { "trigger", "start-trigger-1.3.2" } },
				{ { "timing", "glide" }, { "start-value", { { "voltage", 4.f } } }, { "end-value", { { "voltage", 0.f } } }, { "output", { { "index", 3 } } } }
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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 5u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		
		// First the 0.1 segment before the segment-block is executed
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-0.1")).Times(1);

		// Then the segment-block gets executed
		// The first segment-block segment takes up 5 process calls
		for (int j = 0; j < 5; j++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			if (j == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}

			// The first time the first segment gets executed, the block start actions should get executed, and then the segment start actions
			if (j == 0) {
				EXPECT_CALL(mockTriggerHandler, setTrigger("block-start-trigger-1")).Times(1);
				EXPECT_CALL(mockTriggerHandler, setTrigger("block-start-trigger-2")).Times(1);

				EXPECT_CALL(mockTriggerHandler, setTrigger("start-trigger-1.1.1")).Times(1);
				EXPECT_CALL(mockTriggerHandler, setTrigger("start-trigger-1.1.2")).Times(1);
			}

			// The segment glide actions should get executed
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, j)).Times(1);

			// If this is the last time the segment is executed, the segment end actions should get executed
			if (j == 4) {
				EXPECT_CALL(mockTriggerHandler, setTrigger("end-trigger-1.1.1")).Times(1);
				EXPECT_CALL(mockTriggerHandler, setTrigger("end-trigger-1.1.2")).Times(1);
			}
		}

		// The middle segment of the segment-block should take up two process calls, and trigger when it ends
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);

		// The last segment-block segment takes up another 5 process calls
		for (int j = 0; j < 5; j++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			if (j == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			}

			// If this is the first time the segment is executed, the segment start actions should get executed
			if (j == 0) {
				EXPECT_CALL(mockTriggerHandler, setTrigger("start-trigger-1.3.1")).Times(1);
				EXPECT_CALL(mockTriggerHandler, setTrigger("start-trigger-1.3.2")).Times(1);
			}

			// The segment glide actions should get executed (for the last invocation, the exact end value should be used)
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, 4 - j)).Times(1);
			
			// If this is the last time the segment is executed, the segment end actions should get executed and then the segment-block end actions
			if (j == 4) {
				EXPECT_CALL(mockTriggerHandler, setTrigger("end-trigger-1.3.1")).Times(1);
				EXPECT_CALL(mockTriggerHandler, setTrigger("end-trigger-1.3.2")).Times(1);

				EXPECT_CALL(mockTriggerHandler, setTrigger("block-end-trigger-1")).Times(1);
				EXPECT_CALL(mockTriggerHandler, setTrigger("block-end-trigger-2")).Times(1);
			}
		}

		// Finally, the last non-segment-block segment should be executed
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(3).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-0.2")).Times(1);
	}

	int processCount =
		2 + // The first non-block segment
		5 + 2 + 5 + // 5 for the first segment of the block, 2 for the middle, 5 for the last
		4; // The last non-block segment

	for (int i = 0; i < processCount; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSegmentBlock, ScriptWithRepeatingSegmentBlockWithStartAndEndActionsShouldTriggerExecuteStartAndEndActions) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{ { "ref", "segment-0.1" } },
				{
					{ "segment-block", "segment-block-1" },
					{ "actions", json::array({
						{ { "ref", "block-end-trigger-1" } },
						{ { "ref", "block-start-trigger-1" } },
						{ { "timing", "end" }, { "trigger", "block-end-trigger-2" } },
						{ { "timing", "start" }, { "trigger", "block-start-trigger-2" } },
					}) }
				},
				{ { "ref", "segment-0.2" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "repeat", 3 },{ "segments", json::array({ { { "ref", "segment-1.1" } }, { { "ref", "segment-1.2" } }, { { "ref", "segment-1.3" } } })} }
	});
	json["component-pool"]["actions"] = json::array({
		{ { "id", "block-start-trigger-1" }, { "timing", "start" }, { "trigger", "block-start-trigger-1" } },
		{ { "id", "block-end-trigger-1" }, { "timing", "end" }, { "trigger", "block-end-trigger-1" } }
	});
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1.1" }, { "duration", { { "samples", 5 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "end-trigger-1.1.1" } },
				{ { "timing", "start" }, { "trigger", "start-trigger-1.1.1" } },
				{ { "timing", "end" }, { "trigger", "end-trigger-1.1.2" } },
				{ { "timing", "start" }, { "trigger", "start-trigger-1.1.2" } },
				{ { "timing", "glide" }, { "start-value", { { "voltage", 0.f } } }, { "end-value", { { "voltage", 4.f } } }, { "output", { { "index", 1 } } } }
			})}
		},
		{
			{ "id", "segment-1.2" }, { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-2" } }
			})}
		},
		{
			{ "id", "segment-1.3" }, { "duration", { { "samples", 5 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "end-trigger-1.3.1" } },
				{ { "timing", "start" }, { "trigger", "start-trigger-1.3.1" } },
				{ { "timing", "end" }, { "trigger", "end-trigger-1.3.2" } },
				{ { "timing", "start" }, { "trigger", "start-trigger-1.3.2" } },
				{ { "timing", "glide" }, { "start-value", { { "voltage", 4.f } } }, { "end-value", { { "voltage", 0.f } } }, { "output", { { "index", 3 } } } }
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
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 11u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		
		// First the 0.1 segment before the segment-block is executed
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-0.1")).Times(1);

		// Then the segment-block gets executed
		for (int i = 0; i < 3; i++) {
			// The first segment-block segment takes up 5 process calls
			for (int j = 0; j < 5; j++) {
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				if (j == 0) {
					EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
				}

				// If this is the first repeat of the segment-block, and the first time the first segment gets executed, the block start actions should get executed
				if ((i == 0) && (j == 0)) {
					EXPECT_CALL(mockTriggerHandler, setTrigger("block-start-trigger-1")).Times(1);
					EXPECT_CALL(mockTriggerHandler, setTrigger("block-start-trigger-2")).Times(1);
				}

				// If this is the first time the segment is executed, the segment start actions should get executed
				if (j == 0) {
					EXPECT_CALL(mockTriggerHandler, setTrigger("start-trigger-1.1.1")).Times(1);
					EXPECT_CALL(mockTriggerHandler, setTrigger("start-trigger-1.1.2")).Times(1);
				}

				// The segment glide actions should get executed
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, j)).Times(1);

				// If this is the last time the segment is executed, the segment end actions should get executed
				if (j == 4) {
					EXPECT_CALL(mockTriggerHandler, setTrigger("end-trigger-1.1.1")).Times(1);
					EXPECT_CALL(mockTriggerHandler, setTrigger("end-trigger-1.1.2")).Times(1);
				}
			}

			// The middle segment of the segment-block should take up two process calls, and trigger when it ends
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);

			// The last segment-block segment takes up another 5 process calls
			for (int j = 0; j < 5; j++) {
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				if (j == 0) {
					EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
				}

				// If this is the first time the segment is executed, the segment start actions should get executed
				if (j == 0) {
					EXPECT_CALL(mockTriggerHandler, setTrigger("start-trigger-1.3.1")).Times(1);
					EXPECT_CALL(mockTriggerHandler, setTrigger("start-trigger-1.3.2")).Times(1);
				}

				// The segment glide actions should get executed (for the last invocation, the exact end value should be used)
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, 4 - j)).Times(1);
				
				// If this is the last time the segment is executed, the segment end actions should get executed
				if (j == 4) {
					EXPECT_CALL(mockTriggerHandler, setTrigger("end-trigger-1.3.1")).Times(1);
					EXPECT_CALL(mockTriggerHandler, setTrigger("end-trigger-1.3.2")).Times(1);
				}

				// If this is the last repeat of the segment-block, and the last time the last segment gets executed, the block end actions should get executed
				if ((i == 2) && (j == 4)) {
					EXPECT_CALL(mockTriggerHandler, setTrigger("block-end-trigger-1")).Times(1);
					EXPECT_CALL(mockTriggerHandler, setTrigger("block-end-trigger-2")).Times(1);
				}
			}
		}

		// Finally, the last non-segment-block segment should be executed
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(3).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-0.2")).Times(1);
	}

	int processCount =
		2 + // The first non-block segment
		(5 + 2 + 5) * 3 + // (5 for the first segment of the block, 2 for the middle, 5 for the last) -> repeated 3 times
		4; // The last non-block segment

	for (int i = 0; i < processCount; i++) {
		script.second->process();
	}
}
