#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"
#include <sstream>
#include <stdarg.h>
#include <chrono>

using namespace std;
using namespace timeseq;


CalcValueProcessor::CalcValueProcessor(ScriptCalc *scriptCalc, shared_ptr<ValueProcessor> value) : m_value(value) {
	switch (scriptCalc->operation) {
		case ScriptCalc::ADD:
			m_operation = ValueCalcOperation::ADD;
			break;
		case ScriptCalc::SUB:
			m_operation = ValueCalcOperation::SUB;
			break;
		case ScriptCalc::DIV:
			m_operation = ValueCalcOperation::DIV;
			break;
		case ScriptCalc::MULT:
			m_operation = ValueCalcOperation::MULT;
			break;
		case ScriptCalc::MAX:
			m_operation = ValueCalcOperation::MAX;
			break;
		case ScriptCalc::MIN:
			m_operation = ValueCalcOperation::MIN;
			break;
		case ScriptCalc::REMAIN:
			m_operation = ValueCalcOperation::REMAIN;
			break;
		case ScriptCalc::TRUNC:
		case ScriptCalc::FRAC:
		case ScriptCalc::ROUND:
		case ScriptCalc::QUANTIZE:
		case ScriptCalc::SIGN:
		case ScriptCalc::VTOF:
			// Should not happen in this constructor.
			break;
	}
}

double CalcValueProcessor::calc(double value) {
	double calcValue = m_value->process();

	switch (m_operation) {
		case ValueCalcOperation::ADD:
			return value + calcValue;
		case ValueCalcOperation::SUB:
			return value - calcValue;
		case ValueCalcOperation::MULT:
			return value * calcValue;
		case ValueCalcOperation::DIV:
			if (calcValue != 0.f) {
				return value / calcValue;
			} else {
				return 0.;
			}
		case ValueCalcOperation::MAX:
			return value > calcValue ? value : calcValue;
		case ValueCalcOperation::MIN:
			return value < calcValue ? value : calcValue;
		case ValueCalcOperation::REMAIN:
			if (calcValue != 0.f) {
				return fmod(value, calcValue);
			} else {
				return 0.f;
			}
	}

	// All value-based calculations should have already been handled in the switch...
	return value;
}

double CalcTruncProcessor::calc(double value) {
	return trunc(value);
}

double CalcFracProcessor::calc(double value) {
	float x;
	return modf(value, &x);
}

CalcRoundProcessor::CalcRoundProcessor(ScriptCalc* scriptCalc) : m_scriptCalc(scriptCalc) {}

double CalcRoundProcessor::calc(double value) {
	switch (*m_scriptCalc->roundType) {
		case ScriptCalc::RoundType::UP:
			return ceil(value);
		case ScriptCalc::RoundType::DOWN:
			return floor(value);
		case ScriptCalc::RoundType::NEAR:
			return round(value);
	}

	// All rounding types should have been covered in the switch...
	return value;
}

CalcQuantizeProcessor::CalcQuantizeProcessor(ScriptTuning* scriptTuning) {
	vector<float> tuningValues;

	if (scriptTuning->notes.size() == 1) {
		// If there is only one note in the tuning, add the note one octave lower, the note itself, and the note one octave higher
		tuningValues.push_back(scriptTuning->notes[0] - 1.f);
		tuningValues.push_back(scriptTuning->notes[0]);
		tuningValues.push_back(scriptTuning->notes[0] + 1.f);
	} else {
		// First create a list of all the tuning notes with the last one added at the front one octave lower
		// And the first one added at the back one octave higher
		tuningValues.push_back(scriptTuning->notes.back() - 1.f);
		tuningValues.insert(tuningValues.end(), scriptTuning->notes.begin(), scriptTuning->notes.end());
		tuningValues.push_back(scriptTuning->notes.front() + 1.f);
	}

	// Based on the tuningValues list, determine all the quantization points as being halfway between each tuning value
	for (unsigned int i = 0; i < tuningValues.size() - 1; i++) {
		float boundary = tuningValues[i] + ((tuningValues[i + 1] - tuningValues[i]) / 2.f);
		m_quantizeValues.push_back({ boundary, tuningValues[i]});
	}
	// Add in the last tuningValue entry as a round-up entry
	m_quantizeValues.push_back({ 2.f, tuningValues.back()});
}

