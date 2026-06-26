#include "core/timeseq-processor-parser.hpp"
#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"
#include "util/notes.hpp"

using namespace std;
using namespace timeseq;

inline SequencePositionProcessor::SequenceMoveDirection convertScriptSequenceMoveDirection(ScriptSequenceMoveDirection scriptDirection) {
	switch (scriptDirection) {
		case FORWARD:
			return SequencePositionProcessor::SequenceMoveDirection::FORWARD;
		case BACKWARD:
			return SequencePositionProcessor::SequenceMoveDirection::BACKWARD;
		case RANDOM:
			return SequencePositionProcessor::SequenceMoveDirection::RANDOM;
		case NONE:
			return SequencePositionProcessor::SequenceMoveDirection::NONE;
	}
	// Not really needed, but otherwise the compiler gives a warning
	return SequencePositionProcessor::SequenceMoveDirection::NONE;
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseValue(ScriptValue* scriptValue, vector<string> valueStack) {
	// Check if it's a ref segment block object or a full one
	if (scriptValue->ref.length() == 0) {
		int count = 0;
		m_context.location.push_back("calc");
		vector<shared_ptr<CalcProcessor>> calcProcessors;
		for (vector<ScriptCalc>::iterator it = scriptValue->calc.begin(); it != scriptValue->calc.end(); it++) {
			m_context.location.push_back(to_string(count));
			calcProcessors.push_back(parseCalc(&(*it), valueStack));
			m_context.location.pop_back();
			count++;
		}
		m_context.location.pop_back();

		if ((scriptValue->voltage) || (scriptValue->note)) {
			return parseStaticValue(scriptValue, calcProcessors);
		} else if (scriptValue->variable) {
			return parseVariableValue(scriptValue, calcProcessors);
		} else if (scriptValue->input) {
			return parseInputValue(scriptValue, calcProcessors);
		} else if (scriptValue->output) {
			return parseOutputValue(scriptValue, calcProcessors);
		} else if (scriptValue->rand) {
			return parseRandValue(scriptValue, calcProcessors, valueStack);
		} else if (scriptValue->sequence) {
			return parseSequenceValue(scriptValue, calcProcessors, valueStack);
		}

	} else {
		if (find(valueStack.begin(), valueStack.end(), string("v-") + scriptValue->ref) == valueStack.end()) {
			int count = 0;
			for (vector<ScriptValue>::iterator it = m_context.script->values.begin(); it != m_context.script->values.end(); it++) {
				if (scriptValue->ref.compare(it->id) == 0) {
					m_context.stashLocation();
					m_context.location = { "component-pool",  "values", to_string(count) };
					valueStack.push_back(string("v-") + scriptValue->ref);
					shared_ptr<ValueProcessor> value = parseValue(&(*it), valueStack);
					valueStack.pop_back();
					m_context.popLocation();
					return value;
				}
				count++;
			}

			// Couldn't find the referenced value...
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced value with id '", scriptValue->ref.c_str(), "' in the script values.");
		} else {
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_CircularFound, "Encountered a circular value reference while processing the value with the id '", scriptValue->ref.c_str(), "'. Circular references can not be resolved.");
		}
	}

	return shared_ptr<ValueProcessor>();
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseStaticValue(ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors) {
	float value = 0.f;
	if (scriptValue->voltage) {
		value = *scriptValue->voltage.get();
	} else if (scriptValue->note) {
		string note = *scriptValue->note.get();
		int noteIndex = noteNameToIndex(note[0]);
		if (note.length() > 2) {
			if (note[2] == '-') {
				noteIndex -= 1;
			} else if (note[2] == '+') {
				noteIndex += 1;
			}
		}
		value = note[1] - '0' - 4 + ((float) noteIndex / 12);
	}
	return make_shared<StaticValueProcessor>(value, calcProcessors, scriptValue->quantize);
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseVariableValue(ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors) {
	return make_shared<VariableValueProcessor>(*scriptValue->variable.get(), calcProcessors, scriptValue->quantize, m_variableHandler);
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseInputValue(ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors) {
	m_context.location.push_back("input");
	pair<int, int> input = parseInput(scriptValue->input.get());
	m_context.location.pop_back();

	return make_shared<InputValueProcessor>(input.first, input.second, calcProcessors, scriptValue->quantize, m_portHandler);
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseOutputValue(ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors) {
	m_context.location.push_back("output");
	pair<int, int> output = parseOutput(scriptValue->output.get());
	m_context.location.pop_back();

	return make_shared<OutputValueProcessor>(output.first, output.second, calcProcessors, scriptValue->quantize, m_portHandler);
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseRandValue(ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors, vector<string> valueStack) {
	m_context.location.push_back("rand");

	m_context.location.push_back("lower");
	shared_ptr<ValueProcessor> lowerValueProcessor = parseValue(scriptValue->rand.get()->lower.get(), valueStack);
	m_context.location.pop_back();

	m_context.location.push_back("upper");
	shared_ptr<ValueProcessor> upperValueProcessor = parseValue(scriptValue->rand.get()->upper.get(), valueStack);
	m_context.location.pop_back();

	m_context.location.pop_back();

	return make_shared<RandValueProcessor>(lowerValueProcessor, upperValueProcessor, m_randomValueGenerator, calcProcessors, scriptValue->quantize);
}

shared_ptr<ValueProcessor> ProcessorScriptParser::parseSequenceValue(ScriptValue* scriptValue, vector<shared_ptr<CalcProcessor>>& calcProcessors, vector<string> valueStack) {
	m_context.location.push_back("sequence");

	shared_ptr<ValueProcessor> processor = nullptr;

	ScriptSequenceValue *sequence = &(*scriptValue->sequence);
	shared_ptr<SequencePositionProcessor> sequenceProcessor = resolveSharedSequence(sequence->id);
	if (!sequenceProcessor) {
		sequenceProcessor = resolveNonSharedSequence(sequence->id);
	}

	if (sequenceProcessor) {
		return make_shared<SequenceValueProcessor>(sequenceProcessor, convertScriptSequenceMoveDirection(sequence->moveBefore), convertScriptSequenceMoveDirection(sequence->moveAfter), sequence->wrap, calcProcessors, scriptValue->quantize);
	} else {
		ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::SequenceValue_SequenceNotFound, "The sequence with id '", sequence->id.c_str(), "' could not be found.");
	}

	m_context.location.pop_back();

	return processor;
}

shared_ptr<CalcProcessor> ProcessorScriptParser::parseCalc(ScriptCalc* scriptCalc, vector<string> valueStack) {
	if (scriptCalc->ref.length() == 0) {
		bool valueProcessor = false;
		bool truncProcessor = false;
		bool fracProcessor = false;
		bool roundProcessor = false;
		bool quantizeProcessor = false;
		bool signProcessor = false;
		bool vtofProcessor = false;

		switch (scriptCalc->operation) {
			case ScriptCalc::CalcOperation::ADD:
				m_context.location.push_back("add");
				valueProcessor = true;
				break;
			case ScriptCalc::CalcOperation::SUB:
				m_context.location.push_back("sub");
				valueProcessor = true;
				break;
			case ScriptCalc::CalcOperation::DIV:
				m_context.location.push_back("div");
				valueProcessor = true;
				break;
			case ScriptCalc::CalcOperation::MULT:
				m_context.location.push_back("mult");
				valueProcessor = true;
				break;
			case ScriptCalc::CalcOperation::MAX:
				m_context.location.push_back("max");
				valueProcessor = true;
				break;
			case ScriptCalc::CalcOperation::MIN:
				m_context.location.push_back("min");
				valueProcessor = true;
				break;
			case ScriptCalc::CalcOperation::REMAIN:
				m_context.location.push_back("remain");
				valueProcessor = true;
				break;
			case ScriptCalc::CalcOperation::TRUNC:
				m_context.location.push_back("trunc");
				truncProcessor = true;
				break;
			case ScriptCalc::CalcOperation::FRAC:
				m_context.location.push_back("frac");
				fracProcessor = true;
				break;
			case ScriptCalc::CalcOperation::ROUND:
				m_context.location.push_back("round");
				roundProcessor = true;
				break;
			case ScriptCalc::CalcOperation::QUANTIZE:
				m_context.location.push_back("quantize");
				quantizeProcessor = true;
				break;
			case ScriptCalc::CalcOperation::SIGN:
				m_context.location.push_back("sign");
				signProcessor = true;
				break;
			case ScriptCalc::CalcOperation::VTOF:
				m_context.location.push_back("vtof");
				vtofProcessor = true;
				break;
		}

		shared_ptr<CalcProcessor> calcProcessor;

		if (valueProcessor) {
			shared_ptr<ValueProcessor> valueProcessor = parseValue(scriptCalc->value.get(), valueStack);
			calcProcessor = make_shared<CalcValueProcessor>(scriptCalc, valueProcessor);
		} else if (truncProcessor) {
			calcProcessor = make_shared<CalcTruncProcessor>();
		} else if (fracProcessor) {
			calcProcessor = make_shared<CalcFracProcessor>();
		} else if (roundProcessor) {
			calcProcessor = make_shared<CalcRoundProcessor>(scriptCalc);
		} else if (quantizeProcessor) {
			ScriptTuning* scriptTuning = nullptr;
			if (scriptCalc->tuning->ref.length() == 0) {
				scriptTuning = scriptCalc->tuning.get();
			} else {
				for (vector<ScriptTuning>::iterator it = m_context.script->tunings.begin(); it != m_context.script->tunings.end(); it++) {
					if (it->id == scriptCalc->tuning->ref) {
						scriptTuning = &(*it);
						break;
					}
				}
			}

			if (scriptTuning != nullptr) {
				calcProcessor = make_shared<CalcQuantizeProcessor>(scriptTuning);
			} else {
				ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Calc_QuantizeTuningNotFound, "Could not find the referenced tuning with id '", scriptCalc->tuning->ref.c_str(), "' in the script tunings.");
			}
		} else if (signProcessor) {
			calcProcessor = make_shared<CalcSignProcessor>(scriptCalc);
		} else if (vtofProcessor) {
			calcProcessor = make_shared<CalcVtoFProcessor>();
		}

		m_context.location.pop_back();

		return calcProcessor;
	} else {
		if (find(valueStack.begin(), valueStack.end(), string("c-") + scriptCalc->ref) == valueStack.end()) {
			int count = 0;
			for (vector<ScriptCalc>::iterator it = m_context.script->calcs.begin(); it != m_context.script->calcs.end(); it++) {
				if (scriptCalc->ref.compare(it->id) == 0) {
					m_context.stashLocation();
					m_context.location = { "component-pool",  "calcs", to_string(count) };
					valueStack.push_back(string("c-") + scriptCalc->ref);
					shared_ptr<CalcProcessor> calc = parseCalc(&(*it), valueStack);
					valueStack.pop_back();
					m_context.popLocation();
					return calc;
				}
				count++;
			}

			// Couldn't find the referenced calc...
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced calc with id '", scriptCalc->ref.c_str(), "' in the script calcs.");
		} else {
			ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_CircularFound, "Encountered a circular value reference while processing the calc with the id '", scriptCalc->ref.c_str(), "'. Circular references can not be resolved.");
		}
	}

	return shared_ptr<CalcProcessor>();
}
