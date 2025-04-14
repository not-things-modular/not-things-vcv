#include "timeseq-processor-shared.hpp"
#include "core/timeseq-core.hpp"
#include <gmock/gmock.h>

struct MockPortHandler : PortHandler {
	MOCK_METHOD(float, getInputPortVoltage, (int, int), (override));
	MOCK_METHOD(float, getOutputPortVoltage, (int, int), (override));
	MOCK_METHOD(void, setOutputPortVoltage, (int, int, float), (override));
	MOCK_METHOD(void, setOutputPortChannels, (int, int), (override));
};

struct MockVariableHandler : VariableHandler {
	MOCK_METHOD(float, getVariable, (std::string), (override));
	MOCK_METHOD(void, setVariable, (std::string, float), (override));
};

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
	MockVariableHandler mockVariableHandler;
	ProcessorLoader processorLoader(&mockPortHandler, &mockVariableHandler, nullptr, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["global-actions"] = json::array({
		{ { "timing", "start" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
		{ { "timing", "start" }, { "set-value", { { "output", { { "index", 1 } } }, { "value", { { "voltage", 1.5 } } } } } }
	});

	shared_ptr<Processor> processor = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);

	// Process should not trigger the global start actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(0);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	// A reset should trigger the actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(1);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(1);
	processor->reset();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	// Another process should still not trigger the global start actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(0);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	// A re-reset should re-trigger the actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(1);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(1);
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
			{ { "id", "action-1" }, { "timing", "start" }, { "set-variable", { { "name", "variable-name" }, { "value", { { "voltage", 5 } } } } } },
			{ { "id", "action-2" }, { "timing", "start" }, { "set-value", { { "output", { { "index", 1 } } }, { "value", { { "voltage", 1.5 } } } } } }
		})}
	};
	json["global-actions"] = json::array({
		{ { "ref", "action-1" } },
		{ { "ref", "action-2" } }
	});

	shared_ptr<Processor> processor = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u) << json.dump() << validationErrors[0].location << " " << validationErrors[0].message << " " << validationErrors[1].location << " " << validationErrors[1].message;

	// Process should not trigger the global start actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(0);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	// A reset should trigger the actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(1);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(1);
	processor->reset();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	// Another process should still not trigger the global start actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(0);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(0);
	processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	// A re-reset should re-trigger the actions
	EXPECT_CALL(mockPortHandler, setOutputPortVoltage).Times(1);
	EXPECT_CALL(mockVariableHandler, setVariable).Times(1);
	processor->reset();
}
