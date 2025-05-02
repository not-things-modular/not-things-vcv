#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorSegment, ScriptWithoutComponentPoolAndRefToUnknownSegmentShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0");
}

TEST(TimeSeqProcessorSegment, ScriptWithoutSegmentPoolAndRefToUnknownSegmentShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0");
}

TEST(TimeSeqProcessorSegment, ScriptWithSegmentPoolAndRefToUnknownSegmentShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-2" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		}
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0");
}

TEST(TimeSeqProcessorSegment, ScriptWithRefSegmentShouldLoadRefScript) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-1" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		}
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
	}

	script.second->process();
}

TEST(TimeSeqProcessorSegment, ScriptWithInlineSegmentShouldLoadSegmentScript) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{
					{ "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "timing", "end" }, { "trigger", "trigger-1" } }
					})}
				}
			}) } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u) << validationErrors[0].location << " " << validationErrors[0].message;
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
	}

	script.second->process();
}

TEST(TimeSeqProcessorSegment, ScriptWithShouldLoadStartEndAndGlideSegmentActions) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockVariableHandler mockVariableHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{
					{ "duration", { { "samples", 3 } } }, { "actions", json::array({
						{ { "timing", "start" }, { "trigger", "start-1" } },
						{ { "timing", "end" }, { "trigger", "end-1" } },
						{ { "timing", "glide" }, { "start-value", { { "voltage", 1 } } }, { "end-value", { { "voltage", 1 } } }, { "variable", "var-1" } },
						{ { "timing", "end" }, { "trigger", "end-2" } },
						{ { "timing", "glide" }, { "start-value", { { "voltage", 2 } } }, { "end-value", { { "voltage", 2 } } }, { "variable", "var-2" } },
						{ { "timing", "start" }, { "trigger", "start-2" } },
					})}
				}
			}) } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(script.second->m_timelines.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes.size(), 1u);
	ASSERT_EQ(script.second->m_timelines[0]->m_lanes[0]->m_segments.size(), 1u);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("start-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("start-2")).Times(1);
		EXPECT_CALL(mockVariableHandler, setVariable("var-1", 1)).Times(1);
		EXPECT_CALL(mockVariableHandler, setVariable("var-2", 2)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable("var-1", 1)).Times(1);
		EXPECT_CALL(mockVariableHandler, setVariable("var-2", 2)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable("var-1", 1)).Times(1);
		EXPECT_CALL(mockVariableHandler, setVariable("var-2", 2)).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("end-1")).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger("end-2")).Times(1);
	}

	for (int i = 0; i < 3; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSegment, ScriptWithRefActionButNoPooldActionsShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockVariableHandler mockVariableHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{
					{ "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "ref", "unknown-ref" } }
					})}
				}
			}) } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-ref'"), std::string::npos);
}

TEST(TimeSeqProcessorSegment, ScriptWithRefActionButActionNotInPoolShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockVariableHandler mockVariableHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
				{
					{ "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "ref", "unknown-ref" } }
					})}
				}
			}) } }
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["actions"] = json::array({
		{ { "id", "an-action" }, { "timing", "start" }, { "trigger", "start-1" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0");
	EXPECT_NE(validationErrors[0].message.find("'unknown-ref'"), std::string::npos);
}

TEST(TimeSeqProcessorSegment, ScriptWithCircularSegmentReferenceInSegmentBlockThroughSegmenthouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-2" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{ { "id", "segment-2" }, { "segment-block", "segment-block-1" } },
		{ { "id", "segment-3" }, { "segment-block", "segment-block-2" } }
	});
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-3" } } })} },
		{ { "id", "segment-block-2" }, { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-2" } } })} },
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_CircularFound, "/component-pool/segment-blocks/1/segments/1");
	EXPECT_NE(validationErrors[0].message.find("'segment-2'"), std::string::npos);
}

TEST(TimeSeqProcessorSegment, ScriptWithCircularSegmentReferenceInSegmentBlockThroughSegmentBlockhouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "ref", "segment-2" } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["segments"] = json::array({
		{
			{ "id", "segment-1" }, { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			})}
		},
		{ { "id", "segment-2" }, { "segment-block", "segment-block-1" } },
		{ { "id", "segment-3" }, { "segment-block", "segment-block-2" } }
	});
	json["component-pool"]["segment-blocks"] = json::array({
		{ { "id", "segment-block-1" }, { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-3" } } })} },
		{ { "id", "segment-block-2" }, { "segments", json::array({ { { "ref", "segment-1" } }, { { "segment-block", "segment-block-3" } } })} },
		{ { "id", "segment-block-3" }, { "segments", json::array({ { { "ref", "segment-1" } }, { { "ref", "segment-2" } } })} },
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_CircularFound, "/component-pool/segment-blocks/2/segments/1");
	EXPECT_NE(validationErrors[0].message.find("'segment-2'"), std::string::npos);
}
