#include "timeseq-processor-shared.hpp"

#define INCREASE_BOUNDED(value, bounds) value = (value + 1) % bounds
#define DECREASE_BOUNDED(value, bounds) value = (value - 1) < 0 ? bounds - 1 : value - 1
#define INCREASE_LIMITED(value, bounds) value = (value < bounds - 2) ? value + 1 : bounds - 1
#define DECREASE_LIMITED(value) value = (value > 0) ? value - 1 : 0;

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionShouldFailOnUnknownSequence) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "move-sequence", { { "id", "unknown-ref" } } }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::MoveSequence_SequenceNotFound, "/timelines/0/lanes/0/segments/0/actions/0/move-sequence");
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionShouldFailOnNonSharedSequence) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "move-sequence", { { "id", "non-shared-sequence" } } }
				}
			}) } } }) } },
		}) } }
	});
	json["sequences"] = json::array({
		{ { "id", "non-shared-sequence" }, { "shared", false }, { "values", json::array() } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::MoveSequence_NonSharedSequence, "/timelines/0/lanes/0/segments/0/actions/0/move-sequence");
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionWithNoMoveOrPositionShouldMoveForward) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 6, 7, 8, 9, 0 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" } } } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } }
			}) } },
		}) } }
	});

	json["sequences"] = json::array({
		{ { "id", "simple-shared-sequence" }, { "shared", true }, { "values", sequence1 } },
		{ { "id", "simple-implicitly-shared-sequence" }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	int sequence1idx = 0;
	int sequence2idx = 0;
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 10; i++) {
			// No move occurred yet
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// At the start of the segment, the first sequence is moved once
			INCREASE_BOUNDED(sequence1idx, 5);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// At the end of the segment, the second sequence is moved once
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// No moves occurred
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// Both sequences are moved at the start of the segment
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++) {
			script.second->process();
		}
	}
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionWithForwardMoveShouldMoveForward) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 6, 7, 8, 9, 0 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "forward" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "forward" } } } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "forward" } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "forward" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } }
			}) } },
		}) } }
	});

	json["sequences"] = json::array({
		{ { "id", "simple-shared-sequence" }, { "shared", true }, { "values", sequence1 } },
		{ { "id", "simple-implicitly-shared-sequence" }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	int sequence1idx = 0;
	int sequence2idx = 0;
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 10; i++) {
			// No move occurred yet
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// At the start of the segment, the first sequence is moved once
			INCREASE_BOUNDED(sequence1idx, 5);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// At the end of the segment, the second sequence is moved once
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// No moves occurred
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// Both sequences are moved at the start of the segment
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++) {
			script.second->process();
		}
	}
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionWithBackwardMoveShouldMoveBackward) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 6, 7, 8, 9, 0 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "backward" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "backward" } } } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "backward" } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "backward" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } }
			}) } },
		}) } }
	});

	json["sequences"] = json::array({
		{ { "id", "simple-shared-sequence" }, { "shared", true }, { "values", sequence1 } },
		{ { "id", "simple-implicitly-shared-sequence" }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	int sequence1idx = 0;
	int sequence2idx = 0;
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 10; i++) {
			// No move occurred yet
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// At the start of the segment, the first sequence is moved once
			DECREASE_BOUNDED(sequence1idx, 5);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// At the end of the segment, the second sequence is moved once
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			DECREASE_BOUNDED(sequence2idx, 5);

			// No moves occurred
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// Both sequences are moved at the start of the segment
			DECREASE_BOUNDED(sequence1idx, 5);
			DECREASE_BOUNDED(sequence2idx, 5);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++) {
			script.second->process();
		}
	}
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionWithNoneMoveShouldNotMove) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 6, 7, 8, 9, 0 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "none" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "none" } } } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "none" } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "none" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } }
			}) } },
		}) } }
	});

	json["sequences"] = json::array({
		{ { "id", "simple-shared-sequence" }, { "shared", true }, { "values", sequence1 } },
		{ { "id", "simple-implicitly-shared-sequence" }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	int sequence1idx = 0;
	int sequence2idx = 0;
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 10; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++) {
			script.second->process();
		}
	}
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionWithRandMoveShouldMoveRandomly) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	shared_ptr<MockRandValueGenerator> mockRandValueGenerator = make_shared<MockRandValueGenerator>();
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr, mockRandValueGenerator);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 6, 7, 8, 9, 0 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "random" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "random" } } } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "random" } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "random" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } }
			}) } },
		}) } }
	});

	json["sequences"] = json::array({
		{ { "id", "simple-shared-sequence" }, { "shared", true }, { "values", sequence1 } },
		{ { "id", "simple-implicitly-shared-sequence" }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	int sequence1idx = 0;
	int sequence2idx = 0;
	int randIdx = 0;
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 10; i++) {
			// No move occurred yet
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// At the start of the segment, the first sequence is moved once
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			sequence1idx = randIdx;
			EXPECT_CALL(*mockRandValueGenerator.get(), generate(0.f, 5.f)).Times(1).WillOnce(testing::Return(randIdx));
			INCREASE_BOUNDED(randIdx, 5);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// At the end of the segment, the second sequence is moved once
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			sequence2idx = randIdx;
			EXPECT_CALL(*mockRandValueGenerator.get(), generate(0.f, 5.f)).Times(1).WillOnce(testing::Return(randIdx));
			INCREASE_BOUNDED(randIdx, 5);

			// No moves occurred
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// Both sequences are moved at the start of the segment
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			sequence1idx = randIdx;
			EXPECT_CALL(*mockRandValueGenerator.get(), generate(0.f, 5.f)).Times(1).WillOnce(testing::Return(randIdx));
			INCREASE_BOUNDED(randIdx, 5);
			sequence2idx = randIdx;
			EXPECT_CALL(*mockRandValueGenerator.get(), generate(0.f, 5.f)).Times(1).WillOnce(testing::Return(randIdx));
			INCREASE_BOUNDED(randIdx, 5);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++) {
			script.second->process();
		}
	}
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionWithRandMoveOutOfBoundsShouldReturnIntoBounds) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	shared_ptr<MockRandValueGenerator> mockRandValueGenerator = make_shared<MockRandValueGenerator>();
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr, mockRandValueGenerator);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 6, 7, 8, 9, 0 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "random" } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "random" }, { "wrap", false } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } }
			}) } },
		}) } }
	});

	json["sequences"] = json::array({
		{ { "id", "simple-shared-sequence" }, { "shared", true }, { "values", sequence1 } },
		{ { "id", "simple-implicitly-shared-sequence" }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	// Always return a position out of bounds.
	EXPECT_CALL(*mockRandValueGenerator.get(), generate(0.f, 5.f)).WillRepeatedly(testing::Return(10));

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 10; i++) {
			// No move occurred yet
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 5.f)).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 0.f)).Times(1);
		}
	}

	for (int i = 0; i < 10; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionWithPositionShouldMoveToPosition) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 6, 7, 8, 9, 0 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "position", 2 } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "position", 4 } } } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "position", 1 } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "position", 2 } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } }
			}) } },
		}) } }
	});

	json["sequences"] = json::array({
		{ { "id", "simple-shared-sequence" }, { "shared", true }, { "values", sequence1 } },
		{ { "id", "simple-implicitly-shared-sequence" }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	int sequence1idx = 0;
	int sequence2idx = 0;
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 10; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			sequence1idx = 2;
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			sequence2idx = 4;

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			sequence1idx = 1;
			sequence2idx = 2;
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++) {
			script.second->process();
		}
	}
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionWithPositionOutOfBoundsShouldLimitToBounds) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 6, 7, 8, 9, 0 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "position", 10 } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "position", 4 } } } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "position", 1 } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "position", -1 } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } }
			}) } },
		}) } }
	});

	json["sequences"] = json::array({
		{ { "id", "simple-shared-sequence" }, { "shared", true }, { "values", sequence1 } },
		{ { "id", "simple-implicitly-shared-sequence" }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	int sequence1idx = 0;
	int sequence2idx = 0;
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 10; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			sequence1idx = 4;
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			sequence2idx = 4;

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			sequence1idx = 1;
			sequence2idx = 4;
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++) {
			script.second->process();
		}
	}
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionWithNoWrapShouldStopOnBounds) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 6, 7, 8, 9, 0 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "forward" }, { "wrap", false } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "backward" }, { "wrap", false } } } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "forward" }, { "wrap", false } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "backward" }, { "wrap", false } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } }
			}) } },
		}) } }
	});

	json["sequences"] = json::array({
		{ { "id", "simple-shared-sequence" }, { "shared", true }, { "values", sequence1 } },
		{ { "id", "simple-implicitly-shared-sequence" }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	int sequence1idx = 0;
	int sequence2idx = 0;
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 10; i++) {
			// No move occurred yet
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// At the start of the segment, the first sequence is moved once
			INCREASE_LIMITED(sequence1idx, 5);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// At the end of the segment, the second sequence is moved once
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			DECREASE_LIMITED(sequence2idx);

			// No moves occurred
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			// Both sequences are moved at the start of the segment
			INCREASE_LIMITED(sequence1idx, 5);
			DECREASE_LIMITED(sequence2idx);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++) {
			script.second->process();
		}
	}
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionWithMixShouldMoveMixed) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	shared_ptr<MockRandValueGenerator> mockRandValueGenerator = make_shared<MockRandValueGenerator>();
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr, mockRandValueGenerator);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 6, 7, 8, 9, 0 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "forward" }, { "wrap", false } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "backward" }, { "wrap", false } } } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "random" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "forward" } } } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "none" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "random" } } } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "move-sequence", { { "id", "simple-shared-sequence" }, { "direction", "backward" } } } },
						{ { "move-sequence", { { "id", "simple-implicitly-shared-sequence" }, { "direction", "forward" } } } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-implicitly-shared-sequence" } } } } } }
				}) } }
			}) } },
		}) } }
	});

	json["sequences"] = json::array({
		{ { "id", "simple-shared-sequence" }, { "shared", true }, { "values", sequence1 } },
		{ { "id", "simple-implicitly-shared-sequence" }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	int sequence1idx = 0;
	int sequence2idx = 0;
	int randIdx = 0;
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 1; i++) {
			INCREASE_LIMITED(sequence1idx, 5);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			DECREASE_LIMITED(sequence2idx);

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			sequence1idx = randIdx;
			EXPECT_CALL(*mockRandValueGenerator.get(), generate(0.f, 5.f)).Times(1).WillOnce(testing::Return(randIdx));
			INCREASE_BOUNDED(randIdx, 5);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			sequence2idx = randIdx;
			EXPECT_CALL(*mockRandValueGenerator.get(), generate(0.f, 5.f)).Times(1).WillOnce(testing::Return(randIdx));
			INCREASE_BOUNDED(randIdx, 5);

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);

			DECREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 5);
			INCREASE_BOUNDED(sequence2idx, 5);
		}
	}

	for (int i = 0; i < 1; i++) {
		for (int j = 0; j < 5; j++) {
			script.second->process();
		}
	}
}

