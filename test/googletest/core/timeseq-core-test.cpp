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
	MockProcessor() : Processor(std::shared_ptr<Script>(), std::vector<std::shared_ptr<TimelineProcessor>>(), std::vector<std::shared_ptr<TriggerProcessor>>(), std::vector<std::shared_ptr<ActionProcessor>>()) {}

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

TEST(TimeSeqCore, LoadScriptShouldInitializeLoadingOnInitialScriptLoad) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script(new Script());
	std::shared_ptr<Processor> processor(new Processor({}, {}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script));
	EXPECT_CALL(*mockProcessorLoader, loadScript(script, testing::_)).Times(1).WillOnce(testing::Return(processor));
	EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);

	EXPECT_CALL(mockEventListener, scriptReset()).Times(0);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
	EXPECT_EQ(timeSeqCore.m_script, script);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	// The current sample rate should have been captured in the core
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);

	// Doing one processing cycle should set the processing state to paused and trigger a reset
	EXPECT_CALL(mockEventListener, scriptReset()).Times(1);
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::PAUSED);
}

TEST(TimeSeqCore, LoadScriptShouldReplaceExistingScriptOnNewSuccesfulLoad) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<Script> script2(new Script());
	std::shared_ptr<Processor> processor1(new Processor({}, {}, {}, {}));
	std::shared_ptr<Processor> processor2(new Processor({}, {}, {}, {}));
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

	EXPECT_CALL(mockEventListener, scriptReset()).Times(0);
	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor1);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	// Perform one processing cycle to complete the load
	EXPECT_CALL(mockEventListener, scriptReset()).Times(1);
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::PAUSED);
	// Start the core
	timeSeqCore.start(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);

	resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	// The core should have become idle again
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
	EXPECT_EQ(timeSeqCore.m_script, script2);
	EXPECT_EQ(timeSeqCore.m_processor, processor2);
	// Perform one processing cycle to complete the load
	EXPECT_CALL(mockEventListener, scriptReset()).Times(1); // Another reset should happen
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::PAUSED);
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
	std::shared_ptr<Processor> processor1(new Processor({}, {}, {}, {}));
	std::shared_ptr<Processor> processor2(new Processor({}, {}, {}, {}));
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

		// (4) Finally fail with a processor
		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript).Times(1).WillOnce(testing::Return(nullptr));

		// And check that we still can load a valid script after that
		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script2));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script2, testing::_)).Times(1).WillOnce(testing::Return(processor2));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(69));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor1);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	// Start the core so we can verify that it keeps running
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::PAUSED);
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
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
	EXPECT_EQ(timeSeqCore.m_script, script2);
	EXPECT_EQ(timeSeqCore.m_processor, processor2);
	// The current sample rate should have been re-captured in the core
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 69u);
	// And the core should become paused after one cycle
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::PAUSED);
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
	std::shared_ptr<Processor> processor1(new Processor({}, {}, {}, {}));
	std::shared_ptr<Processor> processor2(new Processor({}, {}, {}, {}));
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
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor1);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	// Start the core
	timeSeqCore.start(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);

	timeSeqCore.reloadScript();
	// The core should have become idle again
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor2);
	// The current sample rate should have been re-captured in the core
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 69u);
	// One cycle should move the state to paused
	timeSeqCore.process(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::PAUSED);
}

TEST(TimeSeqCore, ClearScriptShouldDoNothingWhenNoScriptIsLoaded) {
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(nullptr, nullptr, nullptr, &mockEventListener);

	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);

	timeSeqCore.clearScript();

	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
	EXPECT_EQ(timeSeqCore.m_script, nullptr);
	EXPECT_EQ(timeSeqCore.m_processor, nullptr);
	timeSeqCore.process(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::EMPTY);
}

