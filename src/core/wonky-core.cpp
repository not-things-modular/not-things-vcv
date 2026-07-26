#include "core/wonky-core.hpp"
#include <limits>
#include <random>

using namespace wonky;

constexpr float kMinTheta = 0.001f;
constexpr float kMaxTheta = 0.35f;
constexpr float kStdDevScale = 3.0f;

bool WonkyInputData::operator==(const WonkyInputData& other) const {
	return !(*this != other);
}

bool WonkyInputData::operator!=(const WonkyInputData& other) const {
	return other.bpm != bpm ||
		other.wobbleAmount != wobbleAmount || other.wobbleProbability != wobbleProbability ||
		other.waverAmount != waverAmount || other.waverProbability != waverProbability ||
		other.wanderAmount != wanderAmount || other.wanderRate != wanderRate;
}

Wonkiness::Wonkiness(Randomizer* randomizer) : m_randomizer(randomizer) {}

float Wonkiness::getWanderAmount() const {
		return m_wanderAmount;
}

float Wonkiness::getWaverAmount() const {
	return m_waverAmount;
}

float Wonkiness::getWobbleAmount() const {
	return m_wobbleAmount;
}

bool Wonkiness::isWonky() const {
	return m_wanderAmount != 0.f || m_waverAmount != 0.f || m_wobbleAmount != 0.f;
}

void Wonkiness::determineWonkiness(const WonkyInputData& inputData) {
	// Determine the wander amount
	if ((inputData.wanderAmount > 0.f) && (inputData.wanderRate > 0.f)) {
		const float theta = kMinTheta * std::pow(kMaxTheta / kMinTheta, inputData.wanderRate);
		const float sigma = inputData.wanderAmount / kStdDevScale * std::sqrt(theta * (2.0f - theta));
		const float newOffset = m_wanderAmount * (1.f - theta) + sigma * m_randomizer->randomizeGaussian(0.f, 1.f);
		m_wanderAmount  = std::min(std::max(newOffset, -inputData.wanderAmount), inputData.wanderAmount);
	} else {
		m_wanderAmount = 0.f;
	}

	// Determine the waver amount
	if ((inputData.waverAmount > 0.f) && (inputData.waverProbability > 0.f)) {
		// Check if a waver should occur (either because it is at 100% or because the randomizer said so)
		if ((inputData.waverProbability == 100.f) || (m_randomizer->randomize(0.f, 100.f) <= inputData.waverProbability)) {
			// Generate the waver amount
			m_waverAmount = m_randomizer->randomize(-inputData.waverAmount, inputData.waverAmount);
		} else {
			m_waverAmount = 0.f;
		}
	} else {
		m_waverAmount = 0.f;
	}

	// Determine the wobble amount
	if ((inputData.wobbleAmount > 0.f) && (inputData.wobbleProbability > 0.f)) {
		// Check if a wobble should occur (either because it is at 100% or because the randomizer said so)
		if ((inputData.wobbleProbability == 100.f) || (m_randomizer->randomize(0.f, 100.f) <= inputData.wobbleProbability)) {
			// Generate the wobble amount
			m_wobbleAmount = m_randomizer->randomize(-inputData.wobbleAmount, inputData.wobbleAmount);
		} else {
			m_wobbleAmount = 0.f;
		}
	} else {
		m_wobbleAmount = 0.f;
	}
}

struct WonkyRandomizer : Randomizer {
	WonkyRandomizer() {
		m_generator.seed(std::random_device{}());
	}

	float randomize(float lower, float upper) override {
		std::uniform_real_distribution<float> distribution(lower, upper);
		return distribution(m_generator);
	}

	float randomizeGaussian(float mean, float stddev) override {
		std::normal_distribution<float> distribution(mean, stddev);
		return distribution(m_generator);
	}

	private:
		std::minstd_rand m_generator;
};

WonkyCore::WonkyCore(const SampleRateReader* sampleRateReader, WonkyListener* listener) : WonkyCore(sampleRateReader, new WonkyRandomizer(), listener) {}

WonkyCore::WonkyCore(const SampleRateReader* sampleRateReader, Randomizer* randomizer, WonkyListener* listener) : m_sampleRateReader(sampleRateReader), m_listener(listener), m_randomizer(randomizer), m_wonkiness(randomizer) {}

WonkyCore::~WonkyCore() {
	delete m_randomizer;
}

