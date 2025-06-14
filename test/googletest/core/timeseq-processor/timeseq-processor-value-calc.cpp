#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorValueCalc, ValueWithCalcRefToUnknownCalcShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 1.f }, { "calc", json::array({
					{ { "ref", "calc-ref" } }
				}) } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value/calc/0");
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcWithRefToUnknownValueShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 1.f }, { "calc", json::array({
					{ { "add", { { "ref", "unknown-value" } } } }
				}) } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value/calc/0/add");
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcInvalidValueShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 1.f }, { "calc", json::array({
					{ { "add", { { "voltage", "1.f" } } } }
				}) } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageFloat, "/timelines/0/lanes/0/segments/0/actions/0/set-variable/value/calc/0/add");
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldAddValue) {
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
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 4.3f }, { "calc", json::array({
					{ { "add", { { "voltage", 2.6f } } } }
				}) } } } } } }
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
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 6.9f));
	}

	script.second->process();
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldSubtractValue) {
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
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 8.f }, { "calc", json::array({
					{ { "sub", { { "voltage", 3.8f } } } }
				}) } } } } } }
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
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 4.2f));
	}

	script.second->process();
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldMultiplyValue) {
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
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 4.2f }, { "calc", json::array({
					{ { "mult", { { "voltage", -1.5f } } } }
				}) } } } } } }
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
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, testing::FloatEq(-6.3f)));
	}

	script.second->process();
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldDivideValueByNonZero) {
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
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 7.04f }, { "calc", json::array({
					{ { "div", { { "voltage", -2.2f } } } }
				}) } } } } } }
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
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, testing::FloatEq(-3.2f)));
	}

	script.second->process();
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldDivideValueByZero) {
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
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 7.04f }, { "calc", json::array({
					{ { "div", { { "voltage", 0.f } } } }
				}) } } } } } }
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
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 0.f));
	}

	script.second->process();
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcsShouldExecuteCalcsInOrder) {
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
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 7.04f }, { "calc", json::array({
					{ { "add", { { "voltage", 2.0f } } } },
					{ { "div", { { "voltage", 2.f } } } },
					{ { "ref", "calc-id-2" } },
					{ { "ref", "calc-id-1" } },
					{ { "add", { { "variable", "calc-variable" } } } }
				}) } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["calcs"] = json::array({
		{ { "id", "calc-id-1" }, { "sub", { { "voltage", 3.1f } } } },
		{ { "id", "calc-id-2" }, { "mult", { { "voltage", -1.3f } } } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		std::string calcVariableName = "calc-variable";
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockVariableHandler, getVariable(calcVariableName)).Times(1).WillOnce(testing::Return(6.9f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, testing::FloatEq(-2.076)));
	}

	script.second->process();
}

