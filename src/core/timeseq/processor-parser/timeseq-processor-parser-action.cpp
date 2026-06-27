#include "core/timeseq-processor-parser.hpp"
#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"

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

const shared_ptr<ActionProcessor> ProcessorScriptParser::parseResolvedAction(const ScriptAction* scriptAction) {
	shared_ptr<ActionProcessor> actionProcessor;
	shared_ptr<IfProcessor> ifProcessor;

	if (scriptAction->condition) {
		m_context.location.push_back("if");
		vector<string> stack;
		ifProcessor = parseIf(scriptAction->condition.get(), stack);
		m_context.location.pop_back();
	}

	if (scriptAction->setValue) {
		m_context.location.push_back("set-value");
		actionProcessor = parseSetValueAction(scriptAction, ifProcessor);
		m_context.location.pop_back();
	} else if (scriptAction->setVariable) {
		m_context.location.push_back("set-variable");
		actionProcessor = parseSetVariableAction(scriptAction, ifProcessor);
		m_context.location.pop_back();
	} else if (scriptAction->setPolyphony) {
		m_context.location.push_back("set-polyphony");
		actionProcessor = parseSetPolyphonyAction(scriptAction, ifProcessor);
		m_context.location.pop_back();
	} else if (scriptAction->setLabel) {
		m_context.location.push_back("set-label");
		actionProcessor = parseSetLabelAction(scriptAction, ifProcessor);
		m_context.location.pop_back();
	} else if (scriptAction->assert) {
		m_context.location.push_back("assert");
		actionProcessor = parseAssertAction(scriptAction, ifProcessor);
		m_context.location.pop_back();
	} else if (scriptAction->trigger.length() > 0) {
		m_context.location.push_back("trigger");
		actionProcessor = parseTriggerAction(scriptAction, ifProcessor);
		m_context.location.pop_back();
	} else if (scriptAction->moveSequence) {
		m_context.location.push_back("move-sequence");
		actionProcessor = parseMoveSequenceAction(scriptAction, ifProcessor);
		m_context.location.pop_back();
	} else if (scriptAction->clearSequence.length() > 0) {
		m_context.location.push_back("clear-sequence");
		actionProcessor = parseClearSequenceAction(scriptAction, ifProcessor);
		m_context.location.pop_back();
	} else if (scriptAction->addToSequence) {
		m_context.location.push_back("add-to-sequence");
		actionProcessor = parseAddToSequenceAction(scriptAction, ifProcessor);
		m_context.location.pop_back();
	} else if (scriptAction->removeFromSequence) {
		m_context.location.push_back("remove-from-sequence");
		actionProcessor = parseRemoveFromSequenceAction(scriptAction, ifProcessor);
		m_context.location.pop_back();
	} else {
		return actionProcessor;
	}

	return actionProcessor;
}

const shared_ptr<ActionGlideProcessor> ProcessorScriptParser::parseResolvedGlideAction(const ScriptAction* scriptAction) {
	shared_ptr<IfProcessor> ifProcessor;

	if (scriptAction->condition) {
		m_context.location.push_back("if");
		vector<string> stack;
		ifProcessor = parseIf(scriptAction->condition.get(), stack);
		m_context.location.pop_back();
	}

	m_context.location.push_back("start-value");
	vector<string> stack;
	shared_ptr<ValueProcessor> startValueProcessor = parseValue(&(*scriptAction->startValue.get()), stack);
	m_context.location.pop_back();
	stack.clear();
	m_context.location.push_back("end-value");
	shared_ptr<ValueProcessor> endValueProcessor = parseValue(&(*scriptAction->endValue.get()), stack);
	m_context.location.pop_back();

	int outputPort = -1;
	int outputChannel = -1;
	if (scriptAction->output) {
		m_context.location.push_back("output");
		pair<int, int> output = parseOutput(&(*scriptAction->output));
		m_context.location.pop_back();
		outputPort = output.first;
		outputChannel = output.second;
	}

	float easeFactor = 0.f;
	bool easePow = false;
	if (scriptAction->easeFactor) {
		easeFactor = *scriptAction->easeFactor.get();
	}
	if (scriptAction->easeAlgorithm) {
		if (*scriptAction->easeAlgorithm == ScriptAction::EaseAlgorithm::POW) {
			easePow = true;
		}
	}

	return make_shared<ActionGlideProcessor>(easeFactor, easePow, startValueProcessor, endValueProcessor, ifProcessor, outputPort, outputChannel, scriptAction->variable, m_portHandler, m_variableHandler);
}

