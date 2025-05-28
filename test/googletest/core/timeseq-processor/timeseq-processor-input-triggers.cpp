#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorInputTriggers, ScriptWithNoInputTriggersShouldSucceed) {
	ProcessorLoader processorLoader(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	EXPECT_EQ(script.second->m_triggers.size(), 0u);
}

void testTriggerInvocation(MockPortHandler& mockPortHandler, MockTriggerHandler& mockTriggerHandler, ProcessorLoader& processorLoader, shared_ptr<Processor> processor) {
	// Nothing should happen if none of the inputs are triggered
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(1).WillOnce(testing::Return(0.f));
		EXPECT_CALL(mockTriggerHandler, setTrigger).Times(0);
		processor->process();
		testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
		testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);
	}

	// One trigger should be sent out as long as the voltage remains hight (> 1.f)
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.1f)); // Five total invocations for the first trigger > 1.f, followed by 1 0.f response
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.1f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(2).WillRepeatedly(testing::Return(5.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.1f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(0.f));
	}
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(1).WillOnce(testing::Return(1.1f)); // Four total invocations for the second trigger >1.f, followed by 2 0.f responses
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(3).WillRepeatedly(testing::Return(1.1f)); // Four total invocations for the second trigger >1.f, followed by 2 0.f responses
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(2).WillRepeatedly(testing::Return(0.f)); // Four total invocations for the second trigger >1.f, followed by 2 0.f responses
	}
	// Now do 6 process invocations, which should run through all mocked input voltages
	for (int i = 0; i < 6; i++) {
		processor->process();
	}
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// A new trigger should be sent out for the first trigger again if that voltage goes up again, while the second input remains low
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.1f));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(0.f));
	}
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(2).WillRepeatedly(testing::Return(0.f));
	}
	// Do the two expected process calls
	processor->process(); processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// The same again, but this time with the second trigger high
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(1).WillOnce(testing::Return(1.1f));
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(1).WillRepeatedly(testing::Return(0.f));
	}
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(2).WillRepeatedly(testing::Return(0.f));
	}
	// Do the two expected process calls
	processor->process(); processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// Any further process invocation should do nothing as long as the input voltage remains low
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger1Name)).Times(0);
	EXPECT_CALL(mockTriggerHandler, setTrigger(trigger2Name)).Times(0);
	EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(6).WillRepeatedly(testing::Return(0.f));
	EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(6).WillRepeatedly(testing::Return(0.f));
	for (int i = 0; i < 6; i++) {
		processor->process();
	}
}

TEST(TimeSeqProcessorInputTriggers, ScriptWithInputTriggersWithNonExistingPooledInputsShouldFail) {
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["input-triggers"] = json::array({
		{ { "id", "trigger-1" }, { "input", { { "ref", "input-1" } } } },
		{ { "id", "trigger-2" }, { "input", { { "ref", "input-2" } } } },
		{ { "id", "trigger-3" }, { "input", { { "ref", "input-3" } } } },
		{ { "id", "trigger-4" }, { "input", { { "ref", "input-4" } } } }
	});
	json["component-pool"] = { { "inputs", json::array({
		{ { "id", "input-1" }, { "index", 2 } },
		{ { "id", "input-3" }, { "index", 5 }, { "channel", 2 } }
	})}};

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 2u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/input-triggers/1/input");
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/input-triggers/3/input");
}


TEST(TimeSeqProcessorInputTriggers, ScriptWithInputTriggersWithInlineInputsShouldWork) {
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["input-triggers"] = json::array({
		{ { "id", "trigger-1" }, { "input", { { "index", 2 } } } },
		{ { "id", "trigger-2" }, { "input", { { "index", 5 }, { "channel", 2 } } } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_triggers.size(), 2u);

	testTriggerInvocation(mockPortHandler, mockTriggerHandler, processorLoader, script.second);
}

TEST(TimeSeqProcessorInputTriggers, ScriptWithInputTriggersWithPooledInputsShouldWork) {
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["input-triggers"] = json::array({
		{ { "id", "trigger-1" }, { "input", { { "ref", "input-1" } } } },
		{ { "id", "trigger-2" }, { "input", { { "ref", "input-2" } } } }
	});
	json["component-pool"] = { { "inputs", json::array({
		{ { "id", "input-1" }, { "index", 2 } },
		{ { "id", "input-2" }, { "index", 5 }, { "channel", 2 } }
	})}};

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);
	ASSERT_EQ(script.second->m_triggers.size(), 2u);

	testTriggerInvocation(mockPortHandler, mockTriggerHandler, processorLoader, script.second);
}
