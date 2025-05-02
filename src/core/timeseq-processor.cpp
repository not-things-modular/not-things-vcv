#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"
#include <sstream>
#include <stdarg.h>
#include <chrono>

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


ValueProcessor::ValueProcessor(vector<shared_ptr<CalcProcessor>> calcProcessors, bool quantize) : m_calcProcessors(calcProcessors), m_quantize(quantize) {}

double ValueProcessor::process() {
	double value = processValue();

	for (vector<shared_ptr<CalcProcessor>>::iterator it = m_calcProcessors.begin(); it != m_calcProcessors.end(); it++) {
		value = (*it)->calc(value);
	}

	if (m_quantize) {
		value = quantize(value);
	}

	return value;
}

// The quantizing thresholds within an octave for quantizing to the nearest note.
// The first value is halfway between two quantized notes, the second value is the quantized note that is below it.
// Any value that is below the first value should be quantized down to the note that's in the second value.
const float quantize_treshholds[][2] = {
	{ 1.f / 24, 0.f }, // C
	{ (1.f / 12) + (1.f / 24), (1.f / 12) }, // C#
	{ (2.f / 12) + (1.f / 24), (2.f / 12) }, // D
	{ (3.f / 12) + (1.f / 24), (3.f / 12) }, // Eb
	{ (4.f / 12) + (1.f / 24), (4.f / 12) }, // E
	{ (5.f / 12) + (1.f / 24), (5.f / 12) }, // F
	{ (6.f / 12) + (1.f / 24), (6.f / 12) }, // F#
	{ (7.f / 12) + (1.f / 24), (7.f / 12) }, // G
	{ (8.f / 12) + (1.f / 24), (8.f / 12) }, // Ab
	{ (9.f / 12) + (1.f / 24), (9.f / 12) }, // A
	{ (10.f / 12) + (1.f / 24), (10.f / 12) }, // Bb
	{ (11.f / 12) + (1.f / 24), (11.f / 12) }, // B
};

double ValueProcessor::quantize(double value) {
	double octave;
	double note = std::modf(value, &octave);

	for (int i = 0; i < 12; i++) {
		if (note < quantize_treshholds[i][0]) {
			note = quantize_treshholds[i][1];
			break;
		}
	}

	return octave + note;
}

StaticValueProcessor::StaticValueProcessor(float value, vector<shared_ptr<CalcProcessor>> calcProcessors, bool quantize) : ValueProcessor(calcProcessors, quantize), m_value(value) {}

double StaticValueProcessor::processValue() {
	return m_value;
}

VariableValueProcessor::VariableValueProcessor(string name, vector<shared_ptr<CalcProcessor>> calcProcessors, bool quantize, VariableHandler* variableHandler) : ValueProcessor(calcProcessors, quantize), m_name(name), m_variableHandler(variableHandler) {}

double VariableValueProcessor::processValue() {
	return m_variableHandler->getVariable(m_name);
}

InputValueProcessor::InputValueProcessor(int inputPort, int inputChannel, vector<shared_ptr<CalcProcessor>> calcProcessors, bool quantize, PortHandler* portHandler) : ValueProcessor(calcProcessors, quantize), m_inputPort(inputPort), m_inputChannel(inputChannel), m_portHandler(portHandler) {}

double InputValueProcessor::processValue() {
	return m_portHandler->getInputPortVoltage(m_inputPort, m_inputChannel);
}

OutputValueProcessor::OutputValueProcessor(int outputPort, int outputChannel, vector<shared_ptr<CalcProcessor>> calcProcessors, bool quantize, PortHandler* portHandler) : ValueProcessor(calcProcessors, quantize), m_outputPort(outputPort), m_outputChannel(outputChannel), m_portHandler(portHandler) {}

double OutputValueProcessor::processValue() {
	return m_portHandler->getOutputPortVoltage(m_outputPort, m_outputChannel);
}

RandValueProcessor::RandValueProcessor(shared_ptr<ValueProcessor> lowerValue, shared_ptr<ValueProcessor> upperValue, vector<shared_ptr<CalcProcessor>> calcProcessors, bool quantize) : ValueProcessor(calcProcessors, quantize), m_lowerValue(lowerValue), m_upperValue(upperValue), m_generator(chrono::steady_clock::now().time_since_epoch().count()) {}

