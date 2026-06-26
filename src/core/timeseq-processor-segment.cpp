#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"

using namespace std;
using namespace timeseq;

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


DurationVariableFactorProcessor::DurationVariableFactorProcessor(shared_ptr<ValueProcessor> value, double samplesFactor) : m_value(value), m_samplesFactor(samplesFactor) {}

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


DurationVariableHzProcessor::DurationVariableHzProcessor(shared_ptr<ValueProcessor> value, double sampleRate) : m_value(value), m_sampleRate(sampleRate) {}

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