TEST(TimeSeqProcessorValueCalc, ValueWithCircularCalcRefShouldFail) {
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
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "voltage", 7.04f }, { "calc", json::array({
					{ { "ref", "calc-id-1" } },
				}) } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["calcs"] = json::array({
		{ { "id", "calc-id-1" }, { "sub", { { "voltage", 3.1f }, { "calc", json::array({ { { "ref", "calc-id-1" } } }) } } } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_CircularFound, "/component-pool/calcs/0/sub/calc/0");
	EXPECT_NE(validationErrors[0].message.find("'calc-id-1'"), std::string::npos);
}

TEST(TimeSeqProcessorValueCalc, ValueWithCircularValueRefShouldFail) {
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
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "ref", "value-1" } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["values"] = json::array({
		{ { "id", "value-1" }, { "voltage", 1.f }, { "calc", json::array({ { { "ref", "calc-id-1" } } }) } },
		{ { "id", "value-2" }, { "voltage", 1.f }, { "calc", json::array({ { { "ref", "calc-id-2" } } }) } }
	});
	json["component-pool"]["calcs"] = json::array({
		{ { "id", "calc-id-1" }, { "sub", { { "ref", "value-2" } } } },
		{ { "id", "calc-id-2" }, { "sub", { { "ref", "value-1" } } } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_CircularFound, "/component-pool/calcs/1/sub");
	EXPECT_NE(validationErrors[0].message.find("'value-1'"), std::string::npos);
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldDetermineMaxAndMinValue) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "max-variable" }, { "value", { { "input", 1 }, { "calc", json::array({
					{ { "max", { { "input", 2 } } } }
				}) } } } } } },
				{ { "set-variable", { { "name", "min-variable" }, { "value", { { "input", 1 }, { "calc", json::array({
					{ { "min", { { "input", 2 } } } }
				}) } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	string minVariableName = "min-variable";
	string maxVariableName = "max-variable";

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-1.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.f));
		EXPECT_CALL(mockVariableHandler, setVariable(maxVariableName, 1.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-1.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.f));
		EXPECT_CALL(mockVariableHandler, setVariable(minVariableName, -1.f));

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(maxVariableName, 1.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(minVariableName, 0.f));

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(maxVariableName, 0.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(minVariableName, 0.f));

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-2.3f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(-2.4f));
		EXPECT_CALL(mockVariableHandler, setVariable(maxVariableName, -2.3f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-2.3f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(-2.4f));
		EXPECT_CALL(mockVariableHandler, setVariable(minVariableName, -2.4));

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(6.9f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(6.9f));
		EXPECT_CALL(mockVariableHandler, setVariable(maxVariableName, 6.9f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(6.9f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(6.9f));
		EXPECT_CALL(mockVariableHandler, setVariable(minVariableName, 6.9f));

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-6.9f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(-6.9f));
		EXPECT_CALL(mockVariableHandler, setVariable(maxVariableName, -6.9f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-6.9f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(-6.9f));
		EXPECT_CALL(mockVariableHandler, setVariable(minVariableName, -6.9f));

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(3.3f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(3.4f));
		EXPECT_CALL(mockVariableHandler, setVariable(maxVariableName, 3.4f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(3.3f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(3.4f));
		EXPECT_CALL(mockVariableHandler, setVariable(minVariableName, 3.3f));
	}

	for (int i = 0; i < 7; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldDetermineRemainderValue) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "max-variable" }, { "value", { { "input", 1 }, { "calc", json::array({
					{ { "remain", { { "input", 2 } } } }
				}) } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	string minVariableName = "min-variable";
	string maxVariableName = "max-variable";

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.23f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.f));
		EXPECT_CALL(mockVariableHandler, setVariable(maxVariableName, testing::FloatNear(.23f, 0.0001f))).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.23f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(2.f));
		EXPECT_CALL(mockVariableHandler, setVariable(maxVariableName, testing::FloatNear(1.23f, 0.0001f))).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-5.94f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.23f));
		EXPECT_CALL(mockVariableHandler, setVariable(maxVariableName, testing::FloatNear(-1.02f, 0.0001f))).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-4.20f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(maxVariableName, 0.f)).Times(1);
	}

	for (int i = 0; i < 4; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldTruncValue) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "input", 1 }, { "calc", json::array({
					{ { "trunc", true } }
				}) } } } } } }
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
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.23f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 1.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.0f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 1.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-5.94f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, -5.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-4.f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, -4.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(0.1f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 0.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 0.f)).Times(1);
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldFracValue) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "input", 1 }, { "calc", json::array({
					{ { "frac", true } }
				}) } } } } } }
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
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.23f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, testing::FloatNear(.23f, 0.0001f))).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.0f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 0.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-5.94f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, testing::FloatNear(-.94f, 0.0001f))).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-4.f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 0.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(0.1f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, testing::FloatNear(0.1f, 0.0001f))).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 0.f)).Times(1);
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldRoundValue) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "up-variable" }, { "value", { { "input", 1 }, { "calc", json::array({
					{ { "round", "up" } }
				}) } } } } } },
				{ { "set-variable", { { "name", "down-variable" }, { "value", { { "input", 1 }, { "calc", json::array({
					{ { "round", "down" } }
				}) } } } } } },
				{ { "set-variable", { { "name", "near-variable" }, { "value", { { "input", 1 }, { "calc", json::array({
					{ { "round", "near" } }
				}) } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	string upVariableName = "up-variable";
	string downVariableName = "down-variable";
	string nearVariableName = "near-variable";

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.23f));
		EXPECT_CALL(mockVariableHandler, setVariable(upVariableName, 2.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.23f));
		EXPECT_CALL(mockVariableHandler, setVariable(downVariableName, 1.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.23f));
		EXPECT_CALL(mockVariableHandler, setVariable(nearVariableName, 1.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.51f));
		EXPECT_CALL(mockVariableHandler, setVariable(upVariableName, 2.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.51f));
		EXPECT_CALL(mockVariableHandler, setVariable(downVariableName, 1.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.51f));
		EXPECT_CALL(mockVariableHandler, setVariable(nearVariableName, 2.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.5f));
		EXPECT_CALL(mockVariableHandler, setVariable(upVariableName, 2.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.5f));
		EXPECT_CALL(mockVariableHandler, setVariable(downVariableName, 1.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.5f));
		EXPECT_CALL(mockVariableHandler, setVariable(nearVariableName, 2.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(3.f));
		EXPECT_CALL(mockVariableHandler, setVariable(upVariableName, 3.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(3.f));
		EXPECT_CALL(mockVariableHandler, setVariable(downVariableName, 3.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(3.f));
		EXPECT_CALL(mockVariableHandler, setVariable(nearVariableName, 3.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(upVariableName, 0.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(downVariableName, 0.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(nearVariableName, 0.f)).Times(1);
	}

	for (int i = 0; i < 5; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldSignValue) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "pos-variable" }, { "value", { { "input", 1 }, { "calc", json::array({
					{ { "sign", "pos" } }
				}) } } } } } },
				{ { "set-variable", { { "name", "neg-variable" }, { "value", { { "input", 1 }, { "calc", json::array({
					{ { "sign", "neg" } }
				}) } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	string posVariableName = "pos-variable";
	string negVariableName = "neg-variable";

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.23f));
		EXPECT_CALL(mockVariableHandler, setVariable(posVariableName, 1.23f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.23f));
		EXPECT_CALL(mockVariableHandler, setVariable(negVariableName, -1.23f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-1.23f));
		EXPECT_CALL(mockVariableHandler, setVariable(posVariableName, 1.23f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-1.23f));
		EXPECT_CALL(mockVariableHandler, setVariable(negVariableName, -1.23f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(2.f));
		EXPECT_CALL(mockVariableHandler, setVariable(posVariableName, 2.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(2.f));
		EXPECT_CALL(mockVariableHandler, setVariable(negVariableName, -2.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-2.f));
		EXPECT_CALL(mockVariableHandler, setVariable(posVariableName, 2.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-2.f));
		EXPECT_CALL(mockVariableHandler, setVariable(negVariableName, -2.f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(posVariableName, 0.f)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockVariableHandler, setVariable(negVariableName, 0.f)).Times(1);
	}

	for (int i = 0; i < 5; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorValueCalc, ValueWithCalcShouldQuantizeValueToSingleNoteTuning) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_1_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-variable", { { "name", "output-variable" }, { "value", { { "input", 1 }, { "calc", json::array({
					{ { "quantize", "single-note-tuning" } }
				}) } } } } } }
			}) } } }) } }
		}) } }
	});
	json["component-pool"] = {
		{ "tunings", json::array({
			{
				{ "id", "single-note-tuning" },
				{ "notes", json::array({ .23f}) }
			}
		})}
	};

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.23f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 1.23f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(1.f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 1.23f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(.73001f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 1.23f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(.72999f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, .23f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(4.99f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 5.23f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-1.77f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, -1.77f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-1.f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, -.77f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-1.27001f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, -1.77f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-1.26999f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, -.77f)).Times(1);

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-4.99f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, -4.77f)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(0, 0)).Times(1).WillOnce(testing::Return(-5.275f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, -5.77f)).Times(1);
	}

	for (int i = 0; i < 11; i++) {
		script.second->process();
	}
}
