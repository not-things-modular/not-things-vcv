#pragma once

#define nt_private public

#include "core/timeseq-core.hpp"
#include "core/timeseq-processor.hpp"
#include "../timeseq-json/timeseq-json-shared.hpp"
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

struct MockTriggerHandler : TriggerHandler {
	MOCK_METHOD(std::vector<std::string>&, getTriggers, (), (override));
	MOCK_METHOD(void, setTrigger, (std::string), (override));
};

shared_ptr<Processor> loadProcessor(ProcessorLoader& processorLoader, json& json, vector<ValidationError> *validationErrors);
