#include "timeseq-processor/timeseq-processor-shared.hpp"
#include "core/timeseq-core.hpp"
#include <gmock/gmock.h>

using namespace timeseq;

#define DUMMY_TIMESEQ_SCRIPT "the timeseq script data";

std::string var1 = "var1";
std::string var2 = "var2";
std::string var3 = "var3";
std::string var4 = "var4";

struct MockJsonLoader : JsonLoader {
	MOCK_METHOD(std::shared_ptr<Script>, loadScript, (std::istream&, std::vector<ValidationError>*), (override));
};

struct MockProcessorLoader : ProcessorLoader {
	MockProcessorLoader() : ProcessorLoader(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) {}
	MOCK_METHOD(std::shared_ptr<Processor>, loadScript, (std::shared_ptr<Script> script, std::vector<ValidationError>*), (override));
};

struct MockProcessor : Processor {
	MockProcessor() : Processor(std::vector<std::shared_ptr<TimelineProcessor>>(), std::vector<std::shared_ptr<TriggerProcessor>>(), std::vector<std::shared_ptr<ActionProcessor>>()) {}

	MOCK_METHOD(void, reset, (), (override));
	MOCK_METHOD(void, process, (), (override));
};

ACTION_P(AddValidationErrorAndReturnEmptyScript, validationError) {
	std::vector<ValidationError>* validationErrors = arg1;
	validationErrors->push_back(validationError);
	return std::shared_ptr<Script>();
}

ACTION_P(AddValidationErrorAndReturnEmptyProcessor, validationError) {
	std::vector<ValidationError>* validationErrors = arg1;
	validationErrors->push_back(validationError);
	return std::shared_ptr<Processor>();
}

TEST(TimeSeqCore, LoadScriptWithInvalidJsonScriptShouldHandleValidationErrors) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	TimeSeqCore timeSeqCore(mockJsonLoader, nullptr, nullptr, nullptr);

	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;
	std::vector<ValidationError> returningValidationErrors = { ValidationError("/1", "error-1") };

	EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(AddValidationErrorAndReturnEmptyScript(returningValidationErrors[0]));

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);

	EXPECT_EQ(resultValidationErrors, returningValidationErrors);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);
}

TEST(TimeSeqCore, LoadScriptShouldNotLoadProcessorIfThereIsNoScript) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	TimeSeqCore timeSeqCore(mockJsonLoader, nullptr, nullptr, nullptr);

	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(std::shared_ptr<Script>()));

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);

	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);
}

TEST(TimeSeqCore, LoadScriptShouldNotLoadProcessorOnInvalidProcessorScript) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, nullptr);

	std::shared_ptr<Script> script(new Script());
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;
	std::vector<ValidationError> returningValidationErrors = { ValidationError("/1", "error-1") };

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script));
		EXPECT_CALL(*mockProcessorLoader, loadScript).Times(1).WillOnce(AddValidationErrorAndReturnEmptyProcessor(returningValidationErrors[0]));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);

	EXPECT_EQ(resultValidationErrors, returningValidationErrors);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);
}

TEST(TimeSeqCore, LoadScriptShouldNotLoadProcessorWhenNoProcessorParsed) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, nullptr);

	std::shared_ptr<Script> script(new Script());
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;
	std::vector<ValidationError> returningValidationErrors = { ValidationError("/1", "error-1") };

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script));
		EXPECT_CALL(*mockProcessorLoader, loadScript).Times(1).WillOnce(testing::Return(nullptr));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);

	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);
}

TEST(TimeSeqCore, LoadScriptShouldInitializeIdleOnInitialScriptLoad) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script(new Script());
	std::shared_ptr<Processor> processor(new Processor({}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script));
	EXPECT_CALL(*mockProcessorLoader, loadScript(script, testing::_)).Times(1).WillOnce(testing::Return(processor));
	EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);

	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::IDLE);
	EXPECT_EQ(timeSeqCore.m_script, script);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	// The current sample rate should have been captured in the core
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
}

