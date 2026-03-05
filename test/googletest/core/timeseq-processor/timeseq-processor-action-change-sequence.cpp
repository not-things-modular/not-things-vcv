#include "timeseq-processor-shared.hpp"

#define INCREASE_BOUNDED(value, bounds) value = (value + 1) % bounds
#define DECREASE_BOUNDED(value, bounds) value = (value - 1) < 0 ? bounds - 1 : value - 1
#define INCREASE_LIMITED(value, bounds) value = (value < bounds - 2) ? value + 1 : bounds - 1
#define DECREASE_LIMITED(value) value = (value > 0) ? value - 1 : 0;

TEST(TimeSeqProcessorChangeSequence, AddToSequenceShouldFailOnUnknownSequence) {
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
					{ "add-to-sequence", { { "id", "unknown-ref" }, { "value", 1.f } } }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::AddToSequence_SequenceNotFound, "/timelines/0/lanes/0/segments/0/actions/0/add-to-sequence");
}

TEST(TimeSeqProcessorChangeSequence, RemoveFromSequenceShouldFailOnUnknownSequence) {
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
					{ "remove-from-sequence", { { "id", "unknown-ref" }, { "position", 0 } } }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::RemoveFromSequence_SequenceNotFound, "/timelines/0/lanes/0/segments/0/actions/0/remove-from-sequence");
}

TEST(TimeSeqProcessorChangeSequence, ClearSequenceShouldFailOnUnknownSequence) {
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
					{ "clear-sequence", "unknown-ref" }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::ClearSequence_SequenceNotFound, "/timelines/0/lanes/0/segments/0/actions/0/clear-sequence");
}

