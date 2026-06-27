#include "timeseq-processor-shared.hpp"

TEST(TimeSeqProcessorRefParsing, RefInputShouldCorrectlyRestoreHierarchy) {
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, nullptr, &mockSampleRateReader, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
                { { "set-value", { { "output", 1 }, { "value", { { "input", { { "ref", "input-1" } } } } } } } },
                { { "set-value", { { "output", 2 }, { "value", { { "input", { { "ref", "input-2" } } } } } } } },
			}) } } }) } }
		}) } }
	});
	json["component-pool"] = {
		{ "inputs", json::array({
			{ { "id", "input-1" }, { "index", 1 } }
		})}
	};

    pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	ASSERT_GE(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/1/set-value/value/input");
}

TEST(TimeSeqProcessorRefParsing, RefOutputShouldCorrectlyRestoreHierarchy) {
	MockSampleRateReader mockSampleRateReader;
	ProcessorLoader processorLoader(nullptr, nullptr, nullptr, &mockSampleRateReader, nullptr, nullptr);
	vector<ValidationError> validationErrors;
	json json = getMinimalJson();
	json["timelines"] = json::array({
		{ { "lanes", json::array({
			{ { "segments", json::array({ { { "duration", { { "samples", 1 } } }, { "actions", json::array({
                { { "set-value", { { "output", { { "ref", "output-1" } } }, { "value", { { "input", 1 } } } } } },
                { { "set-value", { { "output", { { "ref", "output-2" } } }, { "value", { { "input", 2 } } } } } },
			}) } } }) } }
		}) } }
	});
	json["component-pool"] = {
		{ "outputs", json::array({
			{ { "id", "output-1" }, { "index", 1 } }
		})}
	};

    pair<shared_ptr<Script>, shared_ptr<Processor>> script = loadProcessor(processorLoader, json, validationErrors);
	ASSERT_GE(validationErrors.size(), 1u);
	expectError(validationErrors, ValidationErrorCode::Ref_NotFound, "/timelines/0/lanes/0/segments/0/actions/1/set-value/output");
}
