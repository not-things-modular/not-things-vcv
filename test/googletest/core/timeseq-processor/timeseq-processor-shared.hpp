#pragma once

#define nt_private public

#include "core/timeseq-processor.hpp"
#include "../timeseq-json/timeseq-json-shared.hpp"

shared_ptr<Processor> loadProcessor(ProcessorLoader& processorLoader, json& json, vector<ValidationError> *validationErrors);
