#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include <sstream>
#include <stdarg.h>

using namespace std;
using namespace timeseq;
	

float ValueProcessor::processValue() {
	return 0.f;
}


TimelineProcessor::TimelineProcessor(ScriptTimeline* scriptTimeline) : m_scriptTimeline(scriptTimeline) {}

TriggerProcessor::TriggerProcessor(std::string id, int inputPort, int inputChannel, PortReader* portReader) : m_portReader(portReader), m_id(id), m_inputPort(inputPort), m_inputChannel(inputChannel) {}

Processor::Processor(vector<shared_ptr<TimelineProcessor>> timelines, vector<shared_ptr<TriggerProcessor>> triggers) : m_timelines(timelines), m_triggers(triggers) {}

void Processor::process() {

}

ProcessorLoader::ProcessorLoader(PortReader* portReader, PortWriter* portWriter) : m_processorScriptParser(portReader, portWriter) {
}

shared_ptr<Processor> ProcessorLoader::loadScript(shared_ptr<Script> script, vector<ValidationError> *validationErrors) {
	return m_processorScriptParser.parseScript(script.get(), validationErrors, vector<string>());
}
