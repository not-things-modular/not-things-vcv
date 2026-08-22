#include "core/wonky-core.hpp"
#include <limits>
#include <random>
#include <algorithm>
#include <cmath>

using namespace wonky;

constexpr float kMinTheta = 0.001f;
constexpr float kMaxTheta = 0.35f;
constexpr float kStdDevScale = 3.0f;

// The least common multiple of the divided clock ratios
constexpr int lcmDivisionRatio = 2116800;

bool ClockRatioData::operator==(const ClockRatioData& other) const {
	return !(*this != other);
}

bool ClockRatioData::operator!=(const ClockRatioData& other) const {
	// Only one instance of each ClockRatioId whould be made, so comparing the IDs should indicate if they are equal.
	return other.id != id;
}

bool WonkyInputData::operator==(const WonkyInputData& other) const {
	return !(*this != other);
}

bool WonkyInputData::operator!=(const WonkyInputData& other) const {
	return other.bpm != bpm ||
		other.wobbleAmount != wobbleAmount || other.wobbleProbability != wobbleProbability ||
		other.waverAmount != waverAmount || other.waverProbability != waverProbability ||
		other.wanderAmount != wanderAmount || other.wanderRate != wanderRate ||
		other.linked != linked ||
		other.clockRates != clockRates;
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
	// Determine the wobble amount
	bool wobbled = false;
	m_wobbleAmount = 0.f;
	if ((inputData.wobbleAmount > 0.f) && (inputData.wobbleProbability > 0.f)) {
		// Check if a wobble should occur (either because it is at 100% or because the randomizer said so)
		if ((inputData.wobbleProbability == 100.f) || (m_randomizer->randomize(0.f, 100.f) <= inputData.wobbleProbability)) {
			// Generate the wobble amount
			m_wobbleAmount = m_randomizer->randomize(-inputData.wobbleAmount, inputData.wobbleAmount);
			wobbled = true;
		}
	}

	// Determine the waver amount
	m_waverAmount = 0.f;
	if ((inputData.waverProbability > 0.f) && (inputData.waverAmount > 0.f)) {
		// Check if waver should be attempted based on the linked property
		float minWaverAmount = -inputData.waverAmount;
		float maxWaverAmount = inputData.waverAmount;
		// Check if the linked property influences waver behaviour
		if (inputData.linked) {
			if (wobbled) {
				// There was a wobble, so waver in the same direction
				minWaverAmount = (m_wobbleAmount > 0.f) ? 0.f : -inputData.waverAmount;
				maxWaverAmount = (m_wobbleAmount > 0.f) ? inputData.waverAmount : 0.f;
			} else {
				// There was no wobble, so don't waver
				minWaverAmount = maxWaverAmount = 0.f;
			}
		}

		// If there is a waverAmount set, use it now
		if (minWaverAmount != 0.f || maxWaverAmount != 0.f) {
			// Check if a waver should occur (either because it is at 100% or because the randomizer said so)
			if ((inputData.waverProbability == 100.f) || (m_randomizer->randomize(0.f, 100.f) <= inputData.waverProbability)) {
				// Generate the waver amount
				m_waverAmount = m_randomizer->randomize(minWaverAmount, maxWaverAmount);
			}
		}
	}

	// Determine the wander amount
	if ((inputData.wanderAmount > 0.f) && (inputData.wanderRate > 0.f)) {
		const float theta = kMinTheta * std::pow(kMaxTheta / kMinTheta, inputData.wanderRate);
		const float sigma = inputData.wanderAmount / kStdDevScale * std::sqrt(theta * (2.0f - theta));

		float t = std::abs(m_wanderAmount) / inputData.wanderAmount;
		float strength = theta * t * t;
		const float newOffset = m_wanderAmount + sigma * m_randomizer->randomizeGaussian(0.f, 1.f) - strength * m_wanderAmount;

		m_wanderAmount = std::min(std::max(newOffset, -inputData.wanderAmount), inputData.wanderAmount);
	} else {
		m_wanderAmount = 0.f;
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
	// Check if the input data changed or the sample rate changed
	if ((m_inputData != inputData) || (m_sampleRateChanged)) {
		// If the BPM changed (or the sample rate changed, which also influences the bpm-to-sample ratio), re-calculate the clock parameters
		bool bpmChanged = (m_inputData.bpm != inputData.bpm);
		if (bpmChanged || m_sampleRateChanged) {
			updateBpm(inputData.bpm);
			updateWonkyClockDuration();

			m_sampleRateChanged = false;
		}

		// Check if the clock rates changed
		m_subClockState.clocksChanged = m_subClockState.clocksChanged || m_inputData.clockRates != inputData.clockRates;

		// Store the input data for future reference
		m_inputData = inputData;

		// If the clock rates or the bpm changed, recalculate the subdivision clocks information
		if (bpmChanged) {
			distributeSubClocks();
		}
	}

	// Advance the clock
	m_clockState.sampleProgress++;

	// Check if a reset was requested or we passed over the clock boundary
	if ((m_reset) || (m_clockState.sampleProgress >= m_clockState.wonkyClockSampleDuration)) {
		// Reset the progress
		m_clockState.sampleProgress = 0;

		// Prepare the duration of the next stable clock tick
		m_clockState.clockSampleDuration = static_cast<int>(m_clockState.clockDuration);
		// Determine drift to account for mismatches between sample rate and clock rate
		m_clockState.wonkyDrift += m_clockState.clockDrift;
		if (m_clockState.wonkyDrift >= 1.f) {
			m_clockState.clockSampleDuration++;
			m_clockState.wonkyDrift--;
		}

		// The end wobble of the previous clock beat becomes the start wobble of the new clock beat
		m_clockState.startWobbleSampleOffset = m_clockState.endWobbleSampleOffset;

		// Generate new wonkiness
		m_wonkiness.determineWonkiness(m_inputData);
		m_listener->wanderChanged(m_wonkiness.getWanderAmount(), m_inputData.wanderAmount);
		m_listener->waverChanged(m_wonkiness.getWaverAmount(), m_inputData.waverAmount);
		m_listener->wobbleChanged(m_wonkiness.getWobbleAmount(), m_inputData.wobbleAmount);

		// Calculate the actual length to use for the current wonky clock output, taking any (carried over) wobble, waver and wander into account
		updateWonkyClockDuration();

		// If the sub-clocks changed since the last main clock signal, redetect which are present
		if (m_subClockState.clocksChanged) {
			m_subClockState.clocksChanged = false;
			detectSubClocks();
		}

		// Reset the position of the faster sub clock tick progress, and let all of them trigger together with the main clock
		m_subClockState.fastClockProgress.fill(0);
		m_subClockState.fastClockNextSamplePosition.fill(0);

		if (m_reset) {
			// Reset the counter for the divided clocks upon reset
			m_clockState.dividedClockProgress = 0;
			// And clear the reset flag
			m_reset = false;
		}

		// Trigger the main clock
		updateMainClockState(true);
	} else if ((m_clockState.gateHigh) && (m_clockState.sampleProgress >= m_clockState.gateDuration)) {
		// We're past the halfway mark of the clock, so the gate goes low
		updateMainClockState(false);
	}

	// Check for each fast clock whether it passed over a tick boundary
	for (int i = 0; i < 8; i++) {
		const ClockRatioData* clockRatio = m_inputData.clockRates[i];
		if (clockRatio->type == ClockRatioType::RATIO_MULTIPLY) {
			if ((m_subClockState.fastClockProgress[i] < clockRatio->ratio * 2) && (m_subClockState.fastClockNextSamplePosition[i] <= m_clockState.sampleProgress)) {
				m_listener->clockGateChanged(i, m_subClockState.fastClockProgress[i] % 2 == 0);
				m_subClockState.fastClockProgress[i]++;
				m_subClockState.fastClockNextSamplePosition[i] = m_clockState.wonkyClockSampleDuration * m_subClockState.fastClockProgress[i] / (clockRatio->ratio * 2);
			}
		}
	}
}

void WonkyCore::reset() {
	// Make sure the clock will retrigger on the next progress
	m_reset = true;
	// Remove any collected drift
	m_clockState.wonkyDrift = 0.;
	// And reset the high gate if needed
	m_clockState.gateHigh = false;
	// Disable all clock outputs, starting from -1 (the main clock index)
	for (int i = -1; i < (int) m_inputData.clockRates.size(); i++) {
		m_listener->clockGateChanged(i, false);
	}
}

void WonkyCore::sampleRateChanged() {
	m_sampleRateChanged = true;
}

void WonkyCore::updateBpm(float bpm) {
	double samplesPerMinute = (double) m_sampleRateReader->getSampleRate() * 60;
	m_clockState.clockDuration = samplesPerMinute / bpm;
	m_clockState.clockSampleDuration = static_cast<int>(m_clockState.clockDuration);
	m_clockState.clockDrift = m_clockState.clockDuration - m_clockState.clockSampleDuration;

	// Reset the accumulated drift since the interal clock changed
	m_clockState.wonkyDrift = 0.;
}

void WonkyCore::updateWonkyClockDuration() {
	// First set the wonky clock duration equal to the duration of the stable clock (as a float for the upcoming calculations)
	float wonkyClockSampleDuration = m_clockState.clockSampleDuration;

	// If there is a wander amount, apply it to the duration of the clock signal
	if (m_wonkiness.getWanderAmount() != 0.f) {
		wonkyClockSampleDuration *= 1.f + (m_wonkiness.getWanderAmount() / 100.f);
	}

	// Then apply the waver amount first, since it moves the whole clock forward or backwards
	if (m_wonkiness.getWaverAmount() != 0.f) {
		wonkyClockSampleDuration += (float) wonkyClockSampleDuration * m_wonkiness.getWaverAmount() / 100.f;
	}

	// Finally calculate how much wobble is to be applied to the end of the clock
	if (m_wonkiness.getWobbleAmount() != 0.f) {
		m_clockState.endWobbleSampleOffset = static_cast<int>((float) wonkyClockSampleDuration * m_wonkiness.getWobbleAmount() / 100.f);
	} else {
		m_clockState.endWobbleSampleOffset = 0;
	}

	// Remove the start wobble duration from the clock duration (remove, since a positive wobble of the previous clock causes the current clock to be shorter)
	wonkyClockSampleDuration -= m_clockState.startWobbleSampleOffset;
	// Add the end wobble duration from the clock duration (add, since a positive wobble causes the clock to become longer)
	wonkyClockSampleDuration += m_clockState.endWobbleSampleOffset;

	// Assign the resulting duratino to the clock state (converting back to an int)
	m_clockState.wonkyClockSampleDuration = static_cast<int>(wonkyClockSampleDuration);

	// Now that we know the wonky clock duration, determine when it should go from high to low
	m_clockState.gateDuration = m_clockState.wonkyClockSampleDuration / 2;

}

void WonkyCore::detectSubClocks() {
	m_subClockState.slowClockIndices.clear();
	for (int i = 0; i < 8; i++) {
		if (m_inputData.clockRates[i]->type == ClockRatioType::RATIO_DIVIDE) {
			m_subClockState.slowClockIndices.push_back(i);
		}
	}
}

void WonkyCore::distributeSubClocks() {
	for (int i = 0; i < 8; i++) {
		if (m_inputData.clockRates[i]->type == ClockRatioType::RATIO_MULTIPLY) {
			int divisions = m_inputData.clockRates[i]->ratio * 2;
			m_subClockState.fastClockNextSamplePosition[i] = m_clockState.wonkyClockSampleDuration * m_subClockState.fastClockProgress[i] / divisions;
		}
	}
}

void WonkyCore::updateMainClockState(bool high) {
	// Update the main clock state
	m_clockState.gateHigh = high;
	m_listener->clockGateChanged(-1, high);

	// Advance the divided clock progress if a new clock signal triggered
	if (high) {
		for (int index : m_subClockState.slowClockIndices) {
			int ratio = m_inputData.clockRates[index]->ratio;
			if (ratio == 1) {
				// The clock runs at the same rate as the main clock, so just let it follow along with the main clock
				m_listener->clockGateChanged(index, true);
			} else {
				int position = m_clockState.dividedClockProgress % ratio;

				if (position == 0) {
					// A divided clock goes high when the position is at the start of the cycle
					m_listener->clockGateChanged(index, true);
				} else if ((ratio % 2 == 0) && (position == ratio / 2)) {
					// And low when it reaches the half-way point of the ratio (but only if it is an even ratio)
					m_listener->clockGateChanged(index, false);
				}
			}
		}

		// Advance the divided clock progress: Increase and take remainder of the Least Common Multiple of all divided clocks
		m_clockState.dividedClockProgress = (m_clockState.dividedClockProgress + 1) % lcmDivisionRatio;
	} else {
		// A divided clock with an uneven ratio goes low halfway between clock ticks (i.e. when the main clock goes low)
		for (int index : m_subClockState.slowClockIndices) {
			int ratio = m_inputData.clockRates[index]->ratio;
			if (ratio == 1) {
				// The clock runs at the same rate as the main clock, so just let it follow along with the main clock
				m_listener->clockGateChanged(index, false);
			}
			else if (ratio % 2 == 1) {
				int position = m_clockState.dividedClockProgress % ratio;

				if (position == (ratio / 2) + 1) {
					m_listener->clockGateChanged(index, false);
				}
			}
		}
	}
}

void WonkyCore::updateSubClockStates(const std::vector<ClockRatioId>& highRatioIds, const std::vector<ClockRatioId>& lowRatioIds) {
	for (unsigned int i = 0; i < m_inputData.clockRates.size(); i++) {
		const ClockRatioData* clockRatio = m_inputData.clockRates[i];
		if (clockRatio->id != ClockRatioId::NO_RATE) {
			if (std::find(highRatioIds.begin(), highRatioIds.end(), clockRatio->id) != highRatioIds.end()) {
				m_listener->clockGateChanged(i, true);
			} else if (std::find(lowRatioIds.begin(), lowRatioIds.end(), clockRatio->id) != lowRatioIds.end()) {
				m_listener->clockGateChanged(i, false);
			}
		}
	}
}