TEST(TimeSeqCore, LoadScriptShouldReplaceExistingScriptOnNewSuccesfulLoad) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<Script> script2(new Script());
	std::shared_ptr<Processor> processor1(new Processor({}, {}, {}));
	std::shared_ptr<Processor> processor2(new Processor({}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor1));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script2));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script2, testing::_)).Times(1).WillOnce(testing::Return(processor2));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(69));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::IDLE);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor1);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	// Start the core
	timeSeqCore.start(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);

	resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	// The core should have become idle again
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::IDLE);
	EXPECT_EQ(timeSeqCore.m_script, script2);
	EXPECT_EQ(timeSeqCore.m_processor, processor2);
	// The current sample rate should have been re-captured in the core
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 69u);
}

TEST(TimeSeqCore, LoadScriptShouldNotReplaceExistingScriptIfNewScriptFailsToLoad) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<Script> script2(new Script());
	std::shared_ptr<Processor> processor1(new Processor({}, {}, {}));
	std::shared_ptr<Processor> processor2(new Processor({}, {}, {}));
	std::vector<ValidationError> returningValidationErrors1 = { ValidationError("/1", "error-1") };
	std::vector<ValidationError> returningValidationErrors2 = { ValidationError("/2", "error-2") };
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor1));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));

		// (1) First fail with validation errors during JSON parsing
		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(AddValidationErrorAndReturnEmptyScript(returningValidationErrors1[0]));

		// (2) Then fail with an empty JSON response
		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(nullptr));

		// (3) Next fail with validation errors during processor parsing
		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript).Times(1).WillOnce(AddValidationErrorAndReturnEmptyProcessor(returningValidationErrors2[0]));

		// (4) Finally fail with an processor
		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript).Times(1).WillOnce(testing::Return(nullptr));

		// And check that we still can load a valid script after that
		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script2));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script2, testing::_)).Times(1).WillOnce(testing::Return(processor2));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(69));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::IDLE);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor1);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	// Start the core so we can verify that it keeps running
	timeSeqCore.start(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);

	// Check failure scenario (1)
	resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors, returningValidationErrors1);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor1);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	
	// Check failure scenario (2)
	resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor1);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);

	// Check failure scenario (3)
	resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors, returningValidationErrors2);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor1);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);

	// Check failure scenario (4)
	resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor1);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);

	// And make sure that a valid script can still be loaded
	resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	// The core should have become idle again
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::IDLE);
	EXPECT_EQ(timeSeqCore.m_script, script2);
	EXPECT_EQ(timeSeqCore.m_processor, processor2);
	// The current sample rate should have been re-captured in the core
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 69u);
}

TEST(TimeSeqCore, ReloadScriptShouldDoNothingWhenNoScriptIsLoaded) {
	TimeSeqCore timeSeqCore(nullptr, nullptr, nullptr, nullptr);

	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);

	timeSeqCore.reloadScript();

	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);
}

TEST(TimeSeqCore, ReloadScriptShouldReloadProcessorFromCurrentScript) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<Processor> processor1(new Processor({}, {}, {}));
	std::shared_ptr<Processor> processor2(new Processor({}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor1));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));

		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor2));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(69));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::IDLE);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor1);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	// Start the core
	timeSeqCore.start(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);

	timeSeqCore.reloadScript();
	// The core should have become idle again
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::IDLE);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor2);
	// The current sample rate should have been re-captured in the core
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 69u);
}

TEST(TimeSeqCore, ClearScriptShouldDoNothingWhenNoScriptIsLoaded) {
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(nullptr, nullptr, nullptr, &mockEventListener);

	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);

	timeSeqCore.clearScript();

	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);
}

TEST(TimeSeqCore, PauseScriptShouldPauseWhenScriptLoaded) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<Processor> processor(new Processor({}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::IDLE);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	// Start the core
	timeSeqCore.start(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);

	timeSeqCore.pause();
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::PAUSED);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
}