TEST(TimeSeqCore, PauseScriptShouldPauseWhenScriptLoaded) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<Processor> processor(new Processor({}, {}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
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
	std::shared_ptr<Processor> processor(new Processor({}, {}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
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
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
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
		EXPECT_CALL(*processor, process()).Times(2);
	}
	timeSeqCore.reset();
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);

	// The next process call will do the actual reset
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	
	// Check that the variables and triggers are cleared
	EXPECT_EQ(timeSeqCore.getVariable(var1), 0.f);
	EXPECT_EQ(timeSeqCore.getVariable(var2), 0.f);
	EXPECT_EQ(timeSeqCore.getTriggers(), std::vector<std::string>({}));
	// Progress should start again from the beginning
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 1u);
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 2u);
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
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
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
	std::shared_ptr<Processor> processor(new Processor({}, {}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script));
	EXPECT_CALL(*mockProcessorLoader, loadScript(script, testing::_)).Times(1).WillOnce(testing::Return(processor));
	EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(12));

	timeSeqCore.loadScript(scriptData);
	timeSeqCore.start(0);

	// Do 10 loops of 12 smaples per sec = 12 * 60 * 60 samples per hour, and check that the elapsed samples loops on the hour
	for (int i = 0; i < 10; i++) {
		for (unsigned int j = 0; j < 12 * 60 * 60; j++) {
			ASSERT_EQ(timeSeqCore.getElapsedSamples(), j);
			timeSeqCore.process(1);
		}
	}
}

TEST(TimeSeqCore, StartScriptShouldCauseImmediateProcessingOnZeroDelay) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<Processor> processor(new Processor({}, {}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	
	// Start the core
	timeSeqCore.start(0);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 1u);

	// Do some more processing loops and verify we moved further
	for (unsigned int i = 2; i < 20; i++) {
		timeSeqCore.process(1);
		EXPECT_EQ(timeSeqCore.getElapsedSamples(), i);
	}
}

TEST(TimeSeqCore, StartScriptShouldDelayProcessingWhenStartedWithDelay) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<Processor> processor(new Processor({}, {}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	
	// Start the core
	timeSeqCore.start(10);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	timeSeqCore.process(1);
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 0u);

	// The next 9 cycles should still delay
	for (unsigned int i = 0; i < 9; i++) {
		timeSeqCore.process(1);
		EXPECT_EQ(timeSeqCore.getElapsedSamples(), 0u);
	}

	// Do some more processing loops and verify we actually started
	for (unsigned int i = 1; i < 20; i++) {
		timeSeqCore.process(1);
		EXPECT_EQ(timeSeqCore.getElapsedSamples(), i);
	}
}

TEST(TimeSeqCore, StartScriptShouldDelayProcessingWhenStartedWithDelayAndProcessMultipleCyclesAfterwards) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<Processor> processor(new Processor({}, {}, {}, {}));
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);
	EXPECT_EQ(resultValidationErrors.size(), 0u);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::LOADING);
	EXPECT_EQ(timeSeqCore.m_script, script1);
	EXPECT_EQ(timeSeqCore.m_processor, processor);
	EXPECT_EQ(timeSeqCore.getCurrentSampleRate(), 420u);
	
	// Start the core
	timeSeqCore.start(10);
	EXPECT_EQ(timeSeqCore.getStatus(), TimeSeqCore::Status::RUNNING);
	timeSeqCore.process(20);
	EXPECT_EQ(timeSeqCore.getElapsedSamples(), 0u);

	// The next 9 cycles should still delay
	for (unsigned int i = 0; i < 9; i++) {
		timeSeqCore.process(20); // Even asking for multiple cycles should do nothing
		EXPECT_EQ(timeSeqCore.getElapsedSamples(), 0u);
	}

	// Do some more processing loops and verify we actually started and completes as many cycles as requested
	for (unsigned int i = 1; i < 20; i++) {
		timeSeqCore.process(20);
		EXPECT_EQ(timeSeqCore.getElapsedSamples(), i * 20);
	}
}