double RandValueProcessor::processValue() {
	float lower = m_lowerValue->process();
	float upper = m_upperValue->process();

	uniform_real_distribution<float> distribution(lower, upper);

	return distribution(m_generator);
}

IfProcessor::IfProcessor(ScriptIf* scriptIf, pair<shared_ptr<ValueProcessor>, shared_ptr<ValueProcessor>> values, pair<shared_ptr<IfProcessor>, shared_ptr<IfProcessor>> ifs) : m_scriptIf(scriptIf), m_values(values), m_ifs(ifs) {}

bool IfProcessor::process(string* message) {
	if (message == nullptr) {
		// If no message needs to be returned upon failure, we can do a simple check for the different operator types
		switch (m_scriptIf->ifOperator) {
			case ScriptIf::IfOperator::EQ: {
				if (m_scriptIf->tolerance) {
					return std::fabs(m_values.first->process() - m_values.second->process()) <= *m_scriptIf->tolerance.get();
				} else {
					return m_values.first->process() == m_values.second->process();
				}
			}
			case ScriptIf::IfOperator::NE: {
				if (m_scriptIf->tolerance) {
					return std::fabs(m_values.first->process() - m_values.second->process()) > *m_scriptIf->tolerance.get();
				} else {
					return m_values.first->process() != m_values.second->process();
				}
			}
			case ScriptIf::IfOperator::LT:
				return m_values.first->process() < m_values.second->process();
			case ScriptIf::IfOperator::LTE:
				return m_values.first->process() <= m_values.second->process();
			case ScriptIf::IfOperator::GT:
				return m_values.first->process() > m_values.second->process();
			case ScriptIf::IfOperator::GTE:
				return m_values.first->process() >= m_values.second->process();
			case ScriptIf::IfOperator::AND:
				return m_ifs.first->process(nullptr) && m_ifs.second->process(nullptr);
			case ScriptIf::IfOperator::OR:
				return m_ifs.second->process(nullptr) || m_ifs.second->process(nullptr);
		}

		return false;
	} else {
		// We'll need to return the details of the comparison if it failed, so we'll need to do some additional work...
		std::ostringstream oss;
		if (m_scriptIf->ifOperator == ScriptIf::IfOperator::AND) {
			string message1;
			string message2;
			bool if1 = m_ifs.first->process(&message1);
			bool if2 = m_ifs.second->process(&message2);

			if (if1 && if2) {
				return true;
			} else {
				oss << "(" << message1 << " and " << message2 << ")";
				*message = oss.str();
				return false;
			}
		} else if (m_scriptIf->ifOperator == ScriptIf::IfOperator::OR) {
			string message1;
			string message2;
			bool if1 = m_ifs.first->process(&message1);
			bool if2 = m_ifs.second->process(&message2);

			if (if1 || if2) {
				return true;
			} else {
				oss << "(" << message1 << " or " << message2 << ")";
				*message = oss.str();
				return false;
			}
		} else {
			double value1 = m_values.first->process();
			double value2 = m_values.second->process();
			string operatorName;
			bool result = false;

			switch (m_scriptIf->ifOperator) {
				case ScriptIf::IfOperator::EQ: {
					if (m_scriptIf->tolerance) {
						result = std::fabs(m_values.first->process() - m_values.second->process()) <= *m_scriptIf->tolerance.get();
					} else {
						result = m_values.first->process() == m_values.second->process();
					}
					operatorName = " eq ";
					break;
				}
				case ScriptIf::IfOperator::NE: {
					if (m_scriptIf->tolerance) {
						result = std::fabs(m_values.first->process() - m_values.second->process()) > *m_scriptIf->tolerance.get();
					} else {
						result = m_values.first->process() != m_values.second->process();
					}
					operatorName = " ne ";
					break;
				}
				case ScriptIf::IfOperator::LT: {
					result = m_values.first->process() < m_values.second->process();
					operatorName = " lt ";
					break;
				}
				case ScriptIf::IfOperator::LTE: {
					result = m_values.first->process() <= m_values.second->process();
					operatorName = " lte ";
					break;
				}
				case ScriptIf::IfOperator::GT: {
					result = m_values.first->process() > m_values.second->process();
					operatorName = " gt ";
					break;
				}
				case ScriptIf::IfOperator::GTE: {
					result = m_values.first->process() >= m_values.second->process();
					operatorName = " gte ";
					break;
				}
				case ScriptIf::IfOperator::AND:
				case ScriptIf::IfOperator::OR: {
						// Shouldn't come here anymore, AND and OR have been handled earlier
					break;
				}
			}

			if (!result) {
				oss << "(" << value1 << operatorName << value2 << ")";
				*message = oss.str();
			}
			return result;
		}
	}
}

