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
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 6.9f));
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
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 4.2f));
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
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", testing::FloatEq(-6.3f)));
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
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", testing::FloatEq(-3.2f)));
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
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", 0.f));
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
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		EXPECT_CALL(mockVariableHandler, getVariable("calc-variable")).Times(1).WillOnce(testing::Return(6.9f));
		EXPECT_CALL(mockVariableHandler, setVariable("output-variable", testing::FloatEq(-2.076)));
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
