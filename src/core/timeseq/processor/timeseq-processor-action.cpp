#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"

using namespace std;
using namespace timeseq;

ActionProcessor::ActionProcessor(const shared_ptr<IfProcessor>& ifProcessor) : m_ifProcessor(ifProcessor) {}

void ActionProcessor::process() {
	if ((!m_ifProcessor) || (m_ifProcessor->process(nullptr))) {
		processAction();
	}
}

ActionSetValueProcessor::ActionSetValueProcessor(const shared_ptr<ValueProcessor>& value, int outputPort, int outputChannel, PortHandler* portHandler, const shared_ptr<IfProcessor>& ifProcessor) : ActionProcessor(ifProcessor), m_value(value), m_outputPort(outputPort), m_outputChannel(outputChannel), m_portHandler(portHandler) {}

void ActionSetValueProcessor::processAction() {
	float value = m_value->process();
	m_portHandler->setOutputPortVoltage(m_outputPort, m_outputChannel, value);
}

ActionSetVariableProcessor::ActionSetVariableProcessor(const shared_ptr<ValueProcessor>& value, const string& name, VariableHandler* variableHandler, const shared_ptr<IfProcessor>& ifProcessor) : ActionProcessor(ifProcessor), m_value(value), m_name(name), m_variableHandler(variableHandler) {}

void ActionSetVariableProcessor::processAction() {
	float value = m_value->process();
	m_variableHandler->setVariable(m_name, value);
}

ActionSetPolyphonyProcessor::ActionSetPolyphonyProcessor(int outputPort, int channelCount, PortHandler* portHandler, const shared_ptr<IfProcessor>& ifProcessor) : ActionProcessor(ifProcessor), m_outputPort(outputPort), m_channelCount(channelCount), m_portHandler(portHandler) {}

void ActionSetPolyphonyProcessor::processAction() {
	m_portHandler->setOutputPortChannels(m_outputPort, m_channelCount);
}

ActionSetLabelProcessor::ActionSetLabelProcessor(int outputPort, const string& label, PortHandler* portHandler, const shared_ptr<IfProcessor>& ifProcessor) : ActionProcessor(ifProcessor), m_outputPort(outputPort), m_label(label), m_portHandler(portHandler) {}

void ActionSetLabelProcessor::processAction() {
	m_portHandler->setOutputPortLabel(m_outputPort, m_label);
}

ActionAssertProcessor::ActionAssertProcessor(const string& name, const shared_ptr<IfProcessor>& expect, bool stopOnFail, AssertListener* assertListener, const shared_ptr<IfProcessor>& ifProcessor) : ActionProcessor(ifProcessor), m_name(name), m_expect(expect), m_stopOnFail(stopOnFail), m_assertListener(assertListener) {}

void ActionAssertProcessor::processAction() {
	// First check the expectation without constructing a message to avoid performance impact
	if (!m_expect->process(nullptr)) {
		// If the expectation failed, re-execute it to generate the message
		string message;
		m_expect->process(&message);
		m_assertListener->assertFailed(m_name, message, m_stopOnFail);
	}
}

ActionTriggerProcessor::ActionTriggerProcessor(const string& trigger, TriggerHandler* triggerHandler, const shared_ptr<IfProcessor>& ifProcessor) : ActionProcessor(ifProcessor), m_trigger(trigger), m_triggerHandler(triggerHandler) {}

void ActionTriggerProcessor::processAction() {
	m_triggerHandler->setTrigger(m_trigger);
}

ActionMoveSequenceDirectionProcessor::ActionMoveSequenceDirectionProcessor(shared_ptr<SequencePositionProcessor>& sequencePositionProcessor, SequencePositionProcessor::SequenceMoveDirection direction, bool wrap, const shared_ptr<IfProcessor>& ifProcessor) : ActionProcessor(ifProcessor), m_sequencePositionProcessor(sequencePositionProcessor), m_direction(direction), m_wrap(wrap) {}

void ActionMoveSequenceDirectionProcessor::processAction() {
	m_sequencePositionProcessor->move(m_direction, m_wrap);
}

ActionMoveSequencePositionProcessor::ActionMoveSequencePositionProcessor(shared_ptr<SequencePositionProcessor>& sequencePositionProcessor, int position, const shared_ptr<IfProcessor>& ifProcessor) : ActionProcessor(ifProcessor), m_sequencePositionProcessor(sequencePositionProcessor), m_position(position) {}

void ActionMoveSequencePositionProcessor::processAction() {
	m_sequencePositionProcessor->move(m_position);
}

ActionClearSequenceProcessor::ActionClearSequenceProcessor(shared_ptr<SequencePositionProcessor>& sequencePositionProcessor, const shared_ptr<IfProcessor>& ifProcessor) : ActionProcessor(ifProcessor), m_sequencePositionProcessor(sequencePositionProcessor) {}

void ActionClearSequenceProcessor::processAction() {
	m_sequencePositionProcessor->getSequenceProcessor()->clear();
}

ActionAddToSequenceSequenceProcessor::ActionAddToSequenceSequenceProcessor(shared_ptr<SequencePositionProcessor>& sequencePositionProcessor, const shared_ptr<ValueProcessor>& value, int position, bool asConstantVoltage, const shared_ptr<IfProcessor>& ifProcessor) : ActionProcessor(ifProcessor), m_sequencePositionProcessor(sequencePositionProcessor), m_value(value), m_position(position), m_asConstantVoltage(asConstantVoltage) {}

