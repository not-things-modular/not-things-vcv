#include "timeseq-processor-shared.hpp"

MATCHER_P3(IncreasingInRangeExclMatcher, start, end, last, "") {
	if (arg <= start || arg >= end) {
		*result_listener << "Encountered value " << arg << " that is not between " << start << " and " << end << " (exclusive).";
		return false;
	} else if (arg <= *last) {
		*result_listener << "Encountered value " << arg << " that is not increased compared to the previous value "<< *last << ".";
		return false;
	} else {
		*last = arg;
		return true;
	}
}

MATCHER_P3(DecreasingInRangeExclMatcher, start, end, last, "") {
	if (arg >= start || arg <= end) {
		*result_listener << "Encountered value " << arg << " that is not between " << end << " and " << start << " (exclusive).";
		return false;
	} else if (arg >= *last) {
		*result_listener << "Encountered value " << arg << " that is not decreased compared to the previous value "<< *last << ".";
		return false;
	} else {
		*last = arg;
		return true;
	}
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithRefToUnknownStartValueShouldFail) {
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
					{ "timing", "glide" },
					{ "start-value", { { "ref", "unknown-ref" } } },
					{ "end-value", { { "voltage", 1.0f } } },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/start-value");
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithInvalidStartValueShouldFail) {
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
					{ "timing", "glide" },
					{ "start-value", { { "voltage", "1.0f" } } },
					{ "end-value", { { "voltage", 1.0f } } },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageFloat, "/timelines/0/lanes/0/segments/0/actions/0/start-value");
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithRefToUnknownEndValueShouldFail) {
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
					{ "timing", "glide" },
					{ "start-value", { { "voltage", 1.0f } } },
					{ "end-value", { { "ref", "unknown-ref" } } },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/0/end-value");
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithInvalidEndValueShouldFail) {
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
					{ "timing", "glide" },
					{ "start-value", { { "voltage", 1.0f } } },
					{ "end-value", { { "voltage", "1.0f" } } },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Value_VoltageFloat, "/timelines/0/lanes/0/segments/0/actions/0/end-value");
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithOnlyStartAndEndValueShouldGlideVariable) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 10 } } }, { "actions", json::array({
				{
					{ "timing", "glide" },
					{ "start-value", { { "voltage", 0.f } } },
					{ "end-value", { { "voltage", 4.5f } } },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 10; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, (float) i / 2.f)).Times(1);
		}

		// No further calls should happen after that
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	}

	for (int i = 0; i < 11; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithOnlyStartAndEndValueShouldGlideOutput) {
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
			{ { "segments", json::array({ { { "duration", { { "samples", 10 } } }, { "actions", json::array({
				{
					{ "timing", "glide" },
					{ "start-value", { { "voltage", 0.f } } },
					{ "end-value", { { "voltage", 4.5f } } },
					{ "output", { { "index", 8 }, { "channel", 16 } } }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 10; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(7, 15, (float) i / 2.f)).Times(1);
		}

		// No further calls should happen after that
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(0);
	}

	for (int i = 0; i < 11; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithStartEndAndPositivePowEaseValueShouldGlideVariableIncreasing) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 10 } } }, { "actions", json::array({
				{
					{ "timing", "glide" },
					{ "start-value", { { "voltage", 1.f } } },
					{ "end-value", { { "voltage", 6.f } } },
					{ "ease-algorithm", "pow" },
					{ "ease-factor", .5f },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	float lastValue = 1.f;
	{
		testing::InSequence inSequence;

		// The first process should start on the exact start value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 1.f)).Times(1);

		for (int i = 1; i < 9; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, IncreasingInRangeExclMatcher(1.f, 6.f, &lastValue))).Times(1);
		}

		// The last process should end on the exact end value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 6.f)).Times(1);

		// No further calls should happen after that
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	}

	for (int i = 0; i < 11; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithStartEndAndPositivePowEaseValueShouldGlideVariableDecreasing) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 10 } } }, { "actions", json::array({
				{
					{ "timing", "glide" },
					{ "start-value", { { "voltage", 6.f } } },
					{ "end-value", { { "voltage", 1.f } } },
					{ "ease-algorithm", "pow" },
					{ "ease-factor", .5f },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	float lastValue = 6.f;
	{
		testing::InSequence inSequence;

		// The first process should start on the exact start value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 6.f)).Times(1);

		for (int i = 1; i < 9; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, DecreasingInRangeExclMatcher(6.f, 1.f, &lastValue))).Times(1);
		}

		// The last process should end on the exact end value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 1.f)).Times(1);

		// No further calls should happen after that
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	}

	for (int i = 0; i < 11; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithStartEndAndNegativePowEaseValueShouldGlideVariableIncreasing) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 10 } } }, { "actions", json::array({
				{
					{ "timing", "glide" },
					{ "start-value", { { "voltage", 1.f } } },
					{ "end-value", { { "voltage", 6.f } } },
					{ "ease-algorithm", "pow" },
					{ "ease-factor", -.5f },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	float lastValue = 1.f;
	{
		testing::InSequence inSequence;

		// The first process should start on the exact start value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 1.f)).Times(1);

		for (int i = 1; i < 9; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, IncreasingInRangeExclMatcher(1.f, 6.f, &lastValue))).Times(1);
		}

		// The last process should end on the exact end value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 6.f)).Times(1);

		// No further calls should happen after that
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	}

	for (int i = 0; i < 11; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithStartEndAndPositiveSigEaseValueShouldGlideVariableIncreasing) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 10 } } }, { "actions", json::array({
				{
					{ "timing", "glide" },
					{ "start-value", { { "voltage", 1.f } } },
					{ "end-value", { { "voltage", 6.f } } },
					{ "ease-algorithm", "sig" },
					{ "ease-factor", .5f },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	float lastValue = 1.f;
	{
		testing::InSequence inSequence;

		// The first process should start on the exact start value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 1.f)).Times(1);

		for (int i = 1; i < 9; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, IncreasingInRangeExclMatcher(1.f, 6.f, &lastValue))).Times(1);
		}

		// The last process should end on the exact end value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 6.f)).Times(1);

		// No further calls should happen after that
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	}

	for (int i = 0; i < 11; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithStartEndAndPositiveSigEaseValueShouldGlideVariableDecreasing) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 10 } } }, { "actions", json::array({
				{
					{ "timing", "glide" },
					{ "start-value", { { "voltage", 6.f } } },
					{ "end-value", { { "voltage", 1.f } } },
					{ "ease-algorithm", "sig" },
					{ "ease-factor", .5f },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	float lastValue = 6.f;
	{
		testing::InSequence inSequence;

		// The first process should start on the exact start value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 6.f)).Times(1);

		for (int i = 1; i < 9; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, DecreasingInRangeExclMatcher(6.f, 1.f, &lastValue))).Times(1);
		}

		// The last process should end on the exact end value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 1.f)).Times(1);

		// No further calls should happen after that
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	}

	for (int i = 0; i < 11; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithStartEndAndNegativeSigEaseValueShouldGlideVariableIncreasing) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 10 } } }, { "actions", json::array({
				{
					{ "timing", "glide" },
					{ "start-value", { { "voltage", 1.f } } },
					{ "end-value", { { "voltage", 6.f } } },
					{ "ease-algorithm", "sig" },
					{ "ease-factor", -.5f },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	float lastValue = 1.f;
	{
		testing::InSequence inSequence;

		// The first process should start on the exact start value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 1.f)).Times(1);

		for (int i = 1; i < 9; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, IncreasingInRangeExclMatcher(1.f, 6.f, &lastValue))).Times(1);
		}

		// The last process should end on the exact end value.
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 6.f)).Times(1);

		// No further calls should happen after that
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	}

	for (int i = 0; i < 11; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithPositiveIfConditionShouldGlideVariable) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 10 } } }, { "actions", json::array({
				{
					{ "timing", "glide" },
					{ "if", { { "eq", json::array({
						{ { "voltage", 1.f } },
						{ { "variable", "if-variable" } }
					}) } } },
					{ "start-value", { { "voltage", 0.f } } },
					{ "end-value", { { "voltage", 4.5f } } },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		std::string ifVariableName = "if-variable";

		// The first process should check the condition and set the start value
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, getVariable(ifVariableName)).Times(1).WillOnce(testing::Return(1.f));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 0.f)).Times(1);

		for (int i = 1; i < 10; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, (float) i / 2.f)).Times(1);
		}

		// No further calls should happen after that
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	}

	for (int i = 0; i < 11; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithNegativeIfConditionShouldDoNothing) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 10 } } }, { "actions", json::array({
				{
					{ "timing", "glide" },
					{ "if", { { "eq", json::array({
						{ { "voltage", 1.f } },
						{ { "variable", "if-variable" } }
					}) } } },
					{ "start-value", { { "voltage", 1.f } } },
					{ "end-value", { { "voltage", 6.f } } },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		std::string ifVariableName = "if-variable";

		// The first process should check the condition, and not execute the glide action
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, getVariable(ifVariableName)).Times(1).WillOnce(testing::Return(2.f));
		EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
		EXPECT_CALL(mockVariableHandler, getVariable).Times(0);
		EXPECT_CALL(mockTriggerHandler, getTriggers).Times(10).WillRepeatedly(testing::ReturnRef(emptyTriggers));
	}

	for (int i = 0; i < 11; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorGlideAction, GlideActionWithOneSampleDurationShouldSetEndValue) {
	testing::NiceMock<MockEventListener> mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(nullptr, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "timing", "glide" },
					{ "start-value", { { "voltage", 0.f } } },
					{ "end-value", { { "voltage", 4.5f } } },
					{ "variable", "output-variable" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockVariableHandler, setVariable(outputVariableName, 4.5f)).Times(1);
	}

	script.second->process();
}
