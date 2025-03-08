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

VariableValueProcessor::VariableValueProcessor(string name, vector<shared_ptr<CalcProcessor>> calcProcessors, VariableHandler* variableHandler) : ValueProcessor(calcProcessors), m_name(name), m_variableHandler(variableHandler) {}

double VariableValueProcessor::processValue() {
	return m_variableHandler->getVariable(m_name);
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

ActionSetVariableProcessor::ActionSetVariableProcessor(shared_ptr<ValueProcessor> value, string name, VariableHandler* variableHandler) : m_value(value), m_name(name), m_variableHandler(variableHandler) {}

void ActionSetVariableProcessor::process() {
	float value = m_value->process();
	m_variableHandler->setVariable(m_name, value);
}

ActionSetPolyphonyProcessor::ActionSetPolyphonyProcessor(int outputPort, int channelCount, PortWriter* portWriter) : m_outputPort(outputPort), m_channelCount(channelCount), m_portWriter(portWriter) {}

void ActionSetPolyphonyProcessor::process() {
	m_portWriter->setOutputPortChannels(m_outputPort, m_channelCount);
}

ActionTriggerProcessor::ActionTriggerProcessor(string trigger) : m_trigger(trigger) {}

void ActionTriggerProcessor::process() {

}

ActionGlideProcessor::ActionGlideProcessor(
	shared_ptr<ValueProcessor> startValue,
	shared_ptr<ValueProcessor> endValue,
	int outputPort,
	int outputChannel,
	string variable,
	PortWriter* portWriter,
	VariableHandler* variableHandler) :
		m_startValueProcessor(startValue), m_endValueProcessor(endValue), m_portWriter(portWriter), m_variableHandler(variableHandler), m_outputPort(outputPort), m_outputChannel(outputChannel), m_variable(variable) {}

void ActionGlideProcessor::start() {
	m_startValue = m_startValueProcessor->process();
	m_endValue = m_endValueProcessor->process();
}

void ActionGlideProcessor::process(uint64_t glidePosition, uint64_t glideLength) {
	double value = m_startValue + ((m_endValue - m_startValue) * glidePosition / glideLength);
	if (m_variable.length() > 0) {

	} else {
		m_portWriter->setOutputPortVoltage(m_outputPort, m_outputChannel, value);
	}
}

DurationProcessor::DurationProcessor(uint64_t duration, double drift) : m_duration(duration), m_drift(drift) {}

DurationProcessor::State DurationProcessor::getState() {
	return m_state;
}

uint64_t DurationProcessor::getPosition() {
	return m_position;
}

uint64_t DurationProcessor::getDuration() {
	return m_duration;
}

double DurationProcessor::process(double drift) {
	if (m_state == State::STATE_IDLE) {
		// The segment is being started.
		m_state = State::STATE_START;
		// Reset the position
		m_position = 0;
		// Add the drift at the start of the segment, so that the end of the segment will wait if we went over 1 total drift
		return drift + m_drift;
	} else if (m_position < m_duration - 1) {
		// The segment isn't done yet, move to the next position
		m_state = State::STATE_PROGRESS;
		m_position++;
		return drift;
	} else if (drift >= 1.) {
		// The segment is done, but we drifted with more then one step, so correct the drift now by waiting one step
		return drift - 1;
	} else {
		// The segment is done, and the drift is below a step. Move to the final position and change to the end state
		m_state = State::STATE_END;
		m_position++;
		return drift;
	}
}

void DurationProcessor::reset() {
	m_state = State::STATE_IDLE;
	m_position = 0;
}

SegmentProcessor::SegmentProcessor(
	ScriptSegment* scriptSegment,
	shared_ptr<DurationProcessor> duration,
	vector<shared_ptr<ActionProcessor>> startActions,
	vector<shared_ptr<ActionProcessor>> endActions,
	vector<shared_ptr<ActionGlideProcessor>> glideActions) :
		m_scriptSegment(scriptSegment), m_duration(duration), m_startActions(startActions), m_endActions(endActions), m_glideActions(glideActions) {}

DurationProcessor::State SegmentProcessor::getState() {
	return m_duration->getState();
}

double SegmentProcessor::process(double drift) {
	drift = m_duration->process(drift);
	
	switch (m_duration->getState()) {
		case DurationProcessor::State::STATE_START:
			processStartActions();
			processGlideActions(true);
			break;
		case DurationProcessor::State::STATE_PROGRESS:
			processGlideActions(false);
			break;
		case DurationProcessor::State::STATE_END:
			processGlideActions(false);
			processEndActions();
			break;
		case DurationProcessor::State::STATE_IDLE:
			// Shouldn't occur: the m_duration::process call will always move away from the idle state
			break;
	}

	return drift;
}

void SegmentProcessor::reset() {
	m_duration->reset();
}

void SegmentProcessor::processStartActions() {
	for (vector<shared_ptr<ActionProcessor>>::iterator it = m_startActions.begin(); it != m_startActions.end(); it++) {
		(*it)->process();
	}
}

void SegmentProcessor::processEndActions() {
	for (vector<shared_ptr<ActionProcessor>>::iterator it = m_endActions.begin(); it != m_endActions.end(); it++) {
		(*it)->process();
	}
}

void SegmentProcessor::processGlideActions(bool start) {
	for (vector<shared_ptr<ActionGlideProcessor>>::iterator it = m_glideActions.begin(); it != m_glideActions.end(); it++) {
		if (start) {
			(*it)->start();
		}
		(*it)->process(m_duration->getPosition(), m_duration->getDuration());
	}
}

LaneProcessor::LaneProcessor(ScriptLane* scriptLane, vector<shared_ptr<SegmentProcessor>> segments) : m_scriptLane(scriptLane), m_segments(segments) {
	if ((m_scriptLane->autoStart) && (m_segments.size() > 0)) {
		m_active = true;
	}
}

bool LaneProcessor::process() {
	bool stopped = false;

	if ((m_active) && (m_segments.size() > 0)) {
		std::vector<std::shared_ptr<SegmentProcessor>>::iterator segment = m_segments.begin() + m_activeSegment;
		DurationProcessor::State state = (*segment)->getState();
		switch (state) {
			case DurationProcessor::State::STATE_IDLE:
			case DurationProcessor::State::STATE_START:
			case DurationProcessor::State::STATE_PROGRESS: {
				// The active segment can do further processing
				m_drift = (*segment)->process(m_drift);
				break;
			}
			case DurationProcessor::State::STATE_END: {
				// The active segment is finished, so reset it and move to the next segment
				(*segment)->reset();
				m_activeSegment++;
				segment++;

				// Check that we haven't reached the end of the segment list
				if (segment != m_segments.end()) {
					// Invoke process on the newly activated segment.
					m_drift = (*segment)->process(m_drift);
				} else {
					// If we reached the end of the segment list, see if we need to loop.
					if ((m_scriptLane->loop) || (m_scriptLane->repeat > 1 && m_activeRepeats < m_scriptLane->repeat)) {
						// Loop to the start of the segment list and increase the repeat count
						if (segment == m_segments.end()) {
							segment = m_segments.begin();
						}
						m_activeRepeats++;
					} else {
						// We've reached the end of this lane, so reset everything and de-active the lane.
						m_activeSegment = 0;
						m_activeRepeats = 0;
						m_drift = 0.;
						m_active = false;
						stopped = true;
					}
				}
				break;
			}
		}
	}

	return stopped;
}

TimelineProcessor::TimelineProcessor(
	ScriptTimeline* scriptTimeline,
	vector<shared_ptr<LaneProcessor>> lanes,
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> startTriggers,
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> stopTriggers) :
		m_scriptTimeline(scriptTimeline), m_lanes(lanes), m_startTriggers(startTriggers), m_stopTriggers(stopTriggers) {}

void TimelineProcessor::process() {
	for (vector<shared_ptr<LaneProcessor>>::iterator it = m_lanes.begin(); it != m_lanes.end(); it++) {
		(*it)->process();
	}
}

TriggerProcessor::TriggerProcessor(string id, int inputPort, int inputChannel, PortReader* portReader) : m_id(id), m_inputPort(inputPort), m_inputChannel(inputChannel), m_portReader(portReader) {}

Processor::Processor(vector<shared_ptr<TimelineProcessor>> timelines, vector<shared_ptr<TriggerProcessor>> triggers, vector<shared_ptr<ActionProcessor>> startActions, vector<shared_ptr<ActionProcessor>> endActions) : m_timelines(timelines), m_triggers(triggers), m_startActions(startActions), m_endActions(endActions) {}

void Processor::reset() {
	for (vector<shared_ptr<ActionProcessor>>::iterator it = m_startActions.begin(); it != m_startActions.end(); it++) {
		(*it)->process();
	}
}

void Processor::process() {
	for (vector<shared_ptr<TimelineProcessor>>::iterator it = m_timelines.begin(); it != m_timelines.end(); it++) {
		(*it)->process();
	}
}

ProcessorLoader::ProcessorLoader(PortReader* portReader, SampleRateReader* sampleRateReader, PortWriter* portWriter, VariableHandler* variableHandler) : m_processorScriptParser(portReader, sampleRateReader, portWriter, variableHandler) {}

shared_ptr<Processor> ProcessorLoader::loadScript(shared_ptr<Script> script, vector<ValidationError> *validationErrors) {
	return m_processorScriptParser.parseScript(script.get(), validationErrors, vector<string>());
}
