#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorSequence, ScriptWithSequenceRefButNoComponentPoolShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "a-sequence" } } } } } }
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SequenceValue_SequenceNotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-value/value/sequence");
}

TEST(TimeSeqProcessorSequence, ScriptWithSequenceRefButNoSequencePoolShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "a-sequence" } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SequenceValue_SequenceNotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-value/value/sequence");
}

TEST(TimeSeqProcessorSequence, ScriptWithSequenceRefToUnknownSequenceShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "a-sequence" } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array();

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::SequenceValue_SequenceNotFound, "/timelines/0/lanes/0/segments/0/actions/0/set-value/value/sequence");
}

TEST(TimeSeqProcessorSequence, ScriptWithSequenceRefToInvalidSequenceShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "a-sequence" } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array({
		{ { "id", "a-sequence" }, { "values", 5 } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Sequence_ValuesArray, "/component-pool/sequences/0");
}

TEST(TimeSeqProcessorSequence, ScriptWithSequenceRefToSequenceWithoutIdShouldFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "a-sequence" } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array({
		{ { "values", json::array({ 0, 1, 2, 3, 4, 5 }) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Id_String, "/component-pool/sequences/0");
}

TEST(TimeSeqProcessorSequence, ScriptWithShorthandSequenceRefShouldUseSequence) {
	MockEventListener mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", "a-sequence" } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array({
		{ { "id", "a-sequence" }, { "values", json::array({ 0, 1, 2, 3, 4, 5 }) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 12; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, i % 6)).Times(1);
		}
	}

	for (int i = 0; i < 12; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSequence, ScriptWithFullSequenceRefShouldUseSequence) {
	MockEventListener mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-sequence" } } } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array({
		{ { "id", "a-sequence" }, { "values", json::array({ 0, 1, 2, 3, 4, 5 }) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 12; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, i % 6)).Times(1);
		}
	}

	for (int i = 0; i < 12; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSequence, SequenceWithRetrieveVoltageOnceUndefinedShouldOnlyRetrieveVoltagesOnce) {
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
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "move-after", "none" } } } } } } } }, // The first sequence value doesn't move the sequence, so we can check what happens when it is used again
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-sequence" } } } } } } } } // The second sequence value should not re-retrieve the variable, but do move the sequence forward
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array({
		{ { "id", "a-sequence" }, { "values", json::array(
			{
				{ { "variable", "var-1" } },
				{ { "variable", "var-2" } },
				{ { "variable", "var-3" } },
				{ { "variable", "var-4" } },
				{ { "variable", "var-5" } }
			}
		 ) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		std::string variableNames[] = { "var-1", "var-2", "var-3", "var-4", "var-5" };
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 5; j++) {
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

				// The first set-value retrieves the variable, stores it and sets it on the output port
				EXPECT_CALL(mockVariableHandler, getVariable(variableNames[j])).Times(1).WillOnce(testing::Return(j));
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, j)).Times(1);

				// The second set-value doesn't retrieve the variable again, but uses the stored value to set on the output port.
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, j)).Times(1);
			}
		}
	}

	for (int i = 0; i < 2 * 5; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSequence, SequenceWithRetrieveVoltageOnceTrueShouldOnlyRetrieveVoltagesOnce) {
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
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "move-after", "none" } } } } } } } }, // The first sequence value doesn't move the sequence, so we can check what happens when it is used again
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-sequence" } } } } } } } } // The second sequence value should not re-retrieve the variable, but do move the sequence forward
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array({
		{ { "id", "a-sequence" }, { "retrieve-voltage-once", true }, { "values", json::array(
			{
				{ { "variable", "var-1" } },
				{ { "variable", "var-2" } },
				{ { "variable", "var-3" } },
				{ { "variable", "var-4" } },
				{ { "variable", "var-5" } }
			}
		 ) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		std::string variableNames[] = { "var-1", "var-2", "var-3", "var-4", "var-5" };
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 5; j++) {
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

				// The first set-value retrieves the variable, stores it and sets it on the output port
				EXPECT_CALL(mockVariableHandler, getVariable(variableNames[j])).Times(1).WillOnce(testing::Return(j));
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, j)).Times(1);

				// The second set-value doesn't retrieve the variable again, but uses the stored value to set on the output port.
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, j)).Times(1);
			}
		}
	}

	for (int i = 0; i < 2 * 5; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSequence, SequenceWithRetrieveVoltageOnceFalseShouldRetrieveVoltagesEachTime) {
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
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "move-after", "none" } } } } } } } }, // The first sequence value doesn't move the sequence, so we can check what happens when it is used again
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-sequence" } } } } } } } } // The second sequence value should re-retrieve the variable, and move the sequence forward
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array({
		{ { "id", "a-sequence" }, { "retrieve-voltage-once", false }, { "values", json::array(
			{
				{ { "variable", "var-1" } },
				{ { "variable", "var-2" } },
				{ { "variable", "var-3" } },
				{ { "variable", "var-4" } },
				{ { "variable", "var-5" } }
			}
		 ) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		std::string variableNames[] = { "var-1", "var-2", "var-3", "var-4", "var-5" };
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 5; j++) {
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);

				// The first set-value retrieves the variable, and sets it on the output port.
				EXPECT_CALL(mockVariableHandler, getVariable(variableNames[j])).Times(1).WillOnce(testing::Return(j));
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, j)).Times(1);

				// The second set-value retrieves the variable again, and sets it on the output port.
				EXPECT_CALL(mockVariableHandler, getVariable(variableNames[j])).Times(1).WillOnce(testing::Return(j));
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, j)).Times(1);
			}
		}
	}

	for (int i = 0; i < 2 * 5; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSequence, SequenceWithSharedFalseShouldMoveIndependently) {
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
			// The first lane lasts two samples, and does one non-move retrieval and one move retrieval, sent to outputs 1 and 2
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "move-after", "none" } } } } } } } }, // It's a non-shared sequence, so the first set-value will never move
				{ { "set-value", { { "output", 2 }, { "value", { { "sequence", { { "id", "a-sequence" } } } } } } } }
			}) } } }) } },
			// The second lane lasts one sample, and does one non-move retrieval and one move retrieval, sent to outputs 3 and 4
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 3 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "move-after", "none" } } } } } } } }, // It's a non-shared sequence, so the first set-value will never move
				{ { "set-value", { { "output", 4 }, { "value", { { "sequence", { { "id", "a-sequence" } } } } } } } }
			}) } } }) } }
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array({
		{ { "id", "a-sequence" }, { "shared", false }, { "values", json::array({ 0.f, 1.f, 2.f, 3.f, 4.f }) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 10; j++) {
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));

				// The first lane only runs every two samples
				if (j % 2 == 0) {
					EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
					EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
					EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, j / 2)).Times(1);
				}

				// The second lane runs on every sample.
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, 0.f)).Times(1);
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(3, 0, j % 5)).Times(1);
			}
		}
	}

	for (int i = 0; i < 2 * 10; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSequence, SequenceWithSharedFalseAndWrapFalseShouldMoveIndependently) {
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
			// The first lane lasts two samples, and does one non-move retrieval and one move retrieval, sent to outputs 1 and 2
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "move-after", "backward" }, { "wrap", false } } } } } } } }, // It's a non-shared sequence, so the first set-value will never move
				{ { "set-value", { { "output", 2 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "wrap", false } } } } } } } }
			}) } } }) } },
			// The second lane lasts one sample, and does one non-move retrieval and one move retrieval, sent to outputs 3 and 4
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 3 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "move-after", "backward" }, { "wrap", false } } } } } } } }, // It's a non-shared sequence, so the first set-value will never move
				{ { "set-value", { { "output", 4 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "wrap", false } } } } } } } }
			}) } } }) } }
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array({
		{ { "id", "a-sequence" }, { "shared", false }, { "values", json::array({ 0.f, 1.f, 2.f, 3.f, 4.f }) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	int value2 = 0;
	int value4 = 0;

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 10; j++) {
				EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));

				// The first lane only runs every two samples
				if (j % 2 == 0) {
					EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
					EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1); // It's always 0.f, since it moves backwards from the first position and does not wrap
					EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, value2)).Times(1);
					value2 = value2 > 3 ? value2 : value2 + 1; // The second value stops moving after reaching the end of the sequence
				}

				// The second lane runs on every sample.
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, 0.f)).Times(1); // It's always 0.f, since it moves backwards from the first position and does not wrap
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(3, 0, value4)).Times(1);
				value4 = value4 > 3 ? value4 : value4 + 1; // The second value stops moving after reaching the end of the sequence
			}
		}
	}

	for (int i = 0; i < 2 * 10; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSequence, SharedSequenceWithMoveBeforeAndMoveAfterShouldComplement) {
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
			// The first lane lasts two samples, and moves forward before retrieving a value from the sequence
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 2 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "move-before", "forward" }, { "move-after", "none" } } } } } } } },
			}) } } }) } },
			// The second lane lasts one sample, and moves forward after retrieving a value from the sequence
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 2 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "move-after", "forward" } } } } } } } },
			}) } } }) } },
			// The third lane lasts three samples, and moves backwards after retrieving a value from the sequence
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 3 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "move-after", "backward" } } } } } } } },
			}) } } }) } },
			// The fourth lane lasts four samples, and moves backwards before retrieving a value from the sequence
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 3 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 4 }, { "value", { { "sequence", { { "id", "a-sequence" }, { "move-before", "backward" }, { "move-after", "none" } } } } } } } },
			}) } } }) } }
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array({
		{ { "id", "a-sequence" }, { "values", json::array({ 0.f, 1.f, 2.f, 3.f, 4.f }) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;
		int pos = 0;
		for (int i = 0; i < 20; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));

			// The first lane only runs every two samples
			if (i % 2 == 0) {
				pos++;
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, pos % 5)).Times(1);
			}

			// The second lane runs on every sample.
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(1, 0, pos % 5)).Times(1);
			pos++;

			// The third lane only runs every three samples
			if (i % 3 == 0) {
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(2, 0, pos % 5)).Times(1);
				pos--;
			}

			// The fourth lane only runs every three samples
			if (i % 3 == 0) {
				pos--;
				EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
				EXPECT_CALL(mockPortHandler, setOutputPortVoltage(3, 0, pos % 5)).Times(1);
			}
		}
	}

	for (int i = 0; i < 20; i++) {
		script.second->process();
	}
}

TEST(TimeSeqProcessorSequence, ScriptWithEmptySequenceShouldUseZeroValues) {
	MockEventListener mockEventListener;
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson(SCRIPT_VERSION_1_2_0);
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-value", { { "output", 1 }, { "value", { { "sequence", { { "id", "an-empty-sequence" } } } } } } } }
			}) } } }) } },
		}) } }
	});
	json["component-pool"] = json::object();
	json["component-pool"]["sequences"] = json::array({
		{ { "id", "an-empty-sequence" }, { "values", json::array({}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 12; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 0.f)).Times(1);
		}
	}

	for (int i = 0; i < 12; i++) {
		script.second->process();
	}
}
