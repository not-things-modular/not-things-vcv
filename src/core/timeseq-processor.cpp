#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include <sstream>
#include <stdarg.h>

using namespace std;
using namespace timeseq;


float ValueProcessor::processValue() {
	return 0.f;
}

CalcProcessor::CalcProcessor(ScriptCalc *scriptCalc, std::shared_ptr<ValueProcessor> value) : m_scriptCalc(scriptCalc), m_value(value) {}

ValueProcessor::ValueProcessor(vector<shared_ptr<CalcProcessor>> calcProcessors) : m_calcProcessors(calcProcessors) {}

StaticValueProcessor::StaticValueProcessor(float value, vector<shared_ptr<CalcProcessor>> calcProcessors) : ValueProcessor(calcProcessors), m_value(value) {}

float StaticValueProcessor::processValue() {
	return m_value;
}

InputValueProcessor::InputValueProcessor(int inputPort, int inputChannel, vector<shared_ptr<CalcProcessor>> calcProcessors) : ValueProcessor(calcProcessors), m_inputPort(inputPort), m_inputChannel(inputChannel) {}

float InputValueProcessor::processValue() {
	return 0.f;
}

OutputValueProcessor::OutputValueProcessor(int outputPort, int outputChannel, vector<shared_ptr<CalcProcessor>> calcProcessors) : ValueProcessor(calcProcessors), m_outputPort(outputPort), m_outputChannel(outputChannel) {}

float OutputValueProcessor::processValue() {
	return 0.f;
}

RandValueProcessor::RandValueProcessor(shared_ptr<ValueProcessor> lowerValue, shared_ptr<ValueProcessor> upperValue, vector<shared_ptr<CalcProcessor>> calcProcessors) : ValueProcessor(calcProcessors), m_lowerValue(lowerValue), m_upperValue(upperValue) {}

float RandValueProcessor::processValue() {
	return 0.f;
}

ActionSetValueProcessor::ActionSetValueProcessor(shared_ptr<ValueProcessor> value, int outputPort, int outputChannel) : m_value(value), m_outputPort(outputPort), m_outputChannel(outputChannel) {}

void ActionSetValueProcessor::process() {
}

ActionSetPolyphonyProcessor::ActionSetPolyphonyProcessor(int outputPort, int channelCount) : m_outputPort(outputPort), m_channelCount(channelCount) {}

void ActionSetPolyphonyProcessor::process() {

}

ActionTriggerProcessor::ActionTriggerProcessor(string trigger) : m_trigger(trigger) {}

void ActionTriggerProcessor::process() {

}

ActionGlideProcessor::ActionGlideProcessor(shared_ptr<ValueProcessor> startValue, shared_ptr<ValueProcessor> endValue) : m_startValueProcessor(startValue), m_endValueProcessor(endValue) {}

DurationProcessor::DurationProcessor(uint64_t duration, double drift) : m_duration(duration), m_drift(drift) {}

SegmentProcessor::SegmentProcessor(
	ScriptSegment* scriptSegment,
	shared_ptr<DurationProcessor> duration,
	vector<shared_ptr<ActionProcessor>> startActions,
	vector<shared_ptr<ActionProcessor>> endActions,
	vector<shared_ptr<ActionGlideProcessor>> glideActions) :
		m_scriptSegment(scriptSegment), m_duration(duration), m_startActions(startActions), m_endActions(endActions), m_glideActions(glideActions) {}

LaneProcessor::LaneProcessor(ScriptLane* scriptLane, vector<shared_ptr<SegmentProcessor>> segments) : m_scriptLane(scriptLane), m_segments(segments) {}

TimelineProcessor::TimelineProcessor(
	ScriptTimeline* scriptTimeline,
	vector<shared_ptr<LaneProcessor>> laneProcessors,
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> startTriggers,
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> stopTriggers) :
		m_scriptTimeline(scriptTimeline), m_laneProcessors(laneProcessors), m_startTriggers(startTriggers), m_stopTriggers(stopTriggers) {}

TriggerProcessor::TriggerProcessor(string id, int inputPort, int inputChannel, PortReader* portReader) : m_portReader(portReader), m_id(id), m_inputPort(inputPort), m_inputChannel(inputChannel) {}

Processor::Processor(vector<shared_ptr<TimelineProcessor>> timelines, vector<shared_ptr<TriggerProcessor>> triggers) : m_timelines(timelines), m_triggers(triggers) {}

void Processor::process() {

}

ProcessorLoader::ProcessorLoader(PortReader* portReader, SampleRateReader* sampleRateReader, PortWriter* portWriter) : m_processorScriptParser(portReader, sampleRateReader, portWriter) {
}

shared_ptr<Processor> ProcessorLoader::loadScript(shared_ptr<Script> script, vector<ValidationError> *validationErrors) {
	return m_processorScriptParser.parseScript(script.get(), validationErrors, vector<string>());
}