const shared_ptr<ActionGateProcessor> ProcessorScriptParser::parseResolvedGateAction(const ScriptAction* scriptAction) {
	shared_ptr<IfProcessor> ifProcessor;

	if (scriptAction->condition) {
		m_context.location.push_back("if");
		vector<string> stack;
		ifProcessor = parseIf(scriptAction->condition.get(), stack);
		m_context.location.pop_back();
	}

	int outputPort = -1;
	int outputChannel = -1;
	if (scriptAction->output) {
		m_context.location.push_back("output");
		pair<int, int> output = parseOutput(&(*scriptAction->output));
		m_context.location.pop_back();
		outputPort = output.first;
		outputChannel = output.second;
	}

	float gateHighRatio = 0.5f;
	if (scriptAction->gateHighRatio) {
		gateHighRatio = *scriptAction->gateHighRatio.get();
	}

	return make_shared<ActionGateProcessor>(gateHighRatio, ifProcessor, outputPort, outputChannel, m_portHandler);
}

const shared_ptr<ActionProcessor> ProcessorScriptParser::parseSetValueAction(const ScriptAction* scriptAction, const shared_ptr<IfProcessor>& ifProcessor) {
	m_context.location.push_back("value");
	vector<string> stack;
	shared_ptr<ValueProcessor> valueProcessor = parseValue(&scriptAction->setValue.get()->value, stack);
	m_context.location.pop_back();

	m_context.location.push_back("output");
	pair<int, int> output = parseOutput(&scriptAction->setValue.get()->output);
	m_context.location.pop_back();

	return make_shared<ActionSetValueProcessor>(valueProcessor, output.first, output.second, m_portHandler, ifProcessor);
}

const shared_ptr<ActionProcessor> ProcessorScriptParser::parseSetVariableAction(const ScriptAction* scriptAction, const shared_ptr<IfProcessor>& ifProcessor) {
	m_context.location.push_back("value");
	vector<string> stack;
	shared_ptr<ValueProcessor> valueProcessor = parseValue(&scriptAction->setVariable.get()->value, stack);
	m_context.location.pop_back();

	return make_shared<ActionSetVariableProcessor>(valueProcessor, scriptAction->setVariable.get()->name, m_variableHandler, ifProcessor);
}

const shared_ptr<ActionProcessor> ProcessorScriptParser::parseSetPolyphonyAction(const ScriptAction* scriptAction, const shared_ptr<IfProcessor>& ifProcessor) {
	ScriptSetPolyphony* scriptSetPolyphony = scriptAction->setPolyphony.get();
	return make_shared<ActionSetPolyphonyProcessor>(scriptSetPolyphony->index - 1, scriptSetPolyphony->channels, m_portHandler, ifProcessor);
}

const shared_ptr<ActionProcessor> ProcessorScriptParser::parseSetLabelAction(const ScriptAction* scriptAction, const shared_ptr<IfProcessor>& ifProcessor) {
	ScriptSetLabel* scriptSetLabel = scriptAction->setLabel.get();
	return make_shared<ActionSetLabelProcessor>(scriptSetLabel->index - 1, scriptSetLabel->label, m_portHandler, ifProcessor);
}

const shared_ptr<ActionProcessor> ProcessorScriptParser::parseAssertAction(const ScriptAction* scriptAction, const shared_ptr<IfProcessor>& ifProcessor) {
	ScriptAssert* scriptAssert = scriptAction->assert.get();

	m_context.location.push_back("expect");
	vector<string> stack;
	shared_ptr<IfProcessor> expect = parseIf(&scriptAssert->expect, stack);
	m_context.location.pop_back();

	return make_shared<ActionAssertProcessor>(scriptAssert->name, expect, scriptAssert->stopOnFail, m_assertListener, ifProcessor);
}

const shared_ptr<ActionProcessor> ProcessorScriptParser::parseTriggerAction(const ScriptAction* scriptAction, const shared_ptr<IfProcessor>& ifProcessor) {
	return make_shared<ActionTriggerProcessor>(scriptAction->trigger, m_triggerHandler, ifProcessor);
}

