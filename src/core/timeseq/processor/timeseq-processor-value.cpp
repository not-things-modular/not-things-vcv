#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"

using namespace std;
using namespace timeseq;

CalcValueProcessor::CalcValueProcessor(const ScriptCalc *scriptCalc, shared_ptr<ValueProcessor> value) : m_value(value) {
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
	double x;
	return modf(value, &x);
}

CalcRoundProcessor::CalcRoundProcessor(const ScriptCalc* scriptCalc) : m_scriptCalc(scriptCalc) {}

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

CalcQuantizeProcessor::CalcQuantizeProcessor(const ScriptTuning* scriptTuning) {
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
		m_quantizeValues.push_back({ boundary, tuningValues[i] });
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

CalcSignProcessor::CalcSignProcessor(const ScriptCalc* scriptCalc) : m_positive(*scriptCalc->signType == ScriptCalc::SignType::POS) {}

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

	for (const shared_ptr<CalcProcessor>& calcProcessor : m_calcProcessors) {
		value = calcProcessor->calc(value);
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

SequenceValueProcessor::SequenceValueProcessor(shared_ptr<SequencePositionProcessor> sequencePositionProcessor, SequencePositionProcessor::SequenceMoveDirection moveBefore, SequencePositionProcessor::SequenceMoveDirection moveAfter, bool wrap, vector<shared_ptr<CalcProcessor>> calcProcessors, bool quantize) : ValueProcessor(calcProcessors, quantize), m_sequencePositionProcessor(sequencePositionProcessor), m_moveBefore(moveBefore), m_moveAfter(moveAfter), m_wrap(wrap) {}

double SequenceValueProcessor::processValue() {
	if (m_moveBefore != SequencePositionProcessor::SequenceMoveDirection::NONE) {
		m_sequencePositionProcessor->move(m_moveBefore, m_wrap);
	}

	double value = m_sequencePositionProcessor->getCurrentValue();

	if (m_moveAfter != SequencePositionProcessor::SequenceMoveDirection::NONE) {
		m_sequencePositionProcessor->move(m_moveAfter, m_wrap);
	}

	return value;
}
