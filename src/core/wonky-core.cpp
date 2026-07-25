#include "core/wonky-core.hpp"
#include <limits>
#include <random>

using namespace wonky;

bool WonkyInputData::operator==(const WonkyInputData& other) const {
	return !(*this != other);
}

bool WonkyInputData::operator!=(const WonkyInputData& other) const {
	return other.bpm != bpm ||
		other.wobbleAmount != wobbleAmount || other.wobbleProbability != wobbleProbability ||
		other.waverAmount != waverAmount || other.waverProbability != waverProbability ||
		other.wanderAmount != wanderAmount || other.wanderRate != wanderRate;
}

struct WonkyRandomizer : Randomizer {
	WonkyRandomizer() {
		m_generator.seed(std::random_device{}());
	}

	float randomize(float lower, float upper) const override {
		std::uniform_real_distribution<float> distribution = std::uniform_real_distribution<float>(lower, upper);
		return distribution(m_generator);
	}

	private:
		std::minstd_rand m_generator;
};

WonkyCore::WonkyCore(const SampleRateReader* sampleRateReader, WonkyListener* listener) : WonkyCore(sampleRateReader, new WonkyRandomizer(), listener) {}

WonkyCore::WonkyCore(const SampleRateReader* sampleRateReader, const Randomizer* randomizer, WonkyListener* listener) : m_sampleRateReader(sampleRateReader), m_randomizer(randomizer), m_listener(listener) {}

WonkyCore::~WonkyCore() {
	delete m_randomizer;
}

void WonkyCore::process(const WonkyInputData& inputData) {
	// Check if the input data changed
	if (m_inputData != inputData) {
		// If the BPM changed, re-calculate the clock parameters
		if (m_inputData.bpm != inputData.bpm) {
			// Update the clock calculation information if the bpm changed
			if (m_inputData.bpm != inputData.bpm) {
				double samplesPerMinute = (double) m_sampleRateReader->getSampleRate() * 60;
				m_clockData.clockDuration = samplesPerMinute / inputData.bpm;
				m_clockData.sampleDuration = m_clockData.clockDuration;
				m_clockData.clockDrift = m_clockData.clockDuration - m_clockData.sampleDuration;

				// Reset the accumulated drift since the interal clock changed
				m_clockData.wonkyDrift = 0.;
			}
		}
		// Store the input data for future reference
		m_inputData = inputData;
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
		// We're past the halfway mark of the clock, so the gate goes low
		m_clockData.gateHigh = false;
		m_listener->clockGateChanged(false);
	}
}

void WonkyCore::reset() {
	// Make sure the clock will retrigger on the next progress
	m_clockData.sampleProgress = std::numeric_limits<int>::max() - 1;
	// Remove any collected drift
	m_clockData.wonkyDrift = 0.;
	// And reset the high gate if needed
	if (m_clockData.gateHigh) {
		m_clockData.gateHigh = false;
		m_listener->clockGateChanged(false);
	}
}

void WonkyCore::determineWonkiness() {

}