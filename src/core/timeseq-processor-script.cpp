#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include <sstream>
#include <stdarg.h>

using namespace std;
using namespace timeseq;
	

ProcessorScriptParser::ProcessorScriptParser(PortReader* portReader, PortWriter* portWriter) {
	m_portReader = portReader;
	m_portWriter = portWriter;
}

shared_ptr<Processor> ProcessorScriptParser::parseScript(Script* script, vector<ValidationError> *validationErrors, vector<string> location) {
	int count = 0;
	location.push_back("timelines");
	vector<shared_ptr<TimelineProcessor>> timelineProcessors;
	for (vector<ScriptTimeline>::iterator it = script->timelines.begin(); it != script->timelines.end(); it++) {
		location.push_back(to_string(count));
		timelineProcessors.push_back(parseTimeline(&(*it), script, validationErrors, location));
		location.pop_back();
		count++;
	}
	location.pop_back();

	count = 0;
	location.push_back("input-triggers");
	vector<shared_ptr<TriggerProcessor>> triggerProcessors;
	for (vector<ScriptInputTrigger>::iterator it = script->inputTriggers.begin(); it != script->inputTriggers.end(); it++) {
		location.push_back(to_string(count));
		triggerProcessors.push_back(parseInputTrigger(&(*it), script, validationErrors, location));
		location.pop_back();
		count++;
	}
	location.pop_back();

	return shared_ptr<Processor>(new Processor(timelineProcessors, triggerProcessors));
}

shared_ptr<TimelineProcessor> ProcessorScriptParser::parseTimeline(ScriptTimeline* scriptTimeline, Script* script, vector<ValidationError> *validationErrors, vector<string> location) {
	TimelineProcessor* timelineProcessor = new TimelineProcessor(scriptTimeline);

	for (vector<ScriptLane>::iterator it = scriptTimeline->lanes.begin(); it != scriptTimeline->lanes.end(); it++) {
		
	}

	return shared_ptr<TimelineProcessor>(timelineProcessor);
}

std::shared_ptr<TriggerProcessor> ProcessorScriptParser::parseInputTrigger(ScriptInputTrigger* scriptInputTrigger, Script* script, std::vector<ValidationError> *validationErrors, std::vector<std::string> location) {
	// Check if it's a ref input trigger object or a full one
	if (scriptInputTrigger->input.ref.length() == 0) {
		return shared_ptr<TriggerProcessor>(new TriggerProcessor(scriptInputTrigger->id, scriptInputTrigger->input.index, ((bool) scriptInputTrigger->input.channel) ? *scriptInputTrigger->input.channel.get() : 0, m_portReader));
	} else {
		for (vector<ScriptInput>::iterator it = script->inputs.begin(); it != script->inputs.end(); it++) {
			if (scriptInputTrigger->input.ref.compare(it->id)) {
				return shared_ptr<TriggerProcessor>(new TriggerProcessor(scriptInputTrigger->id, it->index, ((bool) it->channel) ? *it->channel.get() : 0, m_portReader));
			}
		}

		// Couldn't find the referenced input...
		location.push_back("input");
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Ref_NotFound, "Could not find the referenced input with id '", scriptInputTrigger->input.ref, "' in the script inputs.");
		location.pop_back();
		return shared_ptr<TriggerProcessor>();
	}
}