void ActionAddToSequenceSequenceProcessor::processAction() {
	if (!m_asConstantVoltage) {
		m_sequencePositionProcessor->getSequenceProcessor()->add(m_value, m_position);
	} else {
		m_sequencePositionProcessor->getSequenceProcessor()->add(make_shared<StaticValueProcessor>((float) m_value->process(), vector<shared_ptr<CalcProcessor>>(), false), m_position);
	}
}

ActionRemoveFromSequenceProcessor::ActionRemoveFromSequenceProcessor(shared_ptr<SequencePositionProcessor>& sequencePositionProcessor, int position, const shared_ptr<IfProcessor>& ifProcessor) : ActionProcessor(ifProcessor), m_sequencePositionProcessor(sequencePositionProcessor), m_position(position) {}

void ActionRemoveFromSequenceProcessor::processAction() {
	m_sequencePositionProcessor->getSequenceProcessor()->remove(m_position);
}

ActionOngoingProcessor::ActionOngoingProcessor(const shared_ptr<IfProcessor>& ifProcessor) : m_ifProcessor(ifProcessor) {}

void ActionOngoingProcessor::start(uint64_t glideLength) {
	m_if = (!m_ifProcessor) || m_ifProcessor->process(nullptr);
}

bool ActionOngoingProcessor::shouldProcess() {
	return m_if;
}

ActionGlideProcessor::ActionGlideProcessor(
	float easeFactor,
	bool easePow,
	const shared_ptr<ValueProcessor>& startValue,
	const shared_ptr<ValueProcessor>& endValue,
	const shared_ptr<IfProcessor>& ifProcessor,
	int outputPort,
	int outputChannel,
	const string& variable,
	PortHandler* portHandler,
	VariableHandler* variableHandler) :
		ActionOngoingProcessor(ifProcessor), m_easeFactor(easeFactor), m_easePow(easePow), m_startValueProcessor(startValue), m_endValueProcessor(endValue), m_portHandler(portHandler), m_variableHandler(variableHandler), m_outputPort(outputPort), m_outputChannel(outputChannel), m_variable(variable) {

	if (!m_easePow) {
		// Multiply the ease factor if we're using the sigmoid function since it reacts slower to the easing factor when compared to the power-based algorithm.
		m_easeFactor *= 3.5f;
	}
}

void ActionGlideProcessor::start(uint64_t glideLength) {
	ActionOngoingProcessor::start(glideLength);

	if (shouldProcess()) {
		m_startValue = m_startValueProcessor->process();
		m_endValue = m_endValueProcessor->process();

		m_valueDelta = m_endValue - m_startValue;
		m_durationInverse = glideLength > 1. ? 1. / (glideLength - 1.) : 1.;
	}
}

void ActionGlideProcessor::process(uint64_t glidePosition) {
	if (shouldProcess()) {
		// The position coming from the DurationProcessor goes from 1 until duration.
		// For glide actions, we want the first call to be at position 0 so that the exact start value is used for the first iteration.
		// The glide will then run through the range in the process() until just before the exact end value, and the exact end value will be set when the end() method is called.
		float ease = m_durationInverse * (glidePosition - 1);
		if (m_easeFactor != 0.f) {
			if (m_easePow) {
				ease = calculatePowEase(ease);
			} else {
				ease = calculateSigEase(ease);
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
	if (shouldProcess()) {
		if (m_variable.length() > 0) {
			m_variableHandler->setVariable(m_variable, m_endValue);
		} else {
			m_portHandler->setOutputPortVoltage(m_outputPort, m_outputChannel, m_endValue);
		}
	}
}

double ActionGlideProcessor::calculatePowEase(float ease) {
	if (m_easeFactor > 0.f) {
		return pow(ease, 1.f + m_easeFactor * 2.f);
	} else {
		return 1.f - pow(1.f - (ease), 1.f - m_easeFactor * 2.f);
	}
}

double ActionGlideProcessor::calculateSigEase(float ease) {
	if (m_easeFactor > 0.f) {
		return ease / (1.0f + m_easeFactor * (1.0f - ease));
	} else {
		return 1.0f - ((1.0f - ease) / (1.0f - m_easeFactor * ease));
	}
}


ActionGateProcessor::ActionGateProcessor(float gateHighRatio, const shared_ptr<IfProcessor>& ifProcessor, int outputPort, int outputChannel, PortHandler* portHandler) :
	ActionOngoingProcessor(ifProcessor), m_portHandler(portHandler), m_outputPort(outputPort), m_outputChannel(outputChannel), m_gateHighRatio(gateHighRatio) {}

void ActionGateProcessor::start(uint64_t glideLength) {
	ActionOngoingProcessor::start(glideLength);

	if (shouldProcess()) {
		// There should be at least one high sample in the gate
		m_gateLowPosition = fmax(ceil(m_gateHighRatio * glideLength), 1.f);

		m_gateHigh = true;
		m_portHandler->setOutputPortVoltage(m_outputPort, m_outputChannel, 10.f);
	}
}

void ActionGateProcessor::process(uint64_t glidePosition) {
	if ((shouldProcess()) && (m_gateHigh)) {
		if (glidePosition > m_gateLowPosition) {
			m_gateHigh = false;
			m_portHandler->setOutputPortVoltage(m_outputPort, m_outputChannel, 0.f);
		}
	}
}

void ActionGateProcessor::end() {
	if ((shouldProcess()) && (m_gateHigh)) {
		m_gateHigh = false;
		m_portHandler->setOutputPortVoltage(m_outputPort, m_outputChannel, 0.f);
	}
}
