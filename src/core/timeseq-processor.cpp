#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include <sstream>
#include <stdarg.h>

using namespace std;
using namespace timeseq;
	

float ValueProcessor::processValue() {
	return 0.f;
}

DurationProcessor::DurationProcessor(uint64_t duration) : m_duration(duration) {}

SegmentProcessor::SegmentProcessor(
	ScriptSegment* scriptSegment,
	DurationProcessor durationProcessor,
	std::vector<std::shared_ptr<ActionProcessor>> startActions,
	std::vector<std::shared_ptr<ActionProcessor>> endActions,
	std::vector<std::shared_ptr<ActionGlideProcessor>> glideActions) :
		m_scriptSegment(scriptSegment), m_durationProcessor(durationProcessor), m_startActions(startActions), m_endActions(endActions), m_glideActions(glideActions) {}

LaneProcessor::LaneProcessor(ScriptLane* scriptLane, vector<shared_ptr<SegmentProcessor>> segments) : m_scriptLane(scriptLane), m_segments(segments) {}

TimelineProcessor::TimelineProcessor(
	ScriptTimeline* scriptTimeline,
	vector<shared_ptr<LaneProcessor>> laneProcessors,
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> startTriggers,
	unordered_map<string, vector<shared_ptr<LaneProcessor>>> stopTriggers) :
		m_scriptTimeline(scriptTimeline), m_laneProcessors(laneProcessors), m_startTriggers(startTriggers), m_stopTriggers(stopTriggers) {}

TriggerProcessor::TriggerProcessor(string id, int inputPort, int inputChannel, PortReader* portReader) : m_portReader(portReader), m_id(id), m_inputPort(inputPort), m_inputChannel(inputChannel) {}

Processor::Processor(vector<shared_ptr<TimelineProcessor>> timelines, vector<shared_ptr<TriggerProcessor>> triggers) : m_timelines(timelines), m_triggers(triggers) {}

void Processor::process() {

}

ProcessorLoader::ProcessorLoader(PortReader* portReader, SampleRateReader* sampleRateReader, PortWriter* portWriter) : m_processorScriptParser(portReader, sampleRateReader, portWriter) {
}

shared_ptr<Processor> ProcessorLoader::loadScript(shared_ptr<Script> script, vector<ValidationError> *validationErrors) {
	return m_processorScriptParser.parseScript(script.get(), validationErrors, vector<string>());
}