TEST(TimeSeqCore, PauseScriptShouldDoNothingWhenNoScriptIsLoaded) {
	TimeSeqCore timeSeqCore(nullptr, nullptr, nullptr, nullptr);

	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);

	timeSeqCore.pause();

	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);
}

TEST(TimeSeqCore, StartScriptShouldDoNothingWhenNoScriptIsLoaded) {
	TimeSeqCore timeSeqCore(nullptr, nullptr, nullptr, nullptr);

	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);

	timeSeqCore.start(0);

	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);
}

TEST(TimeSeqCore, StartScriptShouldRestartScriptButKeepTriggersVariablesAndProgress) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<Processor> processor(new Processor({}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::IDLE);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	// Start the core
	timeSeqCore.start(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);

	// Let the script make some progress
	for (int i = 0; i < 5; i++) {
		timeSeqCore.process(1);
	}
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 5u);

	// Enter some triggers and variables
	timeSeqCore.setVariable(var1, 1.f);
	timeSeqCore.setVariable(var2, 2.f);
	timeSeqCore.setTrigger(trigger1Name);
	timeSeqCore.process(1); // A process call will switch "current" and "next" trigger pool
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({ trigger1Name }));
	timeSeqCore.setTrigger(trigger2Name);
	timeSeqCore.process(1); // Another process call will switch "current" and "next" trigger pool
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 7u);
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({ trigger2Name }));
	EXPECT_EQ(timeSeqCore.getVariable(var1), 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 2.f);

	// Start the core again
	timeSeqCore.start(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	
	// Check that the variables and triggers are still there
	EXPECT_EQ(timeSeqCore.getVariable(var1), 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 2.f);
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({ trigger2Name }));
	// And the progress should have been maintained, and continue from where it left off
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 7u);
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 8u);
}

TEST(TimeSeqCore, ResetScriptShouldRestartScriptAndClearTriggersVariablesAndProgress) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<MockProcessor> processor(new testing::NiceMock<MockProcessor>());
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::IDLE);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	// Start the core
	timeSeqCore.start(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);

	// Let the script make some progress
	for (int i = 0; i < 5; i++) {
		timeSeqCore.process(1);
	}
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 5u);

	// Enter some triggers and variables
	timeSeqCore.setVariable(var1, 1.f);
	timeSeqCore.setVariable(var2, 2.f);
	timeSeqCore.setTrigger(trigger1Name);
	timeSeqCore.process(1); // A process call will switch "current" and "next" trigger pool
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({ trigger1Name }));
	timeSeqCore.setTrigger(trigger2Name);
	timeSeqCore.process(1); // Another process call will switch "current" and "next" trigger pool
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 7u);
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({ trigger2Name }));
	EXPECT_EQ(timeSeqCore.getVariable(var1), 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 2.f);

	// Reset the core

	{
		testing::InSequence inSequence;
		EXPECT_CALL(*processor, reset()).Times(1);
		EXPECT_CALL(*processor, process()).Times(1);
	}
	timeSeqCore.reset();
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	
	// Check that the variables and triggers are cleared
	EXPECT_EQ(timeSeqCore.getVariable(var1), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 0.f);
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({}));
	// Progress should start again from the beginning
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 0u);
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 1u);
}

