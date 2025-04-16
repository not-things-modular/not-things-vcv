#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorInputTriggers, ScriptWithNoInputTriggersShouldSucceed) {
	ProcessorLoader processorLoader(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();

	shared_ptr<Processor> processor = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u);
	EXPECT_EQ(processor->m_triggers.size(), 0u);
}

void testTriggerInvocation(MockPortHandler& mockPortHandler, MockTriggerHandler& mockTriggerHandler, ProcessorLoader& processorLoader, shared_ptr<Processor> processor) {
	// Nothing should happen if none of the inputs are triggered
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
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
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.1f)); // Five total invocations for the first trigger > 1.f, followed by 1 0.f response
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.1f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(2).WillRepeatedly(testing::Return(5.f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.1f));
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(0.f));
	}
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(1).WillOnce(testing::Return(1.1f)); // Four total invocations for the second trigger >1.f, followed by 2 0.f responses
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
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
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(1.1f));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(1).WillOnce(testing::Return(0.f));
	}
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(2).WillRepeatedly(testing::Return(0.f));
	}
	// Do the two expected process calls
	processor->process(); processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// The same again, but this time with the second trigger high
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(1).WillOnce(testing::Return(1.1f));
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(1);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(1).WillRepeatedly(testing::Return(0.f));
	}
	{
		testing::InSequence inSequence;
		EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
		EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(2).WillRepeatedly(testing::Return(0.f));
	}
	// Do the two expected process calls
	processor->process(); processor->process();
	testing::Mock::VerifyAndClearExpectations(&mockPortHandler);
	testing::Mock::VerifyAndClearExpectations(&mockTriggerHandler);

	// Any further process invocation should do nothing as long as the input voltage remains low
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-1")).Times(0);
	EXPECT_CALL(mockTriggerHandler, setTrigger("trigger-2")).Times(0);
	EXPECT_CALL(mockPortHandler, getInputPortVoltage(4, 1)).Times(6).WillRepeatedly(testing::Return(0.f));
	EXPECT_CALL(mockPortHandler, getInputPortVoltage(1, 0)).Times(6).WillRepeatedly(testing::Return(0.f));
	for (int i = 0; i < 6; i++) {
		processor->process();
	}
}

TEST(TimeSeqProcessorInputTriggers, ScriptWithInlineInputTriggersWithInlineInputsShouldLoad) {
	MockPortHandler mockPortHandler;
	MockTriggerHandler mockTriggerHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, nullptr, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["input-triggers"] = json::array({
		{ { "id", "trigger-1" }, { "input", { { "index", 2 } } } },
		{ { "id", "trigger-2" }, { "input", { { "index", 5 }, { "channel", 2 } } } }
	});

	shared_ptr<Processor> processor = loadProcessor(processorLoader, json, &validationErrors);
	ASSERT_EQ(validationErrors.size(), 0u) << validationErrors[0].location << " " << validationErrors[1].message;
	ASSERT_EQ(processor->m_triggers.size(), 2u);

	testTriggerInvocation(mockPortHandler, mockTriggerHandler, processorLoader, processor);
}
