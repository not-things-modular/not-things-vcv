#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorSetLabel, SetLabelActionShouldUpdateOutputLabel) {
	MockEventListener mockEventListener;
	MockTriggerHandler mockTriggerHandler;
	MockSampleRateReader mockSampleRateReader;
	MockPortHandler mockPortHandler;
	ProcessorLoader processorLoader(&mockPortHandler, nullptr, &mockTriggerHandler, &mockSampleRateReader, &mockEventListener, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "loop", true }, { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
				{ { "set-label", { { "index", 3 }, { "label", "this-is-a-label" } } } } 
			}) } } }) } },
		}) } }
	});

	pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, &validationErrors);
	EXPECT_NO_ERRORS(validationErrors);

	vector<string> emptyTriggers = {};
	{
		testing::InSequence inSequence;

		for (int i = 0; i < 5; i++) {
			EXPECT_CALL(mockTriggerHandler, getTriggers()).Times(1).WillOnce(testing::ReturnRef(emptyTriggers));
			EXPECT_CALL(mockEventListener, segmentStarted()).Times(1);
			string label = "this-is-a-label";
			EXPECT_CALL(mockPortHandler, setOutputPortLabel(2, label));
		}
	}
	for (int i = 0; i < 5; i++) {
		script.second->process();
	}
}
