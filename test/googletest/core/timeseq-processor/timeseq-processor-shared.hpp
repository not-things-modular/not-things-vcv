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

struct MockSampleRateReader : SampleRateReader {
	MOCK_METHOD(float, getSampleRate, (), (override));
};

struct MockEventListener : EventListener {
	MOCK_METHOD(void, laneLooped, (), (override));
	MOCK_METHOD(void, segmentStarted, (), (override));
	MOCK_METHOD(void, triggerTriggered, (), (override));
};

struct MockAssertListener : AssertListener {
	MOCK_METHOD(void, assertFailed, (std::string, std::string, bool), (override));
};

struct MockRandValueGenerator : RandValueGenerator {
	MOCK_METHOD(float, generate, (float, float), (override));
};

static std::vector<std::string> mockDefaultTriggerHandlerEmptyTriggers;
#define MOCK_DEFAULT_TRIGGER_HANDLER(mockTriggerHandler) \
	ON_CALL(mockTriggerHandler, getTriggers).WillByDefault(testing::ReturnRef(mockDefaultTriggerHandlerEmptyTriggers));


pair<shared_ptr<Script>, shared_ptr<Processor>> loadProcessor(ProcessorLoader& processorLoader, json& json, vector<ValidationError> *validationErrors);
