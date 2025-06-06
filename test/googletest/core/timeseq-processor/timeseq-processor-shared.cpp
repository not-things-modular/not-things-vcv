#include "timeseq-processor-shared.hpp"

pair<shared_ptr<Script>, shared_ptr<Processor>> loadProcessor(ProcessorLoader& processorLoader, json& json, vector<ValidationError> *validationErrors) {
	JsonLoader jsonLoader;
	shared_ptr<Script> script = loadScript(jsonLoader, json, validationErrors);
	if (validationErrors->size() == 0) {
		return { script, processorLoader.loadScript(script, validationErrors) };
	} else {
		return { script, nullptr };
	}
}