TEST(TimeSeqProcessorMoveSequence, MoveSequenceActionWithEmptySequenceShouldWork) {
	MockEventListener mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	shared_ptr<MockRandValueGenerator> mockRandValueGenerator = make_shared<MockRandValueGenerator>();
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr, mockRandValueGenerator);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "move-sequence", { { "id", "an-empty-sequence" }, { "direction", "forward" } } } },
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "an-empty-sequence" } } } } } } } }
			}) } },
			{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "move-sequence", { { "id", "an-empty-sequence" }, { "direction", "backward" } } } },
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "an-empty-sequence" } } } } } } } }
			}) } },
			{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "move-sequence", { { "id", "an-empty-sequence" }, { "direction", "none" } } } },
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "an-empty-sequence" } } } } } } } }
			}) } },
			{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "move-sequence", { { "id", "an-empty-sequence" }, { "direction", "random" } } } },
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "an-empty-sequence" } } } } } } } }
			}) } },
			{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "move-sequence", { { "id", "an-empty-sequence" }, { "position", 5 } } } },
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "an-empty-sequence" } } } } } } } }
			}) } } }) } },
		}) } }
	});
	json["sequences"] = json::array({
		{ { "id", "an-empty-sequence" }, { "values", json::array({}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	EXPECT_CALL(*mockRandValueGenerator.get(), generate(0.f, 5.f)).WillRepeatedly(testing::Return(2));

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 15; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		}
	}

	for (int i = 0; i < 15; i++) {
		script.second->process();
	}
}