TEST(TimeSeqCore, ResetShouldClearVariablesBeforeResettingProcessor) {
	std::shared_ptr<MockJsonLoader> mockJsonLoader(new MockJsonLoader());
	std::shared_ptr<MockProcessorLoader> mockProcessorLoader(new MockProcessorLoader());
	MockSampleRateReader mockSampleRateReader;
	testing::NiceMock<MockEventListener> mockEventListener;
	TimeSeqCore timeSeqCore(mockJsonLoader, mockProcessorLoader, &mockSampleRateReader, &mockEventListener);

	std::shared_ptr<Script> script1(new Script());
	std::shared_ptr<MockProcessor> processor(new testing::NiceMock<MockProcessor>());
	std::string scriptData = DUMMY_TIMESEQ_SCRIPT;

	std::string variableName = "before1";
	timeSeqCore.setVariable(variableName, 1.f);
	variableName = "before2";
	timeSeqCore.setVariable(variableName, 2.f);

	{
		testing::InSequence inSequence;

		EXPECT_CALL(*mockJsonLoader, loadScript).Times(1).WillOnce(testing::Return(script1));
		EXPECT_CALL(*mockProcessorLoader, loadScript(script1, testing::_)).Times(1).WillOnce(testing::Return(processor));
		EXPECT_CALL(mockSampleRateReader, getSampleRate()).Times(1).WillOnce(testing::Return(420));
	}

	std::vector<ValidationError> resultValidationErrors = timeSeqCore.loadScript(scriptData);

	EXPECT_CALL(*processor.get(), reset()).Times(1).WillOnce(testing::Invoke([&]() {
		std::string variableName = "after1";
		timeSeqCore.setVariable(variableName, 3.f);
		variableName = "after2";
		timeSeqCore.setVariable(variableName, 4.f);
	}));

	variableName = "before1";
	EXPECT_EQ(timeSeqCore.getVariable(variableName), 1.f);
	variableName = "before2";
	EXPECT_EQ(timeSeqCore.getVariable(variableName), 2.f);
	variableName = "after1";
	EXPECT_EQ(timeSeqCore.getVariable(variableName), 0.f);
	variableName = "after2";
	EXPECT_EQ(timeSeqCore.getVariable(variableName), 0.f);

	timeSeqCore.reset();
	timeSeqCore.process(1);

	variableName = "before1";
	EXPECT_EQ(timeSeqCore.getVariable(variableName), 0.f);
	variableName = "before2";
	EXPECT_EQ(timeSeqCore.getVariable(variableName), 0.f);
	variableName = "after1";
	EXPECT_EQ(timeSeqCore.getVariable(variableName), 3.f);
	variableName = "after2";
	EXPECT_EQ(timeSeqCore.getVariable(variableName), 4.f);
}

TEST(TimeSeqCore, ResetShouldClearTriggersBeforeResettingProcessor) {
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
	// Prime the core by starting it and run it for one cycle
	timeSeqCore.start(0);
	timeSeqCore.process(1);

	// Set the triggers that are there before the reset
	std::string triggerName = "before1";
	timeSeqCore.setTrigger(triggerName);
	triggerName = "before2";
	timeSeqCore.setTrigger(triggerName);
	timeSeqCore.process(1); // Do one process cycle so that the triggers become the active set

	EXPECT_CALL(*processor.get(), reset()).Times(1).WillOnce(testing::Invoke([&]() {
		std::string triggerName = "after1";
		timeSeqCore.setTrigger(triggerName);
		triggerName = "after2";
		timeSeqCore.setTrigger(triggerName);
	}));

	std::vector<std::string> triggers = { "before1", "before2" };
	EXPECT_EQ(timeSeqCore.getTriggers(), triggers);

	timeSeqCore.reset();
	timeSeqCore.process(1); // Do a process cycle so that the newly set triggers become the active trigger set
	triggers = { "after1", "after2" };
	EXPECT_EQ(timeSeqCore.getTriggers(), triggers);
}