ActionProcessor::ActionProcessor(std::shared_ptr<IfProcessor> ifProcessor) : m_ifProcessor(ifProcessor) {}

void ActionProcessor::process() {
	if ((!m_ifProcessor) || (m_ifProcessor->process(nullptr))) {
		processAction();
	}
}

ActionSetValueProcessor::ActionSetValueProcessor(shared_ptr<ValueProcessor> value, int outputPort, int outputChannel, PortHandler* portHandler, shared_ptr<IfProcessor> ifProcessor) : ActionProcessor(ifProcessor), m_value(value), m_outputPort(outputPort), m_outputChannel(outputChannel), m_portHandler(portHandler) {}

void ActionSetValueProcessor::processAction() {
	float value = m_value->process();
	m_portHandler->setOutputPortVoltage(m_outputPort, m_outputChannel, value);
}

ActionSetVariableProcessor::ActionSetVariableProcessor(shared_ptr<ValueProcessor> value, string name, VariableHandler* variableHandler, shared_ptr<IfProcessor> ifProcessor) : ActionProcessor(ifProcessor), m_value(value), m_name(name), m_variableHandler(variableHandler) {}

void ActionSetVariableProcessor::processAction() {
	float value = m_value->process();
	m_variableHandler->setVariable(m_name, value);
}

ActionSetPolyphonyProcessor::ActionSetPolyphonyProcessor(int outputPort, int channelCount, PortHandler* portHandler, shared_ptr<IfProcessor> ifProcessor) : ActionProcessor(ifProcessor), m_outputPort(outputPort), m_channelCount(channelCount), m_portHandler(portHandler) {}

void ActionSetPolyphonyProcessor::processAction() {
	m_portHandler->setOutputPortChannels(m_outputPort, m_channelCount);
}

ActionAssertProcessor::ActionAssertProcessor(string name, shared_ptr<IfProcessor> expect, bool stopOnFail, AssertListener* assertListener, shared_ptr<IfProcessor> ifProcessor) : ActionProcessor(ifProcessor), m_name(name), m_expect(expect), m_stopOnFail(stopOnFail), m_assertListener(assertListener) {}

void ActionAssertProcessor::processAction() {
	string message;
	if (!m_expect->process(&message)) {
		m_assertListener->assertFailed(m_name, message, m_stopOnFail);
	}
}

ActionTriggerProcessor::ActionTriggerProcessor(string trigger, TriggerHandler* triggerHandler, shared_ptr<IfProcessor> ifProcessor) : ActionProcessor(ifProcessor), m_trigger(trigger), m_triggerHandler(triggerHandler) {}

void ActionTriggerProcessor::processAction() {
	m_triggerHandler->setTrigger(m_trigger);
}

ActionGlideProcessor::ActionGlideProcessor(
	float easeFactor,
	bool easePow,
	shared_ptr<ValueProcessor> startValue,
	shared_ptr<ValueProcessor> endValue,
	shared_ptr<IfProcessor> ifProcessor,
	int outputPort,
	int outputChannel,
	string variable,
	PortHandler* portHandler,
	VariableHandler* variableHandler) :
		m_easeFactor(easeFactor), m_easePow(easePow), m_startValueProcessor(startValue), m_endValueProcessor(endValue), m_ifProcessor(ifProcessor), m_portHandler(portHandler), m_variableHandler(variableHandler), m_outputPort(outputPort), m_outputChannel(outputChannel), m_variable(variable) {

	if (!m_easePow) {
		// Multiply the ease factor if we're using the sigmoid function since it reacts slower to the easing factor when compared to the power-based algorithm.
		m_easeFactor *= 3.5f;
	}
}

