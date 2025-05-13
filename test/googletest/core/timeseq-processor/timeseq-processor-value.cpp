#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorValue, ScriptWithValueRefButNoComponentPoolShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "unknown-value" } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value");
}

TEST(TimeSeqProcessorValue, ScriptWithActionRefButNoValuePoolShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "unknown-value" } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value");
}

TEST(TimeSeqProcessorValue, ScriptWithValueRefToUnknownValueShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "unknown-value" } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value");
}

TEST(TimeSeqProcessorValue, ScriptWithValueRefToInvalidValueShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "value-id" } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "value-id" }, { "voltage", "4.2" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageFloat, "/component-pool/values/0");
}

TEST(TimeSeqProcessorValue, ScriptWithActionRefShouldUseAction) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "value-id" } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "another-value-id" }, { "voltage", 6.9f } },
		{ { "id", "value-id" }, { "voltage", 4.2f } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 4.2f)).Times(1);
	}

	script.second->process();
}

TEST(TimeSeqProcessorValue, VoltageValueShouldUseExactValue) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", -5.67f } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 5.67f } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 0.f } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 6.9f } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", -4.2f } } } } } } }) } }
			}) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values[] = { -5.67f, 5.67f, 0.f, 6.9f, -4.2f };
		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, setVariable("output-variable", values[i])).Times(1);
		}
	}

	for (int i = 0; i < 5; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValue, NoteValueShouldUseExactValue) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "c4" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "C4" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "c3" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "c3+" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "c5-" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "c4" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "d4" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "e4" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "f4" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "g4" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "a4" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "b4" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "b6-" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "e2+" } } } } } } }) } },
			}) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		float values[] = {
			0.f, 0.f, -1.f,
			-1.f + (1.f / 12), 1.f - (1.f / 12),
			0.f, (1.f / 12) * 2, (1.f / 12) * 4, (1.f / 12) * 5, (1.f / 12) * 7, (1.f / 12) * 9, (1.f / 12) * 11,
			2 + (1.f / 12) * 10, -2 + (1.f / 12) * 5
		};
		for (int i = 0; i < 14; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, setVariable("output-variable", testing::FloatEq(values[i]))).Times(1);
		}
	}

	for (int i = 0; i < 14; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValue, InputValueShouldFailOnUnknownInputRef) {
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
			{ { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "input", { { "ref", "unknown-ref" } } } } } } } } }) } }
			}) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["inputs"] = json::array({
		{ { "id", "not-it-input-1" }, { "index", 8 } },
		{ { "id", "not-it-input-2" }, { "index", 6 }, { "channel", 9 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value/input");
}

TEST(TimeSeqProcessorValue, InputValueShouldReadInputVoltage) {
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
			{ { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "input", { { "index", 2 } } } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "input", { { "index", 1 }, { "channel", 15 } } } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "input", { { "ref", "input-1" } } } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "value-1" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "value-2" } } } } } } }) } }
			}) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["inputs"] = json::array({
		{ { "id", "input-1" }, { "index", 8 } },
		{ { "id", "input-2" }, { "index", 6 }, { "channel", 9 } }
	});
	json["component-pool"]["values"] = json::array({
		{ { "id", "value-1" }, { "input", { { "index", 3 }, { "channel", 6 } } } },
		{ { "id", "value-2" }, { "input", { { "ref", "input-2" } } } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 1.f)).Times(1);
		
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 14)).Times(1).WillOnce(testing::Return(2.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 2.f)).Times(1);
		
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(7, 0)).Times(1).WillOnce(testing::Return(3.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 3.f)).Times(1);
		
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(2, 5)).Times(1).WillOnce(testing::Return(4.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 4.f)).Times(1);
		
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(5, 8)).Times(1).WillOnce(testing::Return(5.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 5.f)).Times(1);
	}

	for (int i = 0; i < 5; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValue, OutputValueShouldFailOnUnknownOutputRef) {
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
			{ { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "output", { { "ref", "unknown-ref" } } } } } } } } }) } }
			}) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["outputs"] = json::array({
		{ { "id", "not-it-output-1" }, { "index", 8 } },
		{ { "id", "not-it-output-2" }, { "index", 6 }, { "channel", 9 } }
	});
	
	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value/output");
}