double CalcQuantizeProcessor::calc(double value) {
	float integral;
	float fract = modf(value, &integral);

	if (fract < 0.f) {
		fract += 1.f;
		integral -= 1.f;
	}

	for (vector<array<float, 2>>::iterator it = m_quantizeValues.begin(); it != m_quantizeValues.end(); it++) {
		if (fract < (*it)[0]) {
			return integral + (*it)[1];
		}
	}

	// Shouldn't reach this point...
	return value;
}

CalcSignProcessor::CalcSignProcessor(ScriptCalc* scriptCalc) : m_positive(*scriptCalc->signType == ScriptCalc::SignType::POS) {}

double CalcSignProcessor::calc(double value) {
	if (((m_positive) && (value < 0)) || ((!m_positive) && (value > 0))) {
		return -value;
	} else {
		return value;
	}
}

double CalcVtoFProcessor::calc(double value) {
	return pow(2, value) * 261.6256f;
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
const float quantize_thresholds[][2] = {
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
	{ 1.f, 1.f }, // C
};

double ValueProcessor::quantize(double value) {
	double octave;
	double note = modf(value, &octave);

	if (note < 0.f) {
		note += 1.f;
		octave -= 1;
	}

	for (int i = 0; i < 13; i++) {
		if (note < quantize_thresholds[i][0]) {
			note = quantize_thresholds[i][1];
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

RandValueGenerator::RandValueGenerator() : m_generator(chrono::steady_clock::now().time_since_epoch().count()) {}
RandValueGenerator::~RandValueGenerator() {}

float RandValueGenerator::generate(float lower, float upper) {
	// uniform_real_distribution expects lower < upper, so make sure to handle possible edge cases.
	if (lower == upper) {
		return lower;
	} else if (lower < upper) {
		uniform_real_distribution<float> distribution(lower, upper);
		return distribution(m_generator);
	} else {
		uniform_real_distribution<float> distribution(upper, lower);
		return distribution(m_generator);
	}
}

RandValueProcessor::RandValueProcessor(shared_ptr<ValueProcessor> lowerValue, shared_ptr<ValueProcessor> upperValue, shared_ptr<RandValueGenerator> randValueGenerator, vector<shared_ptr<CalcProcessor>> calcProcessors, bool quantize) : ValueProcessor(calcProcessors, quantize), m_lowerValue(lowerValue), m_upperValue(upperValue), m_randValueGenerator(randValueGenerator) {}

double RandValueProcessor::processValue() {
	float lower = m_lowerValue->process();
	float upper = m_upperValue->process();

	return m_randValueGenerator->generate(lower, upper);
}

IfProcessor::IfProcessor(ScriptIf* scriptIf, pair<shared_ptr<ValueProcessor>, shared_ptr<ValueProcessor>> values, pair<shared_ptr<IfProcessor>, shared_ptr<IfProcessor>> ifs) : m_scriptIf(scriptIf), m_values(values), m_ifs(ifs) {}

bool IfProcessor::process(string* message) {
	if (message == nullptr) {
		// If no message needs to be returned upon failure, we can do a simple check for the different operator types
		switch (m_scriptIf->ifOperator) {
			case ScriptIf::IfOperator::EQ: {
				if (m_scriptIf->tolerance) {
					return fabs(m_values.first->process() - m_values.second->process()) <= *m_scriptIf->tolerance.get();
				} else {
					return m_values.first->process() == m_values.second->process();
				}
			}
			case ScriptIf::IfOperator::NE: {
				if (m_scriptIf->tolerance) {
					return fabs(m_values.first->process() - m_values.second->process()) > *m_scriptIf->tolerance.get();
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
				return m_ifs.first->process(nullptr) || m_ifs.second->process(nullptr);
		}

		return false;
	} else {
		// We'll need to return the details of the comparison if it failed, so we'll need to do some additional work...
		ostringstream oss;
		oss.precision(10);
		if (m_scriptIf->ifOperator == ScriptIf::IfOperator::AND) {
			string message1;
			string message2;
			bool if1 = m_ifs.first->process(&message1);
			bool if2 = m_ifs.second->process(&message2);

			oss << "(" << message1 << " and " << message2 << ")";
			*message = oss.str();

			if (if1 && if2) {
				// Normally, we shouldn't arrive here anymore since the assert action will first verify the expectation without requesting the detailed message
				return true;
			} else {
				return false;
			}
		} else if (m_scriptIf->ifOperator == ScriptIf::IfOperator::OR) {
			string message1;
			string message2;
			bool if1 = m_ifs.first->process(&message1);
			bool if2 = m_ifs.second->process(&message2);

			oss << "(" << message1 << " or " << message2 << ")";
			*message = oss.str();

			if (if1 || if2) {
				// Normally, we shouldn't arrive here anymore since the assert action will first verify the expectation without requesting the detailed message
				return true;
			} else {
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
						result = fabs(value1 - value2) <= *m_scriptIf->tolerance.get();
					} else {
						result = value1 == value2;
					}
					operatorName = " eq ";
					break;
				}
				case ScriptIf::IfOperator::NE: {
					if (m_scriptIf->tolerance) {
						result = fabs(value1 - value2) > *m_scriptIf->tolerance.get();
					} else {
						result = value1 != value2;
					}
					operatorName = " ne ";
					break;
				}
				case ScriptIf::IfOperator::LT: {
					result = value1 < value2;
					operatorName = " lt ";
					break;
				}
				case ScriptIf::IfOperator::LTE: {
					result = value1 <= value2;
					operatorName = " lte ";
					break;
				}
				case ScriptIf::IfOperator::GT: {
					result = value1 > value2;
					operatorName = " gt ";
					break;
				}
				case ScriptIf::IfOperator::GTE: {
					result = value1 >= value2;
					operatorName = " gte ";
					break;
				}
				case ScriptIf::IfOperator::AND:
				case ScriptIf::IfOperator::OR: {
						// Shouldn't come here anymore, AND and OR have been handled earlier
					break;
				}
			}

			oss << "(" << value1 << operatorName << value2 << ")";
			*message = oss.str();

			return result;
		}
	}
}

ActionProcessor::ActionProcessor(shared_ptr<IfProcessor> ifProcessor) : m_ifProcessor(ifProcessor) {}

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

ActionSetLabelProcessor::ActionSetLabelProcessor(int outputPort, string label, PortHandler* portHandler, shared_ptr<IfProcessor> ifProcessor) : ActionProcessor(ifProcessor), m_outputPort(outputPort), m_label(label), m_portHandler(portHandler) {}

void ActionSetLabelProcessor::processAction() {
	m_portHandler->setOutputPortLabel(m_outputPort, m_label);
}

ActionAssertProcessor::ActionAssertProcessor(string name, shared_ptr<IfProcessor> expect, bool stopOnFail, AssertListener* assertListener, shared_ptr<IfProcessor> ifProcessor) : ActionProcessor(ifProcessor), m_name(name), m_expect(expect), m_stopOnFail(stopOnFail), m_assertListener(assertListener) {}

void ActionAssertProcessor::processAction() {
	// First check the expectation without constructing a message to avoid performance impact
	if (!m_expect->process(nullptr)) {
		// If the expectation failed, re-execute it to generate the message
		string message;
		m_expect->process(&message);
		m_assertListener->assertFailed(m_name, message, m_stopOnFail);
	}
}

ActionTriggerProcessor::ActionTriggerProcessor(string trigger, TriggerHandler* triggerHandler, shared_ptr<IfProcessor> ifProcessor) : ActionProcessor(ifProcessor), m_trigger(trigger), m_triggerHandler(triggerHandler) {}

void ActionTriggerProcessor::processAction() {
	m_triggerHandler->setTrigger(m_trigger);
}

ActionOngoingProcessor::ActionOngoingProcessor(shared_ptr<IfProcessor> ifProcessor) : m_ifProcessor(ifProcessor) {}

void ActionOngoingProcessor::start(uint64_t glideLength) {
	m_if = (!m_ifProcessor) || m_ifProcessor->process(nullptr);
}

bool ActionOngoingProcessor::shouldProcess() {
	return m_if;
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
	if (shouldProcess()) {
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


ActionGateProcessor::ActionGateProcessor(float gateHighRatio, shared_ptr<IfProcessor> ifProcessor, int outputPort, int outputChannel, PortHandler* portHandler) :
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

void DurationProcessor::setDuration(uint64_t duration) {
	m_duration = duration;
}

void DurationProcessor::setDrift(double drift) {
	m_drift = drift;
}

DurationConstantProcessor::DurationConstantProcessor(uint64_t duration, double drift) {
	setDuration(duration);
	setDrift(drift);
}

void DurationConstantProcessor::prepareForStart() {}


DurationVariableFactorProcessor::DurationVariableFactorProcessor(std::shared_ptr<ValueProcessor> value, double samplesFactor) : m_value(value), m_samplesFactor(samplesFactor) {}

void DurationVariableFactorProcessor::prepareForStart() {
	double value = m_value->process();
	double refactoredValue = (m_samplesFactor != 1.f) ? value * m_samplesFactor : value;
	
	if (refactoredValue >= 1.f) {
		uint64_t duration = floor(refactoredValue);
		setDuration(duration);
		setDrift(refactoredValue - duration);
	} else {
		setDuration(1);
		setDrift(0.);
	}
}


DurationVariableHzProcessor::DurationVariableHzProcessor(std::shared_ptr<ValueProcessor> value, double sampleRate) : m_value(value), m_sampleRate(sampleRate) {}

void DurationVariableHzProcessor::prepareForStart() {
	double value = m_value->process();
	double refactoredValue = m_sampleRate / value;
	
	if (refactoredValue >= 1.f) {
		uint64_t duration = floor(refactoredValue);
		setDuration(duration);
		setDrift(refactoredValue - duration);
	} else {
		setDuration(1);
		setDrift(0.);
	}
}


SegmentProcessor::SegmentProcessor(SegmentProcessor& segmentProcessor) :
	m_scriptSegment(segmentProcessor.m_scriptSegment),
	m_duration(segmentProcessor.m_duration),
	m_startActions(segmentProcessor.m_startActions),
	m_endActions(segmentProcessor.m_endActions),
	m_ongoingActions(segmentProcessor.m_ongoingActions),
	m_eventListener(segmentProcessor.m_eventListener) {
}

SegmentProcessor::SegmentProcessor(
	ScriptSegment* scriptSegment,
	shared_ptr<DurationProcessor> duration,
	vector<shared_ptr<ActionProcessor>> startActions,
	vector<shared_ptr<ActionProcessor>> endActions,
	vector<shared_ptr<ActionOngoingProcessor>> ongoingActions,
	EventListener* eventListener) :
		m_scriptSegment(scriptSegment), m_duration(duration), m_startActions(startActions), m_endActions(endActions), m_ongoingActions(ongoingActions), m_eventListener(eventListener) {}

void SegmentProcessor::pushStartActions(vector<shared_ptr<ActionProcessor>> startActions) {
	m_startActions.insert(m_startActions.begin(), startActions.begin(), startActions.end());
}

void SegmentProcessor::pushEndActions(vector<shared_ptr<ActionProcessor>> endActions) {
	m_endActions.insert(m_endActions.end(), endActions.begin(), endActions.end());
}

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
		m_duration->prepareForStart();
		starting = true; // The glide actions will have to be processed from their start position.
	}

	drift = m_duration->process(drift);

	switch (m_duration->getState()) {
		case DurationProcessor::DurationState::STATE_START:
			// Shouldn't occur since duration processing will move us away from the start state
			break;
		case DurationProcessor::DurationState::STATE_PROGRESS:
			processOngoingActions(starting, false);
			break;
		case DurationProcessor::DurationState::STATE_END:
			processOngoingActions(starting, true);
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

void SegmentProcessor::processOngoingActions(bool start, bool end) {
	for (vector<shared_ptr<ActionOngoingProcessor>>::iterator it = m_ongoingActions.begin(); it != m_ongoingActions.end(); it++) {
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
				// The active segment is finished, so move to the next segment
				m_activeSegment++;
				segment++;

				// Check that we haven't reached the end of the segment list
				if (segment != m_segments.end()) {
					// Reset the newly activated segment so it is at its start position (e.g. if a reset occurred)
					(*segment)->reset();
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

			// If there is a first segment, make sure it is at its starting position
			if (m_segments.size() > 0) {
				m_segments[0]->reset();
			}

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

	// If there is a first segment, make sure it is at its starting position
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

Processor::Processor(shared_ptr<Script> script, vector<shared_ptr<TimelineProcessor>> timelines, vector<shared_ptr<TriggerProcessor>> triggers, vector<shared_ptr<ActionProcessor>> startActions) : m_timelines(timelines), m_triggers(triggers), m_startActions(startActions), m_script(script) {}

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
