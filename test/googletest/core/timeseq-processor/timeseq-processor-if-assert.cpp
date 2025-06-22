#include "timeseq-processor-shared.hpp"
#include "string.hpp"

std::string formatAssert(float value1, float value2, std::string operatorName) {
	std::ostringstream oss;

	oss.precision(10);
	oss << "(" << value1 << " " << operatorName << " " << value2 << ")";
	return oss.str();
}

TEST(TimeSeqProcessorIfAssert, ActionWithEqIfShouldCheckIfResult) {
	MockTriggerHandler mockTriggerHandler;
	MockAssertListener mockAssertListener;
	MockEventListener mockEventListener;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", false },
						{ "expect", { { "eq", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id" } }
						}) } } }
					} }
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
			if (i % 2 != 1) {
				string name = "the-assert";
				string message = formatAssert(1.f, values[i], "eq");
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i])); // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockAssertListener, assertFailed(name, message, false)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIfAssert, ActionWithEqIfShouldCheckIfResultWithTolerance) {
	MockTriggerHandler mockTriggerHandler;
	MockAssertListener mockAssertListener;
	MockEventListener mockEventListener;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", false },
						{ "expect", { { "tolerance", .0001f }, { "eq", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id" } }
						}) } } }
					} }
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
			if ((i % 2 != 1) && (i != 0) && (i != 4)) {
				string name = "the-assert";
				string message = formatAssert(1.f, values[i], "eq");
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i]));  // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockAssertListener, assertFailed(name, message, false)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIfAssert, ActionWithNeIfShouldCheckIfResultWithTolerance) {
	MockTriggerHandler mockTriggerHandler;
	MockAssertListener mockAssertListener;
	MockEventListener mockEventListener;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", false },
						{ "expect", { { "tolerance", .0001f }, { "ne", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id" } }
						}) } } }
					} }
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
			if (i != 2) {
				string name = "the-assert";
				string message = formatAssert(1.f, values[i], "ne");
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i])); // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockAssertListener, assertFailed(name, message, false)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIfAssert, ActionWithNeIfShouldCheckIfResult) {
	MockTriggerHandler mockTriggerHandler;
	MockAssertListener mockAssertListener;
	MockEventListener mockEventListener;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", false },
						{ "expect", { { "ne", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id" } }
						}) } } }
					} }
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
			if (i % 2 != 0) {
				string name = "the-assert";
				string message = formatAssert(1.f, values[i], "ne");
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i])); // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockAssertListener, assertFailed(name, message, false)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIfAssert, ActionWithGtShouldCheckIfResult) {
	MockTriggerHandler mockTriggerHandler;
	MockAssertListener mockAssertListener;
	MockEventListener mockEventListener;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", false },
						{ "expect", { { "gt", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id" } }
						}) } } }
					} }
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
			if (i != 0 && i != 2 && i != 4) {
				string name = "the-assert";
				string message = formatAssert(1.f, values[i], "gt");
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i])); // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockAssertListener, assertFailed(name, message, false)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIfAssert, ActionWithGteShouldCheckIfResult) {
	MockTriggerHandler mockTriggerHandler;
	MockAssertListener mockAssertListener;
	MockEventListener mockEventListener;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", false },
						{ "expect", { { "gte", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id" } }
						}) } } }
					} }
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
				string name = "the-assert";
				string message = formatAssert(1.f, values[i], "gte");
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i])); // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockAssertListener, assertFailed(name, message, false)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIfAssert, ActionWithLtShouldCheckIfResult) {
	MockTriggerHandler mockTriggerHandler;
	MockAssertListener mockAssertListener;
	MockEventListener mockEventListener;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", false },
						{ "expect", { { "lt", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id" } }
						}) } } }
					} }
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
			if (i != 3 && i != 5) {
				string name = "the-assert";
				string message = formatAssert(1.f, values[i], "lt");
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i])); // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockAssertListener, assertFailed(name, message, false)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIfAssert, ActionWithLteShouldCheckIfResult) {
	MockTriggerHandler mockTriggerHandler;
	MockAssertListener mockAssertListener;
	MockEventListener mockEventListener;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", false },
						{ "expect", { { "lte", json::array({
							{ { "voltage", 1.f } },
							{ { "ref", "variable-value-id" } }
						}) } } }
					} }
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
			if (i != 1 && i != 3 && i != 5) {
				string name = "the-assert";
				string message = formatAssert(1.f, values[i], "lte");
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariableName)).Times(1).WillOnce(testing::Return(values[i])); // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockAssertListener, assertFailed(name, message, false)).Times(1);
			}
		}
	}

	for (int i = 0; i < 6; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIfAssert, ActionWithAndShouldCheckIfResult) {
	MockTriggerHandler mockTriggerHandler;
	MockAssertListener mockAssertListener;
	MockEventListener mockEventListener;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", false },
						{ "expect", { { "and", json::array({
							{ { "eq", json::array({
								{ { "voltage", 1.f } },
								{ { "ref", "variable-value-id-1" } }
							}) } },
							{ { "eq", json::array({
								{ { "voltage", 1.f } },
								{ { "ref", "variable-value-id-2" } }
							}) } }
						}) } } }
					} }
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
			if (1.f == values1[i]) {
				// The second value is only retrieved in the initial expectation check if the first one matched.
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariable2)).Times(1).WillOnce(testing::Return(values2[i]));
			}
			if (i != 3) {
				string name = "the-assert";
				string message = std::string("(") + formatAssert(1.f, values1[i], "eq") + " and " + formatAssert(1.f, values2[i], "eq") + ")";
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariable1)).Times(1).WillOnce(testing::Return(values1[i])); // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariable2)).Times(1).WillOnce(testing::Return(values2[i])); // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockAssertListener, assertFailed(name, message, false)).Times(1);
			}
		}
	}

	for (int i = 0; i < 4; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorIfAssert, ActionWithOrShouldCheckIfResult) {
	MockTriggerHandler mockTriggerHandler;
	MockAssertListener mockAssertListener;
	MockEventListener mockEventListener;
	testing::NiceMock<MockSampleRateReader> mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", false },
						{ "expect", { { "or", json::array({
							{ { "eq", json::array({
								{ { "voltage", 1.f } },
								{ { "ref", "variable-value-id-1" } }
							}) } },
							{ { "eq", json::array({
								{ { "voltage", 1.f } },
								{ { "ref", "variable-value-id-2" } }
							}) } }
						}) } } }
					} }
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
			if (1.f != values1[i]) {
				// The second variable is only retrieved in the original assertion check if the first one didn't match
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariable2)).Times(1).WillOnce(testing::Return(values2[i]));
			}
			if (i == 0) {
				string name = "the-assert";
				string message = "((1 eq 0) or (1 eq 0))";
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariable1)).Times(1).WillOnce(testing::Return(values1[i])); // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockVariableHandler, getVariable(inputVariable2)).Times(1).WillOnce(testing::Return(values2[i])); // The second getVariable call is to construct the assertion message
				EXPECT_CALL(mockAssertListener, assertFailed(name, message, false)).Times(1);
			}
		}
	}

	for (int i = 0; i < 4; i++) {
		script.second->process();
	}
}
