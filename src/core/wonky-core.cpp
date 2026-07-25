#include "core/wonky-core.hpp"

using namespace wonky;

bool WonkyInputData::operator==(const WonkyInputData& other) const {
	return !(*this != other);
}

bool WonkyInputData::operator!=(const WonkyInputData& other) const {
	return other.bpm != bpm;
}

WonkyCore::WonkyCore(const SampleRateReader* sampleRateReader, WonkyListener* listener) {
	m_sampleRateReader = sampleRateReader;
	m_listener = listener;
}

void WonkyCore::process(const WonkyInputData& inputData) {
	// If the input data changed, re-calculate the clock parameters
	if (m_inputData != inputData) {
		m_inputData = inputData;

		// Calculate the new clock information
		double samplesPerMinute = (double) m_sampleRateReader->getSampleRate() * 60;
		m_clockData.clockDuration = samplesPerMinute / m_inputData.bpm;
		m_clockData.sampleDuration = m_clockData.clockDuration;
		m_clockData.clockDrift = m_clockData.clockDuration - m_clockData.sampleDuration;

		// Since the parameters changed, reset the accumulated drift
		m_clockData.wonkyDrift = 0.;
	}

	// Advance the clock
	m_clockData.sampleProgress++;
	// If we passed over a clock boundary, start the next one
	if (m_clockData.sampleProgress >= m_clockData.sampleDuration) {
		// Reset the progress
		m_clockData.sampleProgress = 0;

		// Determine the new sample duration of this clock pulse (with possible drift added)
		m_clockData.sampleDuration = m_clockData.clockDuration;
		m_clockData.wonkyDrift += m_clockData.clockDrift;
		if (m_clockData.wonkyDrift >= 1.) {
			m_clockData.sampleDuration++;
			m_clockData.wonkyDrift--;
		}

		// Set the gate high, and calculate the duration of the gate
		m_clockData.gateHigh = true;
		m_clockData.gateDuration = m_clockData.sampleDuration / 2;

		// Send out a gate change notification
		m_listener->clockGateChanged(true);
	} else if ((m_clockData.gateHigh) && (m_clockData.sampleProgress >= m_clockData.gateDuration)) {
		m_clockData.gateHigh =- false;
		m_listener->clockGateChanged(false);
	}
}

void WonkyCore::reset() {
	m_clockData.sampleProgress = 0;
	m_clockData.wonkyDrift = 0.;
}