TEST(TimeSeqCore, SetVariableShouldUpdateVariable) {
	TimeSeqCore timeSeqCore(nullptr, nullptr, nullptr, nullptr);

	EXPECT_EQ(timeSeqCore.getVariable(var1), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var3), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var4), 0.f);

	timeSeqCore.setVariable(var2, 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var1), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var3), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var4), 0.f);

	timeSeqCore.setVariable(var1, 2.f);
	EXPECT_EQ(timeSeqCore.getVariable(var1), 2.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var3), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var4), 0.f);

	timeSeqCore.setVariable(var3, 3.f);
	EXPECT_EQ(timeSeqCore.getVariable(var1), 2.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var3), 3.f);
	EXPECT_EQ(timeSeqCore.getVariable(var4), 0.f);

	timeSeqCore.setVariable(var3, 4.f);
	EXPECT_EQ(timeSeqCore.getVariable(var1), 2.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var3), 4.f);
	EXPECT_EQ(timeSeqCore.getVariable(var4), 0.f);

	timeSeqCore.setVariable(var3, 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var1), 2.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var3), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var4), 0.f);

	timeSeqCore.setVariable(var1, 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var1), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var3), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var4), 0.f);

	timeSeqCore.setVariable(var4, 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var1), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var3), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var4), 0.f);

	timeSeqCore.setVariable(var3, 5.f);
	EXPECT_EQ(timeSeqCore.getVariable(var1), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 1.f);
	EXPECT_EQ(timeSeqCore.getVariable(var3), 5.f);
	EXPECT_EQ(timeSeqCore.getVariable(var4), 0.f);
}

TEST(TimeSeqCore, ProcessShouldDoNothingWhenNoScriptLoaded) {
	TimeSeqCore timeSeqCore(nullptr, nullptr, nullptr, nullptr);

	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 0u);

	timeSeqCore.process(1);

	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 0u);
}

TEST(TimeSeqCore, ProcessShouldAdvanceScriptAndHandleTriggers) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<MockProcessor> processor(new testing::NiceMock<MockProcessor>());
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::IDLE);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	// Start the core
	timeSeqCore.start(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 0u);

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*processor, process()).Times(5);

		EXPECT_CALL(mockEventListener, triggerTriggered).Times(1);
		EXPECT_CALL(*processor, process()).Times(1);
		EXPECT_CALL(mockEventListener, triggerTriggered).Times(2);
		EXPECT_CALL(*processor, process()).Times(1);
		EXPECT_CALL(mockEventListener, triggerTriggered).Times(2);
		EXPECT_CALL(*processor, process()).Times(1);

		EXPECT_CALL(*processor, process()).Times(5);
	}

	for (unsigned int i = 0; i < 5; i++) {
		timeSeqCore.process(1);
		EXPECT_EQ(timeSeqCore.getElapsedSamples(), i + 1);
	}

	timeSeqCore.setTrigger(trigger1Name);
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>()); // The trigger gets active after calling the process method.
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({ trigger1Name })); // The trigger should have become active now.
	timeSeqCore.setTrigger(trigger2Name);
	timeSeqCore.setTrigger(trigger3Name);
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({ trigger1Name })); // The trigger gets active after calling the process method.
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({ trigger2Name, trigger3Name })); // The trigger should have become active now.
	timeSeqCore.setTrigger(trigger3Name);
	timeSeqCore.setTrigger(trigger4Name);
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({ trigger2Name, trigger3Name })); // The trigger gets active after calling the process method.
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({ trigger3Name, trigger4Name })); // The trigger should have become active now.

	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 8u);

	// The triggers should be cleared now
	for (unsigned int i = 0; i < 5; i++)  {
		timeSeqCore.process(1);
		EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>());
		EXPECT_EQ(timeSeqCore.getElapsedSamples(), 9 + i);
	}
}

TEST(TimeSeqCore, ElapsedSamplesShouldLoopOnHourBoundary) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script(new Script());
	std::shared_ptr<Processor> processor(new Processor({}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script));
	EXPECT_CALL(*mockProcessorLoader, loadScript(script, testing::_)).Times(1).WillOnce(testing::Return(processor));
	EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(12));

	timeSeqCore.loadScript(scriptData);

	// Do 10 loops of 12 smaples per sec = 12 * 60 * 60 samples per hour, and check that the elapsed samples loops on the hour
	for (int i = 0; i < 10; i++) {
		for (unsigned int j = 0; j < 12 * 60 * 60; j++) {
			ASSERT_EQ(timeSeqCore.getElapsedSamples(), j);
			timeSeqCore.process(1);
		}
	}
}
