#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorGlobalActions, ScriptWithNoGlobalActionsShouldSucceed) {
	ProcessorLoader processorLoader(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();

	shared_ptr<Processor> processor = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	EXPECT_EQ(processor->m_startActions.size(), 0u);
}

TEST(TimeSeqProcessorGlobalActions, ScriptWithNonStartGlobalActionsShouldFail) {
	ProcessorLoader processorLoader(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["global-actions"] = json::array({
		{ { "timing", "end" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
		{ { "timing", "glide" }, { "start-value", { { "voltage", 5 } } }, { "end-value", { { "voltage", 5 } } }, { "output", { { "index", 1 } } } }
	});

	shared_ptr<Processor> processor = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Script_GlobalActionTiming, "/global-actions/0");
	expectError(validationErrors, ValidationErrorCode::Script_GlobalActionTiming, "/global-actions/1");
}

TEST(TimeSeqProcessorGlobalActions, ScriptWithInlineStartActionsShouldLoad) {
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, &mockTriggerHandler, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["global-actions"] = json::array({
		{ { "timing", "start" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 4.1 } } } } } },
		{ { "timing", "start" }, { "set-value", { { "output", { { "index", 1 } } }, { "value", { { "voltage", 1.5 } } } } } },
		{ { "timing", "start" }, { "trigger", "trigger-name" } },
	});

	shared_ptr<Processor> processor = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(processor->m_startActions.size(), 3u);

	// Process should not trigger the global start actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(0);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	EXPECT_CALL(mockTriggerHandler, setTrigger).Times(0);
	processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	testing::Mock::VerifyAndClearExpectations(&mockVariableHandler);
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	// A reset should trigger the actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 1.5f)).Times(1);
	EXPECT_CALL(mockVariableHandler, setVariable("variable-name", 4.1)).Times(1);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-name")).Times(1);
	processor->reset();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	testing::Mock::VerifyAndClearExpectations(&mockVariableHandler);
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	// Another process should still not trigger the global start actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(0);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	EXPECT_CALL(mockTriggerHandler, setTrigger).Times(0);
	processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	testing::Mock::VerifyAndClearExpectations(&mockVariableHandler);
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	// A re-reset should re-trigger the actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 1.5f)).Times(1);
	EXPECT_CALL(mockVariableHandler, setVariable("variable-name", 4.1)).Times(1);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-name")).Times(1);
	processor->reset();
}

TEST(TimeSeqProcessorGlobalActions, ScriptWithPooledStartActionsShouldLoad) {
	MockPortHandler mockPortHandler;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, nullptr, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "start" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 4.1 } } } } } },
			{ { "id", "action-2" }, { "timing", "start" }, { "set-value", { { "output", { { "index", 1 } } }, { "value", { { "voltage", 1.5 } } } } } }
		})}
	};
	json["global-actions"] = json::array({
		{ { "ref", "action-1" } },
		{ { "ref", "action-2" } }
	});

	shared_ptr<Processor> processor = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	ASSERT_EQ(processor->m_startActions.size(), 2u);

	// Process should not trigger the global start actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(0);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	// A reset should trigger the actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 1.5f)).Times(1);
	EXPECT_CALL(mockVariableHandler, setVariable("variable-name", 4.1)).Times(1);
	processor->reset();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	// Another process should still not trigger the global start actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(0);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	// A re-reset should re-trigger the actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage(0, 0, 1.5f)).Times(1);
	EXPECT_CALL(mockVariableHandler, setVariable("variable-name", 4.1)).Times(1);
	processor->reset();
}

TEST(TimeSeqProcessorGlobalActions, ScriptWithUnknownPooledStartActionsShouldFail) {
	MockPortHandler mockPortHandler;
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, nullptr, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["component-pool"] = {
		{ "actions", json::array({
			{ { "id", "action-1" }, { "timing", "start" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
			{ { "id", "action-2" }, { "timing", "start" }, { "set-value", { { "output", { { "index", 1 } } }, { "value", { { "voltage", 1.5 } } } } } }
		})}
	};
	json["global-actions"] = json::array({
		{ { "ref", "action-1" } },
		{ { "ref", "action-1.5" } },
		{ { "ref", "action-2" } }
	});

	shared_ptr<Processor> processor = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/global-actions/1");
}
