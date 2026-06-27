#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"
#include <sstream>
#include <stdarg.h>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace timeseq;


LaneProcessor::LaneProcessor(const ScriptLane* scriptLane, const vector<shared_ptr<SegmentProcessor>>& segments, EventListener* eventListener) : m_scriptLane(scriptLane), m_segments(segments), m_eventListener(eventListener) {
	reset();
}

LaneProcessor::LaneState LaneProcessor::getState() {
	return m_state;
}

bool LaneProcessor::process() {
	bool stopped = false;

	if ((m_state == LaneState::STATE_PROCESSING) && (m_segments.size() > 0)) {
		vector<shared_ptr<SegmentProcessor>>::const_iterator segment = m_segments.begin() + m_activeSegment;
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

void LaneProcessor::processTriggers(const vector<string>& triggers) {
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
	const ScriptTimeline* scriptTimeline,
	const vector<shared_ptr<LaneProcessor>>& lanes,
	const unordered_map<string, vector<shared_ptr<LaneProcessor>>>& startTriggers,
	const unordered_map<string, vector<shared_ptr<LaneProcessor>>>& stopTriggers,
	TriggerHandler* triggerHandler) :
		m_scriptTimeline(scriptTimeline), m_lanes(lanes), m_startTriggers(startTriggers), m_stopTriggers(stopTriggers), m_triggerHandler(triggerHandler) {}

void TimelineProcessor::process() {
	bool checkLoop = false;

	// Check if any lane start or stop triggers were fired
	const vector<string>& triggers = m_triggerHandler->getTriggers();
	if (triggers.size() > 0) {
		for (const shared_ptr<LaneProcessor>& laneProcessor : m_lanes) {
			laneProcessor->processTriggers(triggers);
		}
	}

	// Call process on all lanes
	for (const shared_ptr<LaneProcessor>& lane : m_lanes) {
		bool stopped = lane->process();
		if (stopped) {
			if (m_scriptTimeline->loopLock) {
				// There is a loop-lock, so check looping for all lanes after we processed them all
				checkLoop = true;
			} else {
				// There is no loop-lock, so the lane should immediately check if it needs to loop
				lane->loop();
			}
		}
	}

	if (checkLoop) {
		// Check if all lanes are either idle or pending a loop
		bool loop = true;
		for (const shared_ptr<LaneProcessor>& lane : m_lanes) {
			if (lane->getState() == LaneProcessor::LaneState::STATE_PROCESSING) {
				// There is one still processing, so don't loop yet
				loop = false;
				break;
			}
		}

		if (loop) {
			for (const shared_ptr<LaneProcessor>& lane : m_lanes) {
				lane->loop();
			}
		}
	}
}

void TimelineProcessor::reset() {
	for (const shared_ptr<LaneProcessor>& lanes : m_lanes) {
		lanes->reset();
	}
}

TriggerProcessor::TriggerProcessor(const string& id, int inputPort, int inputChannel, PortHandler* portHandler, TriggerHandler* triggerHandler) : m_id(id), m_inputPort(inputPort), m_inputChannel(inputChannel), m_portHandler(portHandler), m_triggerHandler(triggerHandler) {}

void TriggerProcessor::process() {
	if (m_trigger.process(m_portHandler->getInputPortVoltage(m_inputPort, m_inputChannel), 0.f, 1.f)) {
		m_triggerHandler->setTrigger(m_id);
	}
}

SequenceProcessor::SequenceProcessor(const string& id, const vector<shared_ptr<ValueProcessor>>& values, bool retrieveVoltageOnce) : m_id(id), m_values(values), m_retrieveVoltageOnce(retrieveVoltageOnce) {}

void SequenceProcessor::clear() {
	m_values.clear();
}

string SequenceProcessor::getId() {
	return m_id;
}

const vector<shared_ptr<ValueProcessor>> SequenceProcessor::getValues() const {
	return m_values;
}

bool SequenceProcessor::isRetrieveVoltageOnce() {
	return m_retrieveVoltageOnce;
}

void SequenceProcessor::add(const shared_ptr<ValueProcessor>& value, int position) {
	if (position > -1) {
		if (position >= (int) m_values.size()) {
			m_values.push_back(value);
		} else {
			m_values.insert(m_values.begin() + position, value);
		}
	} else {
		m_values.push_back(value);
	}
}

void SequenceProcessor::remove(int position) {
	if (position > -1) {
		if (position < (int) m_values.size()) {
			m_values.erase(m_values.begin() + position);
		}
	} else if (m_values.size() > 0) {
		m_values.pop_back();
	}
}

SequencePositionProcessor::SequencePositionProcessor(const shared_ptr<SequenceProcessor>& sequenceProcessor, const shared_ptr<RandValueGenerator>& randValueGenerator) : m_position(0), m_sequenceProcessor(sequenceProcessor), m_randValueGenerator(randValueGenerator), m_hasStoredVoltage(false) {}

SequenceProcessor* SequencePositionProcessor::getSequenceProcessor() {
	return m_sequenceProcessor.get();
}

double SequencePositionProcessor::getCurrentValue() {
	if (m_sequenceProcessor->isRetrieveVoltageOnce() && m_hasStoredVoltage) {
		return m_storedVoltage;
	} else {
		int size = m_sequenceProcessor->getValues().size();
		if (size == 0) {
			m_storedVoltage = 0.;
		} else if (m_position >= size - 1) {
			m_storedVoltage = m_sequenceProcessor->getValues().back()->process();
		} else {
			m_storedVoltage = m_sequenceProcessor->getValues()[m_position]->process();
		}

		m_hasStoredVoltage = true;
		return m_storedVoltage;
	}
}

void SequencePositionProcessor::move(SequenceMoveDirection direction, bool wrap) {
	m_hasStoredVoltage = false;

	int size = m_sequenceProcessor->getValues().size();
	if (size > 0) {
		switch (direction) {
			case FORWARD:
				m_position++;
				break;
			case BACKWARD:
				m_position--;
				break;
			case RANDOM:
				// Generate a uniform random value between 0 and the number of values and remove the decimal part.
				// This results in an equal chance for each index, but we'll have to remove the 'size' edge case.
				m_position = min((int) floor(m_randValueGenerator->generate(0.f, size)), (int) size - 1);
				break;
			case NONE:
				break;
		}

		if (m_position < 0) {
			if (wrap) {
				m_position = size - 1;
			} else {
				m_position = 0;
			}
		} else if (m_position > size - 1) {
			if (wrap) {
				m_position = 0;
			} else {
				m_position = size - 1;
			}
		}
	} else {
		m_position = 0;
	}
}

void SequencePositionProcessor::move(int position) {
	m_hasStoredVoltage = false;

	if ((position >= (int) m_sequenceProcessor->getValues().size()) || (position < 0)) {
		m_position = max((int) m_sequenceProcessor->getValues().size() - 1, 0);
	} else {
		m_position = position;
	}
}

Processor::Processor(const shared_ptr<Script>& script, const vector<shared_ptr<TimelineProcessor>>& timelines, const vector<shared_ptr<TriggerProcessor>>& triggers, const vector<shared_ptr<ActionProcessor>>& startActions) : m_timelines(timelines), m_triggers(triggers), m_startActions(startActions), m_script(script) {}

void Processor::reset() {
	for (const shared_ptr<ActionProcessor>& actions : m_startActions) {
		actions->process();
	}

	for (const shared_ptr<TimelineProcessor>& timeline : m_timelines) {
		timeline->reset();
	}
}

void Processor::process() {
	for (const shared_ptr<TimelineProcessor>& timeline : m_timelines) {
		timeline->process();
	}

	for (const shared_ptr<TriggerProcessor>& trigger : m_triggers) {
		trigger->process();
	}
}