void ActionGlideProcessor::start(uint64_t glideLength) {
	m_startValue = m_startValueProcessor->process();
	m_endValue = m_endValueProcessor->process();
	m_if = (!m_ifProcessor) || m_ifProcessor->process(nullptr);

	m_valueDelta = m_endValue - m_startValue;
	m_durationInverse = 1. / glideLength;
}

void ActionGlideProcessor::process(uint64_t glidePosition) {
	if (m_if) {
		// The position coming from the DurationProcessor goes from 1 until duration.
		// For glide actions, we want the first call to be at position 0 so that the exact start value is used for the first iteration.
		// The glide will then run through the range in the process() until just before the exact end value, and the exact end value will be set when the end() method is called.
		float ease = m_durationInverse * (glidePosition - 1);
		if (m_easeFactor != 0.f) {
			if (m_easePow) {
				ease = calculatePowEase(ease, (glidePosition - 1));
			} else {
				ease = calculateSigEase(ease, (glidePosition - 1));
			}
		}

		double value = m_startValue + m_valueDelta * ease;
		if (m_variable.length() > 0) {
			m_variableHandler->setVariable(m_variable, value);
		} else {
			m_portHandler->setOutputPortVoltage(m_outputPort, m_outputChannel, value);
		}
	}
}

void ActionGlideProcessor::end() {
	if (m_if) {
		if (m_variable.length() > 0) {
			m_variableHandler->setVariable(m_variable, m_endValue);
		} else {
			m_portHandler->setOutputPortVoltage(m_outputPort, m_outputChannel, m_endValue);
		}
	}
}

double ActionGlideProcessor::calculatePowEase(float ease, uint64_t glidePosition) {
	if (m_easeFactor > 0.f) {
		return pow(ease, 1.f + m_easeFactor * 2.f);
	} else {
		return 1.f - pow(1.f - (ease), 1.f - m_easeFactor * 2.f);
	}
}

double ActionGlideProcessor::calculateSigEase(float ease, uint64_t glidePosition) {
	if (m_easeFactor > 0.f) {
		return ease / (1.0f + m_easeFactor * (1.0f - ease));
	} else {
		return 1.0f - ((1.0f - ease) / (1.0f - m_easeFactor * ease));
	}
}


DurationProcessor::DurationProcessor(uint64_t duration, double drift) : m_duration(duration), m_drift(drift) {}

DurationProcessor::DurationState DurationProcessor::getState() {
	return m_state;
}

uint64_t DurationProcessor::getPosition() {
	return m_position;
}

uint64_t DurationProcessor::getDuration() {
	return m_duration;
}

double DurationProcessor::process(double drift) {
	if (m_state == DurationState::STATE_START) {
		m_position = 0;
		drift += m_drift;
	}

	if (m_position < m_duration - 1) {
		// The segment isn't done yet, move to the next position
		m_state = DurationState::STATE_PROGRESS;
		m_position++;
		return drift;
	} else if (drift >= 1.) {
		// The segment is done, but we drifted with more then one step, so correct the drift now by waiting one step
		m_state = DurationState::STATE_PROGRESS;
		return drift - 1;
	} else {
		// The segment is done, and the drift is below a step. Move to the final position and change to the end state
		m_state = DurationState::STATE_END;
		m_position++;
		return drift;
	}
}

void DurationProcessor::reset() {
	m_state = DurationState::STATE_START;
	m_position = 0;
}

SegmentProcessor::SegmentProcessor(
	ScriptSegment* scriptSegment,
	shared_ptr<DurationProcessor> duration,
	vector<shared_ptr<ActionProcessor>> startActions,
	vector<shared_ptr<ActionProcessor>> endActions,
	vector<shared_ptr<ActionGlideProcessor>> glideActions,
	EventListener* eventListener) :
		m_scriptSegment(scriptSegment), m_duration(duration), m_startActions(startActions), m_endActions(endActions), m_glideActions(glideActions), m_eventListener(eventListener) {}

DurationProcessor::DurationState SegmentProcessor::getState() {
	return m_duration->getState();
}