TEST(TimeSeqProcessorChangeSequence, ClearSequenceActionShouldClearSequence) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 3> sequence1 = { 1, 2, 3 };
	array<int, 3> sequence2 = { 6, 7, 8 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-1" } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-2" } },
						{ { "set-value", { { "output", 3 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-3" } },
						{ { "set-value", { { "output", 4 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-4" } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "clear-sequence", "simple-shared-sequence" } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-1" } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-2" } },
						{ { "set-value", { { "output", 3 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-3" } },
						{ { "set-value", { { "output", 4 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-4" } },
				}) } },
				{ { "duration", { { "samples", 1 } } }, { "actions", json::array({
						{ { "clear-sequence", "simple-non-shared-sequence" } },
						{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-1" } },
						{ { "set-value", { { "output", 2 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-2" } },
						{ { "set-value", { { "output", 3 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-3" } },
						{ { "set-value", { { "output", 4 }, { "value", { { "sequence", "simple-shared-sequence" } } } } } },
						{ { "ref", "set-non-shared-seq-value-4" } },
				}) } }
			}) } }
		}) } }
	});

	json["sequences"] = json::array({
		{ { "id", "simple-shared-sequence" }, { "shared", true }, { "values", sequence1 } },
		{ { "id", "simple-non-shared-sequence" }, { "shared", false }, { "values", sequence2 } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["actions"] = json::array({
		{ { "id", "set-non-shared-seq-value-1" }, { "set-value", { { "output", 5 }, { "value", { { "sequence", "simple-non-shared-sequence" } } } } } },
		{ { "id", "set-non-shared-seq-value-2" }, { "set-value", { { "output", 6 }, { "value", { { "sequence", "simple-non-shared-sequence" } } } } } },
		{ { "id", "set-non-shared-seq-value-3" }, { "set-value", { { "output", 7 }, { "value", { { "sequence", "simple-non-shared-sequence" } } } } } },
		{ { "id", "set-non-shared-seq-value-4" }, { "set-value", { { "output", 8 }, { "value", { { "sequence", "simple-non-shared-sequence" } } } } } },
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	int sequence1idx = 0;
	int sequence2idx = 0;
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// First four sequence usages with the original sequence contents
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		for (int i = 0; i < 4; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(i, 0, sequence1[sequence1idx])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(i + 4, 0, sequence2[sequence2idx])).Times(1);
			INCREASE_BOUNDED(sequence1idx, 3); // Only the first sequence is shared, so the second (non-shared) sequence never moves
		}

		// Then four sequence usages where the first sequence was cleared
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		for (int i = 0; i < 4; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(i, 0, 0.f)).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(i + 4, 0, sequence2[sequence2idx])).Times(1);
			// Only the first sequence is shared, so the second (non-shared) sequence never moves
		}

		// Then four sequence usages where the seconds sequence was also cleared
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
		for (int i = 0; i < 4; i++) {
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(i, 0, 0.f)).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(i + 4, 0, 0.f)).Times(1);
		}
	}

	for (int i = 0; i < 3; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorChangeSequence, RemoveAddAndClearActionsShouldWork) {
	MockEventListener mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 6, 7, 8, 9, 0 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			// The first lane loops over the sequences
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-non-shared-sequence" } } } } } } } },
				{ { "set-value", { { "output", 2 }, { "value", { { "sequence", { { "id", "a-shared-sequence" } } } } } } } }
			}) } } }) } },
			// The second lane performs actions on the sequences at the end of each segment
			{ { "segments", json::array({
				// Allow one loop of both sequences to complete
				{ { "duration", { { "samples", 5 } } }, { "actions", json::array({
					// Add an item to the second sequence without position (i.e. at the end)
					{ { "timing", "end" }, { "add-to-sequence", { { "id", "a-shared-sequence" }, { "value", 6.5f } } } },
				}) } },
				// Allow another loop of both sequences to complete plus one additional step for the new item in the second sequence
				{ { "duration", { { "samples", 6 } } }, { "actions", json::array({
					// Add an item to the start of the first sequence
					{ { "timing", "end" }, { "add-to-sequence", { { "id", "a-non-shared-sequence" }, { "value", 2.5f }, { "position", 0 } } } },
				}) } },
				// Allow another loop of both sequences so they end up at the first position again
				{ { "duration", { { "samples", 5 } } }, { "actions", json::array({
					// Add another item to the start of the first sequence
					{ { "timing", "end" }, { "add-to-sequence", { { "id", "a-non-shared-sequence" }, { "value", 3.5f }, { "position", 0 } } } },
					// Remove the first item of the second sequence
					{ { "timing", "end" }, { "remove-from-sequence", { { "id", "a-shared-sequence" }, { "position", 0 } } } },
				}) } },
				// Allow another two steps to complete to check the previous changes
				{ { "duration", { { "samples", 2 } } }, { "actions", json::array({
					// Remove the third item from the first sequence
					{ { "timing", "end" }, { "remove-from-sequence", { { "id", "a-non-shared-sequence" }, { "position", 2 } } } },
					// Clear the second sequence
					{ { "timing", "end" }, { "clear-sequence", "a-shared-sequence" } },
				}) } },
				// Allow another two steps to complete to check the previous changes
				{ { "duration", { { "samples", 2 } } }, { "actions", json::array({
					// Clear the first sequence
					{ { "timing", "end" }, { "clear-sequence", "a-non-shared-sequence" } },
					// Add new items to the second sequence
					{ { "timing", "end" }, { "add-to-sequence", { { "id", "a-shared-sequence" }, { "value", 1.f } } } },
					{ { "timing", "end" }, { "add-to-sequence", { { "id", "a-shared-sequence" }, { "value", 2.f }, { "position", 10 } } } }, // Add it out of bounds, which should add it to the end
				}) } },
				// Allow another three steps to complete so the new second sequence can loop once
				{ { "duration", { { "samples", 3 } } }, { "actions", json::array({
					// Add new items to the first sequence
					{ { "timing", "end" }, { "add-to-sequence", { { "id", "a-non-shared-sequence" }, { "value", 3.f } } } },
					{ { "timing", "end" }, { "add-to-sequence", { { "id", "a-non-shared-sequence" }, { "value", 4.f } } } },
				}) } },
				// Wait for two steps to check the new changes
				{ { "duration", { { "samples", 2 } } }, { "actions", json::array({
					// Remove items using negative positions (i.e. from the end)
					{ { "timing", "end" }, { "remove-from-sequence", { { "id", "a-shared-sequence" }, { "position", -1 } } } },
					{ { "timing", "end" }, { "remove-from-sequence", { { "id", "a-non-shared-sequence" }, { "position", -2 } } } },
				}) } },
			}) } }
		}) } }
	});
	json["sequences"] = json::array({
		{ { "id", "a-non-shared-sequence" }, { "shared", false }, { "values", sequence1 } },
		{ { "id", "a-shared-sequence" }, { "shared", true }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// First a normal sequence loop, then a sequence loop over the original elements again (since those haven't been removed yet)
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 5; i++) {
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[i])).Times(1);
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[i])).Times(1);
			}
		}

		// In the next step, the second sequence will have an additional value, while the first sequence will loop again
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 1)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 6.5f)).Times(1);

		// The first sequence will have an item added to the front, so the existing items will have shifted while both sequences are now on their second position
		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[i])).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[i])).Times(1);
		}

		// The first sequence will have two new items now, so check those. The second sequence will have its first two items removed
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 3.5f)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 6.5f)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 2.5f)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 7.f)).Times(1);

		// The first sequence will have an item removed now, and the second sequence is empty
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 2)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 0)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 3)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 0)).Times(1);

		// The first sequence will be cleared now, and the second has items again
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 1)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 2)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 1)).Times(1);

		// The first sequence will also have items now
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 3)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 2)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 4)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 1)).Times(1);

		// Check the final removes
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 3)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 1)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 3)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 1)).Times(1);
	}

	for (int i = 0; i < 27; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorChangeSequence, AddToSequenceAsConstantVoltageShouldRetrieveVariableOnce) {
	MockEventListener mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	json["timelines"] = json::array({
		{ { "lanes", json::array({
			// The first lane adds the variables to the sequences
			{ { "loop", false }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "add-to-sequence", { { "id", "a-non-shared-sequence" }, { "value", { { "variable", "variable-1" } } }, { "as-constant-voltage", true } } } },
				{ { "add-to-sequence", { { "id", "a-non-shared-sequence" }, { "value", { { "variable", "variable-2" } } }, { "as-constant-voltage", true } } } },
				{ { "add-to-sequence", { { "id", "a-shared-sequence" }, { "value", { { "variable", "variable-3" } } }, { "as-constant-voltage", true } } } },
				{ { "add-to-sequence", { { "id", "a-shared-sequence" }, { "value", { { "variable", "variable-4" } } }, { "as-constant-voltage", true } } } },
			}) } } }) } },
			// The first lane loops over the sequences
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-non-shared-sequence" } } } } } } } },
				{ { "set-value", { { "output", 2 }, { "value", { { "sequence", { { "id", "a-shared-sequence" } } } } } } } }
			}) } } }) } },
		}) } }
	});
	json["sequences"] = json::array({
		{ { "id", "a-non-shared-sequence" }, { "shared", false }, { "values", json::array({}) } },
		{ { "id", "a-shared-sequence" }, { "shared", true }, { "values", json::array({}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// Loop multiple times over the two variables in the sequence
		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));

			// The variables should be retrieved only when the first lane is executed to add them to the sequence
			if (i == 0) {
				string variableName = "variable-1";
				EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(1.f));
				variableName = "variable-2";
				EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(2.f));
				variableName = "variable-3";
				EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(3.f));
				variableName = "variable-4";
				EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(4.f));
			}

			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 1.f)).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 3.f)).Times(1);
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 2.f)).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 4.f)).Times(1);
		}
	}

	for (int i = 0; i < 10; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorChangeSequence, AddToSequenceAsNonConstantVoltageShouldRetrieveVariableOnce) {
	MockEventListener mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	json["timelines"] = json::array({
		{ { "lanes", json::array({
			// The first lane adds the variables to the sequences
			{ { "loop", false }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "add-to-sequence", { { "id", "a-non-shared-sequence" }, { "value", { { "variable", "variable-1" } } }, { "as-constant-voltage", false } } } },
				{ { "add-to-sequence", { { "id", "a-non-shared-sequence" }, { "value", { { "variable", "variable-2" } } } } } }, // as-constant-voltage should default to false
				{ { "add-to-sequence", { { "id", "a-shared-sequence" }, { "value", { { "variable", "variable-3" } } }, { "as-constant-voltage", false } } } },
				{ { "add-to-sequence", { { "id", "a-shared-sequence" }, { "value", { { "variable", "variable-4" } } } } } } // as-constant-voltage should default to false
			}) } } }) } },
			// The second lane loops over the sequences
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-non-shared-sequence" } } } } } } } },
				{ { "set-value", { { "output", 2 }, { "value", { { "sequence", { { "id", "a-shared-sequence" } } } } } } } }
			}) } } }) } },
		}) } }
	});
	json["sequences"] = json::array({
		{ { "id", "a-non-shared-sequence" }, { "shared", false }, { "values", json::array({}) } },
		{ { "id", "a-shared-sequence" }, { "shared", true }, { "values", json::array({}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	float variable = 0.f;
	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// Loop multiple times over the two variables in the sequence
		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));

			string variableName = "variable-1";
			EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(variable));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, variable)).Times(1);
			variable += .25f;
			variableName = "variable-3";
			EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(variable));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, variable)).Times(1);
			variable += .25f;

			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));

			variableName = "variable-2";
			EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(variable));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, variable)).Times(1);
			variable += .25f;
			variableName = "variable-4";
			EXPECT_CALL(mockVariableHandler, getVariable(variableName)).Times(1).WillOnce(testing::Return(variable));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, variable)).Times(1);
			variable += .25f;
		}
	}

	for (int i = 0; i < 10; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorChangeSequence, OutOfBoundsPositionAfterRemoveFromSequenceShouldBeHandled) {
	MockEventListener mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	json["timelines"] = json::array({
		{ { "lanes", json::array({
			// The first lane removes 3 items after 4 steps were completed
			{ { "loop", false }, { "segments", json::array({ { { "duration", { { "samples", 5 } } }, { "actions", json::array({
				{ { "timing", "end"}, { "remove-from-sequence", { { "id", "a-non-shared-sequence" }, { "position", -1 } } } },
				{ { "timing", "end"}, { "remove-from-sequence", { { "id", "a-non-shared-sequence" }, { "position", -1 } } } },
				{ { "timing", "end"}, { "remove-from-sequence", { { "id", "a-non-shared-sequence" }, { "position", -1 } } } },
				{ { "timing", "end"}, { "remove-from-sequence", { { "id", "a-shared-sequence" }, { "position", -1 } } } },
				{ { "timing", "end"}, { "remove-from-sequence", { { "id", "a-shared-sequence" }, { "position", -1 } } } },
				{ { "timing", "end"}, { "remove-from-sequence", { { "id", "a-shared-sequence" }, { "position", -1 } } } }
			}) } } }) } },
			// The second lane loops over the sequences
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-non-shared-sequence" } } } } } } } },
				{ { "set-value", { { "output", 2 }, { "value", { { "sequence", { { "id", "a-shared-sequence" } } } } } } } }
			}) } } }) } },
		}) } }
	});
	json["sequences"] = json::array({
		{ { "id", "a-non-shared-sequence" }, { "shared", false }, { "values", json::array({ 1, 2, 3, 4, 5 }) } },
		{ { "id", "a-shared-sequence" }, { "shared", true }, { "values", json::array({ 9, 8, 7, 6, 5 }) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		// The first four steps go over the original sequence values
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 1)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 9)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 2)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 8)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 3)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 7)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 4)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 6)).Times(1);

		// After that, the sequence will have been reduced, and the out-of-bounds position should move to the last items of the sequence and loop after that
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 2)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 8)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 1)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 9)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 2)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 8)).Times(1);
		EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 1)).Times(1);
		EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 9)).Times(1);
	}

	for (int i = 0; i < 8; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorChangeSequence, RemoveFromSequenceShouldDoNothingOnOutOfBoundsPosition) {
	MockEventListener mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	array<int, 5> sequence1 = { 1, 2, 3, 4, 5 };
	array<int, 5> sequence2 = { 9, 8, 7, 6, 5 };
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			// Perform out-of-bounds removes after the first loop of the sequences
			{ { "loop", false }, { "segments", json::array({ { { "duration", { { "samples", 6 } } }, { "actions", json::array({
				{ { "timing", "end"}, { "remove-from-sequence", { { "id", "a-non-shared-sequence" }, { "position", 10 } } } },
				{ { "timing", "end"}, { "remove-from-sequence", { { "id", "a-non-shared-sequence" }, { "position", 5 } } } },
				{ { "timing", "end"}, { "remove-from-sequence", { { "id", "a-shared-sequence" }, { "position", 10 } } } },
				{ { "timing", "end"}, { "remove-from-sequence", { { "id", "a-shared-sequence" }, { "position", 5 } } } },
			}) } } }) } },
			// The second lane loops over the sequences
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-non-shared-sequence" } } } } } } } },
				{ { "set-value", { { "output", 2 }, { "value", { { "sequence", { { "id", "a-shared-sequence" } } } } } } } }
			}) } } }) } },
		}) } }
	});
	json["sequences"] = json::array({
		{ { "id", "a-non-shared-sequence" }, { "shared", false }, { "values", sequence1 } },
		{ { "id", "a-shared-sequence" }, { "shared", true }, { "values", sequence2 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 5; i++) {
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, sequence1[i])).Times(1);
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, sequence2[i])).Times(1);
			}
		}
	}

	for (int i = 0; i < 10; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorChangeSequence, RemoveFromSequenceShouldHandleRemoveFromEndOnEmptySequence) {
	MockEventListener mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);

	json["timelines"] = json::array({
		{ { "lanes", json::array({
			// Perform out-of-bounds removes after the first loop of the sequences
			{ { "loop", false }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "remove-from-sequence", { { "id", "a-non-shared-sequence" }, { "position", -1 } } } },
				{ { "remove-from-sequence", { { "id", "a-shared-sequence" }, { "position", -1 } } } },
			}) } } }) } },
			// The second lane loops over the sequences
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-non-shared-sequence" } } } } } } } },
				{ { "set-value", { { "output", 2 }, { "value", { { "sequence", { { "id", "a-shared-sequence" } } } } } } } }
			}) } } }) } },
		}) } }
	});
	json["sequences"] = json::array({
		{ { "id", "a-non-shared-sequence" }, { "shared", false }, { "values", json::array({}) } },
		{ { "id", "a-shared-sequence" }, { "shared", true }, { "values", json::array({}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0)).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, 0)).Times(1);
		}
	}

	for (int i = 0; i < 5; i++) {
		script.second->process();
	}
}
