#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorAssertAction, AssertActionWithNoStopOnFailShouldStopOnFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockAssertListener mockAssertListener;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" },
						{ "expect", { { "eq", json::array({
							{ { "voltage", 1.f } },
							{ { "voltage", 0.f } }
						}) } } }
					} }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
	EXPECT_CALL(mockAssertListener, assertFailed("the-assert", "(1 eq 0)", true)).Times(1);

	script.second->process();
}

TEST(TimeSeqProcessorAssertAction, AssertActionWithStopOnFailShouldStopOnFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockAssertListener mockAssertListener;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", true },
						{ "expect", { { "eq", json::array({
							{ { "voltage", 1.f } },
							{ { "voltage", 0.f } }
						}) } } }
					} }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
	EXPECT_CALL(mockAssertListener, assertFailed("the-assert", "(1 eq 0)", true)).Times(1);

	script.second->process();
}

TEST(TimeSeqProcessorAssertAction, AssertActionWithStopOnFailFalseShouldNotStopOnFail) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockAssertListener mockAssertListener;
	ProcessorLoader processorLoader(nullptr, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, &mockAssertListener);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{
					{ "assert", { { "name", "the-assert" }, { "stop-on-fail", false },
						{ "expect", { { "eq", json::array({
							{ { "voltage", 1.f } },
							{ { "voltage", 0.f } }
						}) } } }
					} }
				}
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	expectNoErrors(validationErrors);

	vector<string> emptyTriggers = {};
	EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
	EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
	EXPECT_CALL(mockAssertListener, assertFailed("the-assert", "(1 eq 0)", false)).Times(1);

	script.second->process();
}
