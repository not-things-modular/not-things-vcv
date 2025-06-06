#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorSetValueAction, SetValueActionWithUnknownValueShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", { { "index", 1 } } }, { "value", { { "ref", "unknown-value" } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-value/value");
}

TEST(TimeSeqProcessorSetValueAction, SetValueActionWithUnknownOutputShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", { { "ref", "unknown-output" } } }, { "value", { { "variable", "input-variable" } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-value/output");
}

TEST(TimeSeqProcessorSetValueAction, SetValueActionShouldSetProcessedRefValue) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", { { "ref", "the-output" } } }, { "value", { { "ref", "the-value" } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["outputs"] = json::array({
		{ { "id", "the-output" }, { "index", 5 } }
	});
	json["component-pool"]["values"] = json::array({
		{ { "id", "the-value" }, { "variable", "input-variable" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(i));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(4, 0, i)).Times(1);
		}
	}

	for (int i = 0; i < 5; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSetValueAction, SetValueActionShouldSetProcessedValueOnPortWithoutChannel) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", { { "index", 4 } } }, { "value", { { "variable", "input-variable" } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(i));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(3, 0, i)).Times(1);
		}
	}

	for (int i = 0; i < 5; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSetValueAction, SetValueActionShouldSetProcessedValueOnPortWithChannel) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", { { "index", 3 }, { "channel", 6} } }, { "value", { { "variable", "input-variable" } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(i));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 5, i)).Times(1);
		}
	}

	for (int i = 0; i < 5; i++) {
		script.second->process();
	}
}

// TEST(TimeSeqProcessorSetValueAction, SetVariableActionWithShouldSetProcessedValueIfConditionIsTrue) {
// 	MockEventListener mockEventListener;
// 	MockTriggerHandler mockTriggerHandler;
// 	MockSampleRateReader mockSampleRateReader;
// 	MockVariableHandler mockVariableHandler;
// 	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
// 	vector<ValidationError> validationErrors;
// 	json json = getMinimalJson();
// 	json["timelines"] = json::array({
// 		{ { "lanes", json::array({
// 			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
// 				{
// 					{ "set-variable", { { "name", "output-variable" }, { "value", { { "variable", "input-variable" } } } } },
// 					{ "if", { { "eq", json::array({
// 						{ { "variable", "if-variable" } },
// 						{ { "voltage", 1.f } }
// 					}) } } }
// 				}
// 			}) } } }) } },
// 		}) } }
// 	});

// 	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
// 	ASSERT_EQ(validationErrors.size(), 0u) << validationErrors[0].location << " " << validationErrors[0].message;

// 	vector<string> emptyTriggers = {};
// 	{
// 		testing::InSequence inSequence;

// 		for (int i = 0; i < 10; i++) {
// 			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
// 			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
// 			if (i % 2 == 0) {
// 				EXPECT_CALL(mockVariableHandler, getVariable("if-variable")).Times(1).WillOnce(testing::Return(1.f));
// 				EXPECT_CALL(mockVariableHandler, getVariable("input-variable")).Times(1).WillOnce(testing::Return(i));
// 				EXPECT_CALL(mockVariableHandler, setVariable("output-variable", i)).Times(1);
// 			} else {
// 				EXPECT_CALL(mockVariableHandler, getVariable("if-variable")).Times(1).WillOnce(testing::Return(0.f));
// 				EXPECT_CALL(mockVariableHandler, getVariable("input-variable")).Times(0);
// 				EXPECT_CALL(mockVariableHandler, setVariable("output-variable", i)).Times(0);
// 			}
// 		}
// 	}

// 	for (int i = 0; i < 10; i++) {
// 		script.second->process();
// 	}
// }
