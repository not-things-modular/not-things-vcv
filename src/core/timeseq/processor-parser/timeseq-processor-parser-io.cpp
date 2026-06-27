#include "core/timeseq-processor-parser.hpp"
#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"

using namespace std;
using namespace timeseq;

shared_ptr<TriggerProcessor> ProcessorScriptParser::parseInputTrigger(const ScriptInputTrigger* scriptInputTrigger) {
	// Check if it's a ref input trigger object or a full one
	if (scriptInputTrigger->input.ref.length() == 0) {
		return make_shared<TriggerProcessor>(scriptInputTrigger->id, scriptInputTrigger->input.index - 1, ((bool) scriptInputTrigger->input.channel) ? *scriptInputTrigger->input.channel.get() - 1 : 0, m_portHandler, m_triggerHandler);
	} else {
		for (const ScriptInput& input : m_context.script->inputs) {
			if (scriptInputTrigger->input.ref.compare(input.id) == 0) {
				return make_shared<TriggerProcessor>(scriptInputTrigger->id, input.index - 1, ((bool) input.channel) ? *input.channel.get() - 1 : 0, m_portHandler, m_triggerHandler);
			}
		}

		// Couldn't find the referenced input...
		m_context.location.push_back("input");
		ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced input with id '", scriptInputTrigger->input.ref.c_str(), "' in the script inputs.");
		m_context.location.pop_back();
		return shared_ptr<TriggerProcessor>();
	}
}

const pair<int, int> ProcessorScriptParser::parseInput(const ScriptInput* scriptInput) {
	// Check if it's a ref input or a full one
	if (scriptInput->ref.length() == 0) {
		return pair<int, int>(scriptInput->index - 1, scriptInput->channel ? *scriptInput->channel.get() - 1 : 0);
	} else {
		int count = 0;
		for (const ScriptInput& input : m_context.script->inputs) {
			if (scriptInput->ref.compare(input.id) == 0) {
				m_context.stashLocation();
				m_context.location = { "component-pool",  "inputs", to_string(count) };
				pair<int, int> result = parseInput(&input);
				m_context.popLocation();
				return result;
			}
			count++;
		}

		// Couldn't find the referenced input...
		ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced input with id '", scriptInput->ref.c_str(), "' in the script inputs.");
		return pair<int, int>(-1, -1);
	}

}

const pair<int, int> ProcessorScriptParser::parseOutput(const ScriptOutput* scriptOutput) {
	// Check if it's a ref output or a full one
	if (scriptOutput->ref.length() == 0) {
		return pair<int, int>(scriptOutput->index - 1, scriptOutput->channel ? *scriptOutput->channel.get() - 1 : 0);
	} else {
		int count = 0;
		for (const ScriptOutput& output : m_context.script->outputs) {
			if (scriptOutput->ref.compare(output.id) == 0) {
				m_context.stashLocation();
				m_context.location = { "component-pool",  "outputs", to_string(count) };
				pair<int, int> result = parseOutput(&output);
				m_context.popLocation();
				return result;
			}
			count++;
		}

		// Couldn't find the referenced output...
		ADD_VALIDATION_ERROR(m_context.validationErrors, m_context.location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced output with id '", scriptOutput->ref.c_str(), "' in the script outputs.");
		return pair<int, int>(-1, -1);
	}
}
