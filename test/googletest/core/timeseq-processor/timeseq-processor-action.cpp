#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorAction, ScriptWithActionRefButNoComponentPoolShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "ref", "unknown-action" } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0");
}

TEST(TimeSeqProcessorAction, ScriptWithActionRefButNoActionPoolShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "ref", "unknown-action" } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0");
}

TEST(TimeSeqProcessorAction, ScriptWithActionRefToUnknownActionShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "ref", "unknown-action" } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["actions"] = json::array({});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0");
}

TEST(TimeSeqProcessorAction, ScriptWithActionRefToInvalidActionShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "ref", "action-id" } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["actions"] = json::array({
		{ { "id", "action-id" }, { "timing", "buh" }, { "trigger", "trigger-1" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Action_TimingEnum, "/component-pool/actions/0");
}

TEST(TimeSeqProcessorAction, ScriptWithActionRefShouldUseAction) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "ref", "action-id" } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["actions"] = json::array({
		{ { "id", "action-id" }, { "timing", "end" }, { "trigger", "trigger-1" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
	}

	script.second->process();
}

TEST(TimeSeqProcessorAction, ScriptWithActionWithoutConditionShouldWork) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "timing", "end" }, { "trigger", "trigger-1" } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
	}

	script.second->process();
}

TEST(TimeSeqProcessorAction, ScriptWithActionWithConditionShouldCheckCondition) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "timing", "end" },
					{ "trigger", "trigger-1" },
					{ "if", { { "eq", json::array({
						{ { "variable", "the-value" } },
						{ { "voltage", 1.0 }}
					}) } } }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		std::string variableName = "the-value";

		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 5; i++) {
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(1.0f));
				EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
			}

			for (int i = 0; i < 5; i++) {
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(2.0f));
				EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
			}
		}
	}

	for (int i = 0; i < 20; i++) {
		script.second->process();
	}
}