TEST(TimeSeqProcessorValue, OutputValueShouldReadOutputVoltage) {
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
			{ { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "output", { { "index", 2 } } } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "output", { { "index", 1 }, { "channel", 15 } } } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "output", { { "ref", "output-1" } } } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "value-1" } } } } } } }) } },
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({ { { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "value-2" } } } } } } }) } }
			}) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["outputs"] = json::array({
		{ { "id", "output-1" }, { "index", 8 } },
		{ { "id", "output-2" }, { "index", 6 }, { "channel", 9 } }
	});
	json["component-pool"]["values"] = json::array({
		{ { "id", "value-1" }, { "output", { { "index", 3 }, { "channel", 6 } } } },
		{ { "id", "value-2" }, { "output", { { "ref", "output-2" } } } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getOutputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 1.f)).Times(1);
		
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getOutputPortVoltage(0, 14)).Times(1).WillOnce(testing::Return(2.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 2.f)).Times(1);
		
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getOutputPortVoltage(7, 0)).Times(1).WillOnce(testing::Return(3.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 3.f)).Times(1);
		
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getOutputPortVoltage(2, 5)).Times(1).WillOnce(testing::Return(4.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 4.f)).Times(1);
		
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getOutputPortVoltage(5, 8)).Times(1).WillOnce(testing::Return(5.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 5.f)).Times(1);
	}

	for (int i = 0; i < 5; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValue, RandValueWithUnknownLowerValueRefShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	shared_ptr<MockRandValueGenerator> mockRandValueGenerator = shared_ptr<MockRandValueGenerator>(new MockRandValueGenerator());
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr, mockRandValueGenerator);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-variable", { { "name", "output-variable" }, { "value", { { "rand", {
							{ "lower", { { "ref", "lower-value-id" } } },
							{ "upper", { { "variable", "upper-value" } } }
						} } } } } } }
					}) } }
			}) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value/rand/lower");
}

TEST(TimeSeqProcessorValue, RandValueWithInvalidLowerValueRefShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	shared_ptr<MockRandValueGenerator> mockRandValueGenerator = shared_ptr<MockRandValueGenerator>(new MockRandValueGenerator());
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr, mockRandValueGenerator);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-variable", { { "name", "output-variable" }, { "value", { { "rand", {
							{ "lower", { { "voltage", "1.f" } } },
							{ "upper", { { "variable", "upper-value" } } }
						} } } } } } }
					}) } }
			}) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageFloat, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value/rand/lower");
}

TEST(TimeSeqProcessorValue, RandValueWithUnknownUpperValueRefShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	shared_ptr<MockRandValueGenerator> mockRandValueGenerator = shared_ptr<MockRandValueGenerator>(new MockRandValueGenerator());
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr, mockRandValueGenerator);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-variable", { { "name", "output-variable" }, { "value", { { "rand", {
							{ "lower", { { "variable", "lower-value" } } },
							{ "upper", { { "ref", "upper-value-id" } } }
						} } } } } } }
					}) } }
			}) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value/rand/upper");
}

TEST(TimeSeqProcessorValue, RandValueWithInvalidUpperValueRefShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	shared_ptr<MockRandValueGenerator> mockRandValueGenerator = shared_ptr<MockRandValueGenerator>(new MockRandValueGenerator());
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr, mockRandValueGenerator);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-variable", { { "name", "output-variable" }, { "value", { { "rand", {
							{ "lower", { { "variable", "lower-value" } } },
							{ "upper", { { "voltage", "1.f" } } }
						} } } } } } }
					}) } }
			}) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageFloat, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value/rand/upper");
}

TEST(TimeSeqProcessorValue, RandValueShouldDetectCircularReferenceInLowerValue) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	shared_ptr<MockRandValueGenerator> mockRandValueGenerator = shared_ptr<MockRandValueGenerator>(new MockRandValueGenerator());
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr, mockRandValueGenerator);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "rand-value-id" }} } } } }
					}) } }
			}) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "rand-value-id" }, { "rand", {
			{ "lower", { { "ref", "rand-value-id" } } },
			{ "upper", { { "voltage", 1.f } } }
		} } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_CircularFound, "/component-pool/values/0/rand/lower");
}

TEST(TimeSeqProcessorValue, RandValueShouldDetectCircularReferenceInUpperValue) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	shared_ptr<MockRandValueGenerator> mockRandValueGenerator = shared_ptr<MockRandValueGenerator>(new MockRandValueGenerator());
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr, mockRandValueGenerator);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "rand-value-id" }} } } } }
					}) } }
			}) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "rand-value-id" }, { "rand", {
			{ "lower", { { "voltage", 1.f } } },
			{ "upper", { { "ref", "rand-value-id" } } }
		} } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_CircularFound, "/component-pool/values/0/rand/upper");
}

