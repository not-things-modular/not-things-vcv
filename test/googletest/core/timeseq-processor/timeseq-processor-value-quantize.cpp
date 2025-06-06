#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorValueQuantize, VariableValueShouldQuantizeToNoteValue) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "variable", "input-variable" }, { "quantize", true } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	vector<float> values = { 0.f, -1.f, (1.f / 12), 1.f + (9.f / 12), -(1.f / 12), -2 + (7.f / 12) };
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i] + ((-0.0005) * (i + 1))));
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, testing::FloatEq(values[i]))).Times(1);
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValueQuantize, InputValueShouldQuantizeToNoteValue) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "input", { { "index", 1 } } }, { "quantize", true } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	vector<float> values = { 0.f, -1.f, (1.f / 12), 1.f + (9.f / 12), -(1.f / 12), -2 + (7.f / 12) };
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(values[i] + ((-0.0005) * (i + 1))));
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, testing::FloatEq(values[i]))).Times(1);
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValueQuantize, OutputValueShouldQuantizeToNoteValue) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "output", { { "index", 1 } } }, { "quantize", true } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	vector<float> values = { 0.f, -1.f, (1.f / 12), 1.f + (9.f / 12), -(1.f / 12), -2 + (7.f / 12) };
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, getOutputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(values[i] + ((-0.0005) * (i + 1))));
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, testing::FloatEq(values[i]))).Times(1);
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

MATCHER(IsQuantized, "") {
	float result = std::round(arg / (1.f / 12));
	float expected = result * (1.f / 12);
	return std::fabs(arg - expected) <= 0.000001;
}

TEST(TimeSeqProcessorValueQuantize, RandValueShouldQuantizeToNoteValue) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "rand", { { "lower", { { "voltage", -10.f } } }, { "upper", { { "voltage", 10.f } } } } }, { "quantize", true } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u) << json.dump();
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};

	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1000).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, IsQuantized())).Times(1000);

	for (int i = 0; i < 1000; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValueQuantize, NoteValueShouldNeedNoQuantize) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "c4" }, { "quantize", true } } } } } },
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "c3" }, { "quantize", true } } } } } },
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "c4+" }, { "quantize", true } } } } } },
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "a5" }, { "quantize", true } } } } } },
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "c4-" }, { "quantize", true } } } } } },
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "note", "g2" }, { "quantize", true } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	vector<float> values = { 0.f, -1.f, (1.f / 12), 1.f + (9.f / 12), -(1.f / 12), -2 + (7.f / 12) };
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		for (int i = 0; i < 6; i++) {
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, testing::FloatEq(values[i]))).Times(1);
		}
	}

	script.second->process();
}
