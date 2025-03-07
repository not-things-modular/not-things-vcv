#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"
#include <sstream>
#include <stdarg.h>

using namespace std;
using namespace timeseq;


CalcProcessor::CalcProcessor(ScriptCalc *scriptCalc, shared_ptr<ValueProcessor> value) : m_scriptCalc(scriptCalc), m_value(value) {}

double CalcProcessor::calc(double value) {
	double calcValue = m_value->process();

	switch (m_scriptCalc->operation) {
		case ScriptCalc::ADD:
			return value + calcValue;
		case ScriptCalc::SUB:
			return value - calcValue;
		case ScriptCalc::MULT:
			return value * calcValue;
		case ScriptCalc::DIV:
			if (calcValue != 0.) {
				return value / calcValue;
			} else {
				return 0.;
			}
		default:
			// Shouldn't occur due to script parsing, but just to be sure...
			return value;
	}
}


ValueProcessor::ValueProcessor(vector<shared_ptr<CalcProcessor>> calcProcessors) : m_calcProcessors(calcProcessors) {}

double ValueProcessor::process() {
	double value = processValue();

	for (vector<shared_ptr<CalcProcessor>>::iterator it = m_calcProcessors.begin(); it != m_calcProcessors.end(); it++) {
		value = (*it)->calc(value);
	}

	return value;
}

StaticValueProcessor::StaticValueProcessor(float value, vector<shared_ptr<CalcProcessor>> calcProcessors) : ValueProcessor(calcProcessors), m_value(value) {}

double StaticValueProcessor::processValue() {
	return m_value;
}

InputValueProcessor::InputValueProcessor(int inputPort, int inputChannel, vector<shared_ptr<CalcProcessor>> calcProcessors, PortReader* portReader) : ValueProcessor(calcProcessors), m_inputPort(inputPort), m_inputChannel(inputChannel), m_portReader(portReader) {}

double InputValueProcessor::processValue() {
	return m_portReader->getInputPortVoltage(m_inputPort, m_inputChannel);
}

OutputValueProcessor::OutputValueProcessor(int outputPort, int outputChannel, vector<shared_ptr<CalcProcessor>> calcProcessors, PortReader* portReader) : ValueProcessor(calcProcessors), m_outputPort(outputPort), m_outputChannel(outputChannel), m_portReader(portReader) {}

double OutputValueProcessor::processValue() {
	return m_portReader->getOutputPortVoltage(m_outputPort, m_outputChannel);
}

RandValueProcessor::RandValueProcessor(shared_ptr<ValueProcessor> lowerValue, shared_ptr<ValueProcessor> upperValue, vector<shared_ptr<CalcProcessor>> calcProcessors) : ValueProcessor(calcProcessors), m_lowerValue(lowerValue), m_upperValue(upperValue) {}

double RandValueProcessor::processValue() {
	return 0.f;
}

ActionSetValueProcessor::ActionSetValueProcessor(shared_ptr<ValueProcessor> value, int outputPort, int outputChannel, PortWriter* portWriter) : m_value(value), m_outputPort(outputPort), m_outputChannel(outputChannel), m_portWriter(portWriter) {}

void ActionSetValueProcessor::process() {
	float value = m_value->process();
	m_portWriter->setOutputPortVoltage(m_outputPort, m_outputChannel, value);
}

ActionSetPolyphonyProcessor::ActionSetPolyphonyProcessor(int outputPort, int channelCount, PortWriter* portWriter) : m_outputPort(outputPort), m_channelCount(channelCount), m_portWriter(portWriter) {}

void ActionSetPolyphonyProcessor::process() {
	m_portWriter->setOutputPortChannels(m_outputPort, m_channelCount);
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

TriggerProcessor::TriggerProcessor(string id, int inputPort, int inputChannel, PortReader* portReader) : m_id(id), m_inputPort(inputPort), m_inputChannel(inputChannel), m_portReader(portReader) {}

Processor::Processor(vector<shared_ptr<TimelineProcessor>> timelines, vector<shared_ptr<TriggerProcessor>> triggers, vector<shared_ptr<ActionProcessor>> startActions, vector<shared_ptr<ActionProcessor>> endActions) : m_timelines(timelines), m_triggers(triggers), m_startActions(startActions), m_endActions(endActions) {}

void Processor::reset() {
	for (std::vector<std::shared_ptr<ActionProcessor>>::iterator it = m_startActions.begin(); it != m_startActions.end(); it++) {
		(*it)->process();
	}
}

void Processor::process() {

}

ProcessorLoader::ProcessorLoader(PortReader* portReader, SampleRateReader* sampleRateReader, PortWriter* portWriter) : m_processorScriptParser(portReader, sampleRateReader, portWriter) {
}

shared_ptr<Processor> ProcessorLoader::loadScript(shared_ptr<Script> script, vector<ValidationError> *validationErrors) {
	return m_processorScriptParser.parseScript(script.get(), validationErrors, vector<string>());
}
