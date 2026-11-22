#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"

using namespace std;
using namespace timeseq;

SegmentProcessor::SegmentProcessor(const SegmentProcessor& segmentProcessor) :
	m_duration(segmentProcessor.m_duration),
	m_startActions(segmentProcessor.m_startActions),
	m_endActions(segmentProcessor.m_endActions),
	m_ongoingActions(segmentProcessor.m_ongoingActions),
	m_disableUi(segmentProcessor.m_disableUi),
	m_eventListener(segmentProcessor.m_eventListener) {
}

SegmentProcessor::SegmentProcessor(
	const shared_ptr<DurationProcessor>& duration,
	const vector<shared_ptr<ActionProcessor>>& startActions,
	const vector<shared_ptr<ActionProcessor>>& endActions,
	const vector<shared_ptr<ActionOngoingProcessor>>& ongoingActions,
	bool disableUi,
	EventListener* eventListener) :
		m_duration(duration), m_startActions(startActions), m_endActions(endActions), m_ongoingActions(ongoingActions), m_disableUi(disableUi), m_eventListener(eventListener) {}

void SegmentProcessor::pushStartActions(const vector<shared_ptr<ActionProcessor>>& startActions) {
	m_startActions.insert(m_startActions.begin(), startActions.begin(), startActions.end());
}

void SegmentProcessor::pushEndActions(const vector<shared_ptr<ActionProcessor>>& endActions) {
	m_endActions.insert(m_endActions.end(), endActions.begin(), endActions.end());
}

DurationProcessor::DurationState SegmentProcessor::getState() {
	return m_duration->getState();
}

double SegmentProcessor::process(double drift) {
	bool starting = false;

	// Trigger the start actions if we're at the start of the segment
	if (m_duration->getState() == DurationProcessor::DurationState::STATE_START) {
		if (!m_disableUi) {
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
	for (const shared_ptr<ActionProcessor>& actionProcessor : m_startActions) {
		actionProcessor->process();
	}
}

void SegmentProcessor::processEndActions() {
	for (const shared_ptr<ActionProcessor>& actionProcessor : m_endActions) {
		actionProcessor->process();
	}
}

void SegmentProcessor::processOngoingActions(bool start, bool end) {
	for (const shared_ptr<ActionOngoingProcessor>& actionProcessor : m_ongoingActions) {
		if (start) {
			actionProcessor->start(m_duration->getDuration());
		}

		if (end) {
			actionProcessor->end();
		} else {
			actionProcessor->process(m_duration->getPosition());
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


DurationVariableFactorProcessor::DurationVariableFactorProcessor(const shared_ptr<ValueProcessor>& value, double samplesFactor) : m_value(value), m_samplesFactor(samplesFactor) {}

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


DurationVariableHzProcessor::DurationVariableHzProcessor(const shared_ptr<ValueProcessor>& value, double sampleRate) : m_value(value), m_sampleRate(sampleRate) {}

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

DurationTriggerProcessor::DurationTriggerProcessor(const std::string triggerId, int division, TriggerHandler *triggerHandler) : m_triggerHandler(triggerHandler), m_triggerId(triggerId), m_division(division) {}

void DurationTriggerProcessor::prepareForStart() {
	m_triggerCounter = 0;
	// The duration is expected to start from 1 iso 0, so reported position and duration is increased by 1
	m_position = 1;
	setDuration(m_division + 1);
}

double DurationTriggerProcessor::process(double drift) {
	if (m_state == DurationState::STATE_START) {
		m_triggerCounter = 0;
		// The duration is expected to start from 1 iso 0, so reported position and duration is increased by 1
		m_position = 1;
		setDuration(m_division + 1);
	}
	m_state = DurationState::STATE_PROGRESS;

	const vector<string>& triggers = m_triggerHandler->getTriggers();
	if (triggers.size() > 0) {
		if (find(triggers.begin(), triggers.end(), m_triggerId) != triggers.end()) {
			m_triggerCounter++;
			m_position = m_triggerCounter + 1; // The duration is expected to start from 1 iso 0, so reported position and duration is increased by 1
			if (m_triggerCounter >= m_division) {
				m_position = m_division + 1; // Don't allow the position to go past the duration of the segment
				m_state = DurationState::STATE_END;
			}
		}
	}

	return 0.;
}