const shared_ptr<ActionProcessor> ProcessorScriptParser::parseMoveSequenceAction(const ScriptAction* scriptAction, const shared_ptr<IfProcessor>& ifProcessor) {
	ScriptMoveSequence* moveSequence = &(*scriptAction->moveSequence);
	shared_ptr<SequencePositionProcessor> sequenceProcessor = resolveSharedSequence(moveSequence->id);

	if (!sequenceProcessor) {
		if (hasNonSharedSequence(moveSequence->id)) {
			addValidationError(m_context.validationErrors, m_context.location, ValidationErrorCode::MoveSequence_NonSharedSequence, "The sequence with id '", moveSequence->id.c_str(), "' is not a 'shared' sequence. Only shared sequences can be moved.");
		} else {
			addValidationError(m_context.validationErrors, m_context.location, ValidationErrorCode::MoveSequence_SequenceNotFound, "The sequence with id '", moveSequence->id.c_str(), "' could not be found.");
		}

		return nullptr;
	}

	if (moveSequence->direction) {
		return make_shared<ActionMoveSequenceDirectionProcessor>(sequenceProcessor, convertScriptSequenceMoveDirection(*moveSequence->direction), moveSequence->wrap, ifProcessor);
	} else {
		return make_shared<ActionMoveSequencePositionProcessor>(sequenceProcessor, *moveSequence->position, ifProcessor);
	}
}

const shared_ptr<ActionProcessor> ProcessorScriptParser::parseClearSequenceAction(const ScriptAction* scriptAction, const shared_ptr<IfProcessor>& ifProcessor) {
	shared_ptr<SequencePositionProcessor> sequenceProcessor = resolveSharedSequence(scriptAction->clearSequence);
	if (!sequenceProcessor) {
		sequenceProcessor = resolveNonSharedSequence(scriptAction->clearSequence);
	}

	if (sequenceProcessor) {
		return make_shared<ActionClearSequenceProcessor>(sequenceProcessor, ifProcessor);
	} else {
		addValidationError(m_context.validationErrors, m_context.location, ValidationErrorCode::ClearSequence_SequenceNotFound, "The sequence with id '", scriptAction->clearSequence.c_str(), "' could not be found.");
		return nullptr;
	}
}

const shared_ptr<ActionProcessor> ProcessorScriptParser::parseAddToSequenceAction(const ScriptAction* scriptAction, const shared_ptr<IfProcessor>& ifProcessor) {
	ScriptAddToSequence* addToSequence = &(*scriptAction->addToSequence);

	m_context.location.push_back("value");
	vector<string> stack;
	shared_ptr<ValueProcessor> value = parseValue(&addToSequence->value, stack);
	m_context.location.pop_back();

	shared_ptr<SequencePositionProcessor> sequenceProcessor = resolveSharedSequence(addToSequence->id);
	if (!sequenceProcessor) {
		sequenceProcessor = resolveNonSharedSequence(addToSequence->id);
	}

	if (sequenceProcessor) {
		return make_shared<ActionAddToSequenceSequenceProcessor>(sequenceProcessor, value, addToSequence->position, addToSequence->asConstantVoltage, ifProcessor);
	} else {
		addValidationError(m_context.validationErrors, m_context.location, ValidationErrorCode::AddToSequence_SequenceNotFound, "The sequence with id '", addToSequence->id.c_str(), "' could not be found.");
		return nullptr;
	}
}

const shared_ptr<ActionProcessor> ProcessorScriptParser::parseRemoveFromSequenceAction(const ScriptAction* scriptAction, const shared_ptr<IfProcessor>& ifProcessor) {
	ScriptRemoveFromSequence* removeFromSequence = &(*scriptAction->removeFromSequence);
	shared_ptr<SequencePositionProcessor> sequenceProcessor = resolveSharedSequence(removeFromSequence->id);
	if (!sequenceProcessor) {
		sequenceProcessor = resolveNonSharedSequence(removeFromSequence->id);
	}

	if (sequenceProcessor) {
		return make_shared<ActionRemoveFromSequenceProcessor>(sequenceProcessor, removeFromSequence->position, ifProcessor);
	} else {
		addValidationError(m_context.validationErrors, m_context.location, ValidationErrorCode::RemoveFromSequence_SequenceNotFound, "The sequence with id '", removeFromSequence->id.c_str(), "' could not be found.");
		return nullptr;
	}
}

const ScriptAction* ProcessorScriptParser::resolveScriptAction(const ScriptAction* scriptAction, vector<string>& resolvedLocation) const {
	if (scriptAction->ref.length() == 0) {
		resolvedLocation = m_context.location;
		return scriptAction;
	} else {
		int count = 0;
		for (const ScriptAction& action : m_context.script->actions) {
			if (scriptAction->ref.compare(action.id) == 0) {
				resolvedLocation = { "component-pool", "actions", to_string(count) };
				return &action;
			}
			count++;
		}

		return nullptr;
	}
}