void WonkyCore::process(const WonkyInputData& inputData) {
	// Check if the input data changed
	if (m_inputData != inputData) {
		// If the BPM changed, re-calculate the clock parameters
		if (m_inputData.bpm != inputData.bpm) {
			updateBpm(inputData.bpm);
			if (m_wonkiness.isWonky()) {
				updateWonkiness();
			}
		}
		// Store the input data for future reference
		m_inputData = inputData;
	}

	// Advance the clock
	m_clockData.sampleProgress++;

	// If there is a wobbleDelay present, we're still processing the wobble of the previous gate
	if (m_clockData.wobbleDelay > 0) {
		m_clockData.wobbleDelay--;
		if ((m_clockData.wobbleDelay == 0) && (!m_clockData.gateHigh)) {
			// We completed the previous wobble, so the gate can go high now
			m_clockData.gateHigh = true;
			m_listener->clockGateChanged(true);
		}
	}

	// If we passed over a clock boundary, complete this clock and start the next one
	if ((m_reset) || (m_clockData.sampleProgress >= m_clockData.clockSampleDuration)) {
		// Reset the progress
		m_clockData.sampleProgress = 0;
		m_reset = false;

		// If there is a positive wobble sample offset, we'll have to delay the gate signal
		if (m_clockData.currentWobbleSampleOffset > 0) {
			m_clockData.wobbleDelay = m_clockData.currentWobbleSampleOffset;
		} else {
			m_clockData.wobbleDelay = 0;
		}

		// If wobbleDelay is 0, the gate must go high now (unless it is already high)
		if ((!m_clockData.gateHigh) && (m_clockData.wobbleDelay == 0)) {
			m_clockData.gateHigh = true;
			m_listener->clockGateChanged(true);
		}
		
		// Generate new wonkiness
		m_wonkiness.determineWonkiness(m_inputData);
		m_listener->wanderChanged(m_wonkiness.getWanderAmount(), m_inputData.wanderAmount);
		m_listener->waverChanged(m_wonkiness.getWaverAmount(), m_inputData.waverAmount);
		m_listener->wobbleChanged(m_wonkiness.getWobbleAmount(), m_inputData.wobbleAmount);

		// Prepare the new clock data
		m_clockData.clockSampleDuration = m_clockData.clockDuration;
		if (m_wonkiness.isWonky()) {
			// If there is wonkiness, apply it
			updateWonkiness();
			// If there is wonkiness, the clock is unstable anyway, so don't apply drift accuracy
			m_clockData.wonkyDrift = 0.f;
		} else {
			// No wonkyness means high-accuracy, so determine the drift.
			m_clockData.wonkyDrift += m_clockData.clockDrift;
			if (m_clockData.wonkyDrift >= 1.f) {
				m_clockData.clockSampleDuration++;
				m_clockData.wonkyDrift--;
			}

			// Set all the other clock parameters to a simple clock with a half-duration gate
			m_clockData.gateDuration = m_clockData.clockSampleDuration / 2;
			m_clockData.currentWobbleSampleOffset = 0;
		}
	} else if ((m_clockData.gateHigh) && (m_clockData.sampleProgress >= m_clockData.gateDuration)) {
		// We're past the halfway mark of the clock, so the gate goes low
		m_clockData.gateHigh = false;
		m_listener->clockGateChanged(false);
	}
}

void WonkyCore::reset() {
	// Make sure the clock will retrigger on the next progress
	m_reset = true;
	// Remove any collected drift
	m_clockData.wonkyDrift = 0.;
	// And reset the high gate if needed
	if (m_clockData.gateHigh) {
		m_clockData.gateHigh = false;
		m_listener->clockGateChanged(false);
	}
}

void WonkyCore::updateBpm(int bpm) {
	double samplesPerMinute = (double) m_sampleRateReader->getSampleRate() * 60;
	m_clockData.clockDuration = samplesPerMinute / bpm;
	m_clockData.clockSampleDuration = m_clockData.clockDuration;
	m_clockData.clockDrift = m_clockData.clockDuration - m_clockData.clockSampleDuration;

	// Reset the accumulated drift since the interal clock changed
	m_clockData.wonkyDrift = 0.;
}

void WonkyCore::updateWonkiness() {
	// If there is a wander amount, apply it to the duration of the clock signal
	if (m_wonkiness.getWanderAmount() != 0.f) {
		m_clockData.clockSampleDuration *= 1.f + (m_wonkiness.getWanderAmount() / 100.f);
	}

	// First apply the waver amount, since it moves the whole clock forward or backwards
	if (m_wonkiness.getWaverAmount() != 0.f) {
		m_clockData.clockSampleDuration += (float) m_clockData.clockSampleDuration * m_wonkiness.getWaverAmount() / 100.f;
	}
	
	// Then calculate how much wobble is to be applied
	if (m_wonkiness.getWobbleAmount() != 0.f) {
		m_clockData.currentWobbleSampleOffset = (float) m_clockData.clockSampleDuration * m_wonkiness.getWobbleAmount() / 100.f;
	}

	// Now we can determine the amount of time the gate should remain high based on the impact of waver and wobble
	m_clockData.gateDuration = (m_clockData.clockSampleDuration + m_clockData.currentWobbleSampleOffset) / 2;
}
