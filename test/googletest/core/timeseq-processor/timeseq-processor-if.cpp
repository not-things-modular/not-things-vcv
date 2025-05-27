#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorIf, ActionWithIfShouldFailOnUnknownValueRef) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "eq", json::array({
						{ { "voltage", 1.f } },
						{ { "ref", "unknown-value-ref" } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/if/eq/1");
}

TEST(TimeSeqProcessorIf, ActionWithIfShouldFailOnInvalidValues) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "eq", json::array({
						{ { "voltage", "1.f" } },
						{ { "ref", "invalid-value-ref" } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "invalid-value-ref" }, { "note", "h4" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageFloat, "/timelines/0/lanes/0/segments/0/actions/0/if/eq/0");
	expectError(validationErrors, ValidationErrorCode::Value_NoteFormat, "/component-pool/values/0");
}

TEST(TimeSeqProcessorIf, ActionWithIfShouldDetectCircularRefInValues) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "eq", json::array({
						{ { "voltage", 1.f } },
						{ { "ref", "variable-value-id" } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" }, { "calc", json::array({ { { "add", { { "ref", "variable-value-id" } } } } }) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_CircularFound, "/component-pool/values/0/calc/0/add");
	EXPECT_NE(validationErrors[0].message.find("'variable-value-id'"), std::string::npos);
}

TEST(TimeSeqProcessorIf, ActionWithUnknownIfRefShouldFail) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "ref", "an-unknown-id" } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" } }
	});
	json["component-pool"]["ifs"] = json::array({
		{ { "id", "not-the-id" }, { "eq", json::array({
			{ { "voltage", 1.f } },
			{ { "ref", "variable-value-id" } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/if");
	EXPECT_NE(validationErrors[0].message.find("'an-unknown-id'"), std::string::npos);
}

TEST(TimeSeqProcessorIf, ActionWithCircularIfRefShouldFail) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "ref", "if-id-1" } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" } }
	});
	json["component-pool"]["ifs"] = json::array({
		{ { "id", "if-id-1" }, { "and", json::array({
			{ { "eq", json::array({
				{ { "voltage", 1.f } },
				{ { "voltage", 2.f } }
			}) } },
			{ { "ref", "if-id-1" } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_CircularFound, "/component-pool/ifs/0/and/1");
	EXPECT_NE(validationErrors[0].message.find("'if-id-1'"), std::string::npos);
}

TEST(TimeSeqProcessorIf, ActionWithEqIfShouldCheckIfResult) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "eq", json::array({
						{ { "voltage", 1.f } },
						{ { "ref", "variable-value-id" } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values[] = { 0.9999999f, 1.f, -10.f, 1.f, 1.0000001f, 1.f };
		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i]));
			if (i % 2 == 1) {
				EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 3.45f)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIf, ActionWithEqIfShouldCheckRefIfResult) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "ref", "ref-id" } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" } }
	});
	json["component-pool"]["ifs"] = json::array({
		{ { "id", "ref-id" }, {"eq", json::array({
			{ { "voltage", 1.f } },
			{ { "ref", "variable-value-id" } }
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values[] = { 0.9999999f, 1.f, -10.f, 1.f, 1.0000001f, 1.f };
		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i]));
			if (i % 2 == 1) {
				EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 3.45f)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIf, ActionWithEqIfShouldCheckIfResultWithTolerance) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "tolerance", .0001f }, { "eq", json::array({
						{ { "voltage", 1.f } },
						{ { "ref", "variable-value-id" } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values[] = { 0.9999999f, 1.f, -10.f, 1.f, 1.0000001f, 1.f };
		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i]));
			if ((i % 2 == 1) || (i == 0) || (i == 4)) {
				EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 3.45f)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIf, ActionWithNeIfShouldCheckIfResultWithTolerance) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "tolerance", .0001f }, { "ne", json::array({
						{ { "voltage", 1.f } },
						{ { "ref", "variable-value-id" } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values[] = { 0.9999999f, 1.f, -10.f, 1.f, 1.0000001f, 1.f };
		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i]));
			if (i == 2) {
				EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 3.45f)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIf, ActionWithNeIfShouldCheckIfResult) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "ne", json::array({
						{ { "voltage", 1.f } },
						{ { "ref", "variable-value-id" } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values[] = { 0.9999999f, 1.f, -10.f, 1.f, 1.0000001f, 1.f };
		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i]));
			if (i % 2 == 0) {
				EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 3.45f)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIf, ActionWithGtShouldCheckIfResult) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "gt", json::array({
						{ { "voltage", 1.f } },
						{ { "ref", "variable-value-id" } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values[] = {0.9999999f, 1.f, -10.f, 5.f, -5.f, 1.0000001f };
		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i]));
			if (i == 0 || i == 2 || i == 4) {
				EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 3.45f)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIf, ActionWithGteShouldCheckIfResult) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "gte", json::array({
						{ { "voltage", 1.f } },
						{ { "ref", "variable-value-id" } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values[] = {0.9999999f, 1.f, -10.f, 5.f, -5.f, 1.0000001f };
		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i]));
			if (i == 0 || i == 1 || i == 2 || i == 4) {
				EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 3.45f)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIf, ActionWithLtShouldCheckIfResult) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "lt", json::array({
						{ { "voltage", 1.f } },
						{ { "ref", "variable-value-id" } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values[] = {0.9999999f, 1.f, -10.f, 5.f, -5.f, 1.0000001f };
		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i]));
			if (i == 3 || i == 5) {
				EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 3.45f)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIf, ActionWithLteShouldCheckIfResult) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "lte", json::array({
						{ { "voltage", 1.f } },
						{ { "ref", "variable-value-id" } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id" }, { "variable", "input-variable" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values[] = {0.9999999f, 1.f, -10.f, 5.f, -5.f, 1.0000001f };
		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i]));
			if (i == 1 || i == 3 || i == 5) {
				EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 3.45f)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIf, ActionWithAndShouldCheckIfResult) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "and", json::array({
						{ { "eq", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id-1" } }
						}) } },
						{ { "eq", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id-2" } }
						}) } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id-1" }, { "variable", "input-variable-1" } },
		{ { "id", "variable-value-id-2" }, { "variable", "input-variable-2" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 0u) << json.dump();
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values1[] = { 0.f, 0.f, 1.f, 1.f };
		float values2[] = { 0.f, 1.f, 0.f, 1.f };
		for (int i = 0; i < 4; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariable1)).Times(1).WillOnce(testing::Return(values1[i]));
			if ((i == 2) || (i == 3))
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariable2)).Times(1).WillOnce(testing::Return(values2[i]));
			if (i == 3) {
				EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 3.45f)).Times(1);
			}
		}
	}

	for (int i = 0; i < 4; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIf, ActionWithOrShouldCheckIfResult) {
	MockEventListener mockEventListener;
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
					{ "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 3.45f } } } } },
					{ "if", { { "or", json::array({
						{ { "eq", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id-1" } }
						}) } },
						{ { "eq", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id-2" } }
						}) } }
					}) } } }
				}
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "variable-value-id-1" }, { "variable", "input-variable-1" } },
		{ { "id", "variable-value-id-2" }, { "variable", "input-variable-2" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_EQ(validationErrors.size(), 0u) << json.dump();
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values1[] = { 0.f, 0.f, 1.f, 1.f };
		float values2[] = { 0.f, 1.f, 0.f, 1.f };
		for (int i = 0; i < 4; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariable1)).Times(1).WillOnce(testing::Return(values1[i]));
			if ((i == 0) || (i == 1))
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariable2)).Times(1).WillOnce(testing::Return(values2[i]));
			if (i != 0) {
				EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 3.45f)).Times(1);
			}
		}
	}

	for (int i = 0; i < 4; i++) {
		script.second->process();
	}
}