double SegmentProcessor::process(double drift) {
	bool starting = false;

	// Trigger the start actions if we're at the start of the segment
	if (m_duration->getState() == DurationProcessor::DurationState::STATE_START) {
		if (!m_scriptSegment->disableUi) {
			m_eventListener->segmentStarted();
		}
		processStartActions();
		starting = true; // The glide actions will have to be processed from their start position.
	}

	drift = m_duration->process(drift);

	switch (m_duration->getState()) {
		case DurationProcessor::DurationState::STATE_START:
			// Shouldn't occur since duration processing will move us away from the start state
			break;
		case DurationProcessor::DurationState::STATE_PROGRESS:
			processGlideActions(starting, false);
			break;
		case DurationProcessor::DurationState::STATE_END:
			processGlideActions(false, true);
			processEndActions();
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

void SegmentProcessor::processGlideActions(bool start, bool end) {
	for (vector<shared_ptr<ActionGlideProcessor>>::iterator it = m_glideActions.begin(); it != m_glideActions.end(); it++) {
		if (start) {
			(*it)->start(m_duration->getDuration());
		}

		if (end) {
			(*it)->end();
		} else {
			(*it)->process(m_duration->getPosition());
		}
	}
}

LaneProcessor::LaneProcessor(ScriptLane* scriptLane, vector<shared_ptr<SegmentProcessor>> segments, EventListener* eventListener) : m_scriptLane(scriptLane), m_segments(segments), m_eventListener(eventListener) {
	reset();
}

LaneProcessor::LaneState LaneProcessor::getState() {
	return m_state;
}

bool LaneProcessor::process() {
	bool stopped = false;

	if ((m_state == LaneState::STATE_PROCESSING) && (m_segments.size() > 0)) {
		vector<shared_ptr<SegmentProcessor>>::iterator segment = m_segments.begin() + m_activeSegment;
		DurationProcessor::DurationState state = (*segment)->getState();
		switch (state) {
			case DurationProcessor::DurationState::STATE_START:
			case DurationProcessor::DurationState::STATE_PROGRESS: {
				// The active segment can do further processing
				m_drift = (*segment)->process(m_drift);
				break;
			}
			case DurationProcessor::DurationState::STATE_END: {
				// The active segment is finished, so reset it and move to the next segment
				(*segment)->reset();
				m_activeSegment++;
				segment++;

				// Check that we haven't reached the end of the segment list
				if (segment != m_segments.end()) {
					// Invoke process on the newly activated segment.
					m_drift = (*segment)->process(m_drift);
				} else {
					// We reached the end, so stop this lane and wait for the timeline processor to trigger looping if needed
					m_state = LaneState::STATE_PENDING_LOOP;
					stopped = true;
				}
				break;
			}
		}
	}

	return stopped;
}

void LaneProcessor::loop() {
	if (m_state == LaneState::STATE_PENDING_LOOP) {
		// Check if we need to loop or repeat
		if ((m_scriptLane->loop) || (m_scriptLane->repeat > 1 && m_repeatCount < m_scriptLane->repeat - 1)) {
			m_repeatCount++;
			m_activeSegment = 0;
			m_state = LaneState::STATE_PROCESSING;

			if (!m_scriptLane->disableUi) {
				m_eventListener->laneLooped();
			}

			process();
		}
	}
}

void LaneProcessor::reset() {
	m_repeatCount = 0;
	m_activeSegment = 0;
	m_drift = 0.;

	if (m_segments.size() > 0) {
		m_segments[0]->reset();
	}

	if ((m_scriptLane->autoStart) && (m_segments.size() > 0)) {
		m_state = LaneState::STATE_PROCESSING;
	} else {
		m_state = LaneState::STATE_IDLE;
	}
}

void LaneProcessor::processTriggers(vector<string>& triggers) {
	// No use in starting if we have no segments...
	if (m_segments.size() > 0) {
		// Restarts must be done no matter the current state
		if ((m_scriptLane->restartTrigger.length() > 0) && (find(triggers.begin(), triggers.end(), m_scriptLane->restartTrigger) != triggers.end())) {
			reset();
			m_state = LaneState::STATE_PROCESSING;
		} else if (m_state != LaneState::STATE_PROCESSING) {
			// Starts must only be done if we're not already running
			if ((m_scriptLane->startTrigger.length() > 0) && (find(triggers.begin(), triggers.end(), m_scriptLane->startTrigger) != triggers.end())) {
				reset();
				m_state = LaneState::STATE_PROCESSING;
			}
		}
	}

	if ((m_state != LaneState::STATE_IDLE) && (m_scriptLane->stopTrigger.length() > 0) && (find(triggers.begin(), triggers.end(), m_scriptLane->stopTrigger) != triggers.end())) {
		m_state = LaneState::STATE_IDLE;
	}

}

TimelineProcessor::TimelineProcessor(
	ScriptTimeline* scriptTimeline,
	vector<shared_ptr<LaneProcessor>> lanes,
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> startTriggers,
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> stopTriggers,
	TriggerHandler* triggerHandler) :
		m_scriptTimeline(scriptTimeline), m_lanes(lanes), m_startTriggers(startTriggers), m_stopTriggers(stopTriggers), m_triggerHandler(triggerHandler) {}

void TimelineProcessor::process() {
	bool checkLoop = false;

	// Check if any lane start or stop triggers were fired
	vector<string>& triggers = m_triggerHandler->getTriggers();
	if (triggers.size() > 0) {
		for (vector<shared_ptr<LaneProcessor>>::iterator it = m_lanes.begin(); it != m_lanes.end(); it++) {
			it->get()->processTriggers(triggers);
		}
	}

	// Call process on all lanes
	for (vector<shared_ptr<LaneProcessor>>::iterator it = m_lanes.begin(); it != m_lanes.end(); it++) {
		bool stopped = (*it)->process();
		if (stopped) {
			if (m_scriptTimeline->loopLock) {
				// There is a loop-lock, so check looping for all lanes after we processed them all
				checkLoop = true;
			} else {
				// There is no loop-lock, so the lane should immediately check if it needs to loop
				(*it)->loop();
			}
		}
	}

	if (checkLoop) {
		// Check if all lanes are either idle or pending a loop
		bool loop = true;
		for (vector<shared_ptr<LaneProcessor>>::iterator it = m_lanes.begin(); it != m_lanes.end(); it++) {
			if ((*it)->getState() == LaneProcessor::LaneState::STATE_PROCESSING) {
				// There is one still processing, so don't loop yet
				loop = false;
				break;
			}
		}

		if (loop) {
			for (vector<shared_ptr<LaneProcessor>>::iterator it = m_lanes.begin(); it != m_lanes.end(); it++) {
				(*it)->loop();
			}
		}
	}
}

void TimelineProcessor::reset() {
	for (vector<shared_ptr<LaneProcessor>>::iterator it = m_lanes.begin(); it != m_lanes.end(); it++) {
		(*it)->reset();
	}
}

TriggerProcessor::TriggerProcessor(string id, int inputPort, int inputChannel, PortHandler* portHandler, TriggerHandler* triggerHandler) : m_id(id), m_inputPort(inputPort), m_inputChannel(inputChannel), m_portHandler(portHandler), m_triggerHandler(triggerHandler) {}

void TriggerProcessor::process() {
	if (m_trigger.process(m_portHandler->getInputPortVoltage(m_inputPort, m_inputChannel), 0.f, 1.f)) {
		m_triggerHandler->setTrigger(m_id);
	}
}

Processor::Processor(vector<shared_ptr<TimelineProcessor>> timelines, vector<shared_ptr<TriggerProcessor>> triggers, vector<shared_ptr<ActionProcessor>> startActions) : m_timelines(timelines), m_triggers(triggers), m_startActions(startActions) {}

void Processor::reset() {
	for (vector<shared_ptr<ActionProcessor>>::iterator it = m_startActions.begin(); it != m_startActions.end(); it++) {
		(*it)->process();
	}

	for (vector<shared_ptr<TimelineProcessor>>::iterator it = m_timelines.begin(); it != m_timelines.end(); it++) {
		(*it)->reset();
	}
}

void Processor::process() {
	for (vector<shared_ptr<TimelineProcessor>>::iterator it = m_timelines.begin(); it != m_timelines.end(); it++) {
		(*it)->process();
	}

	for (vector<shared_ptr<TriggerProcessor>>::iterator it = m_triggers.begin(); it != m_triggers.end(); it++) {
		(*it)->process();
	}
}