TEST(TimeSeqProcessorValue, RandValueShouldUseProvidedValues) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	shared_ptr<MockRandValueGenerator> mockRandValueGenerator = shared_ptr<MockRandValueGenerator>(new MockRandValueGenerator());
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr, mockRandValueGenerator);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-variable", { { "name", "output-variable" }, { "value", { { "rand", {
							{ "lower", { { "variable", "lower-value" } } },
							{ "upper", { { "variable", "upper-value" } } }
						} } } } } } }
					}) } }
			}) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockVariableHandler, getVariable("lower-value")).Times(1).WillOnce(testing::Return(1.f));
		EXPECT_CALL(mockVariableHandler, getVariable("upper-value")).Times(1).WillOnce(testing::Return(9.f));
		EXPECT_CALL(*mockRandValueGenerator.get(), generate(1.f, 9.f)).Times(1).WillOnce(testing::Return(5.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 5.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockVariableHandler, getVariable("lower-value")).Times(1).WillOnce(testing::Return(3.f));
		EXPECT_CALL(mockVariableHandler, getVariable("upper-value")).Times(1).WillOnce(testing::Return(6.f));
		EXPECT_CALL(*mockRandValueGenerator.get(), generate(3.f, 6.f)).Times(1).WillOnce(testing::Return(6.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 6.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockVariableHandler, getVariable("lower-value")).Times(1).WillOnce(testing::Return(-1.f));
		EXPECT_CALL(mockVariableHandler, getVariable("upper-value")).Times(1).WillOnce(testing::Return(5.f));
		EXPECT_CALL(*mockRandValueGenerator.get(), generate(-1.f, 5.f)).Times(1).WillOnce(testing::Return(-5.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", -5.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockVariableHandler, getVariable("lower-value")).Times(1).WillOnce(testing::Return(-5.1f));
		EXPECT_CALL(mockVariableHandler, getVariable("upper-value")).Times(1).WillOnce(testing::Return(0.1f));
		EXPECT_CALL(*mockRandValueGenerator.get(), generate(-5.1f, 0.1f)).Times(1).WillOnce(testing::Return(9.6f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 9.6f)).Times(1);

	}

	for (int i = 0; i < 4; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValue, RandValueShouldRandomizeInRange) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-variable", { { "name", "output-variable" }, { "value", { { "rand", {
							{ "lower", { { "ref", "lower-value-id" } } },
							{ "upper", { { "ref", "upper-value-id" } } }
						} } } } } } }
					}) } }
			}) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "lower-value-id" }, { "variable", "lower-value" } },
		{ { "id", "upper-value-id" }, { "variable", "upper-value" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};

	for (int j = 0; j < 5; j++) {
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(2000).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, getVariable("lower-value")).Times(2000).WillRepeatedly(testing::Return(1.f));
		EXPECT_CALL(mockVariableHandler, getVariable("upper-value")).Times(2000).WillRepeatedly(testing::Return(9.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", testing::AllOf(testing::Ge(1.f), testing::Le(9.f)))).Times(2000);
		for (int i = 0; i < 2000; i++) {
			script.second->process();
		}

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(2000).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, getVariable("lower-value")).Times(2000).WillRepeatedly(testing::Return(6.f)); // Reverse the lower/higher order tho check
		EXPECT_CALL(mockVariableHandler, getVariable("upper-value")).Times(2000).WillRepeatedly(testing::Return(3.f)); // that the range also works in reverse
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", testing::AllOf(testing::Ge(3.f), testing::Le(6.f)))).Times(2000);
		for (int i = 0; i < 2000; i++) {
			script.second->process();
		}

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(2000).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, getVariable("lower-value")).Times(2000).WillRepeatedly(testing::Return(-1.f));
		EXPECT_CALL(mockVariableHandler, getVariable("upper-value")).Times(2000).WillRepeatedly(testing::Return(5.f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", testing::AllOf(testing::Ge(-1.f), testing::Le(5.1f)))).Times(2000);
		for (int i = 0; i < 2000; i++) {
			script.second->process();
		}

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(2000).WillRepeatedly(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, getVariable("lower-value")).Times(2000).WillRepeatedly(testing::Return(0.1f));  // Reverse the lower/higher order tho check
		EXPECT_CALL(mockVariableHandler, getVariable("upper-value")).Times(2000).WillRepeatedly(testing::Return(-5.1f)); // that the range also works in reverse
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", testing::AllOf(testing::Ge(-5.1f), testing::Le(0.1f)))).Times(2000);
		for (int i = 0; i < 2000; i++) {
			script.second->process();
		}
	}
}

TEST(TimeSeqProcessorValue, RandValueShouldHandleEqualLowerAndUpper) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
					{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-variable", { { "name", "output-variable" }, { "value", { { "rand", {
							{ "lower", { { "ref", "lower-value-id" } } },
							{ "upper", { { "ref", "upper-value-id" } } }
						} } } } } } }
					}) } }
			}) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "lower-value-id" }, { "variable", "lower-value" } },
		{ { "id", "upper-value-id" }, { "variable", "upper-value" } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};

	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(100).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockVariableHandler, getVariable("lower-value")).Times(100).WillRepeatedly(testing::Return(1.f));
	EXPECT_CALL(mockVariableHandler, getVariable("upper-value")).Times(100).WillRepeatedly(testing::Return(1.f));
	EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 1.f)).Times(100);
	for (int i = 0; i < 100; i++) {
		script.second->process();
	}

}
