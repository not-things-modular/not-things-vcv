#include "core/wonky-core.hpp"
#include <limits>
#include <random>
#include <algorithm>

using namespace wonky;

constexpr float kMinTheta = 0.001f;
constexpr float kMaxTheta = 0.35f;
constexpr float kStdDevScale = 3.0f;

int clockRatioFamilyToIndex(ClockRatioFamily family) {
    switch (family) {
        case FAMILY_2: return 0;
        case FAMILY_3: return 1;
        case FAMILY_5: return 2;
        case FAMILY_7: return 3;
        default: return -1; // FAMILY_0/FAMILY_1 don't need family data at all
    }
}

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
	// Check if the input data changed
	if (m_inputData != inputData) {
		// If the BPM changed, re-calculate the clock parameters
		bool bpmChanged = m_inputData.bpm != inputData.bpm;
		if (bpmChanged) {
			updateBpm(inputData.bpm);
			if (m_wonkiness.isWonky()) {
				updateWonkiness();
			}
		}

		// Check if the clock rates changed
		bool clocksChanged = m_inputData.clockRates != inputData.clockRates;

		// Store the input data for future reference
		m_inputData = inputData;

		// If the clock rates or the bpm changed, recalculate the subdivision clocks information
		if (clocksChanged || bpmChanged) {
			updateSubClocks(bpmChanged, clocksChanged);
		}
	}

	// Advance the clock
	m_clockState.sampleProgress++;

	// If there is a wobbleDelay present, we're still processing the wobble of the previous gate
	if (m_clockState.wobbleDelay > 0) {
		m_clockState.wobbleDelay--;
		if ((m_clockState.wobbleDelay == 0) && (!m_clockState.gateHigh)) {
			// We completed the previous wobble, so the gate can go high now
			triggerMainClock();
		}
	}

	// If we passed over a clock boundary, complete this clock and start the next one
	if ((m_reset) || (m_clockState.sampleProgress >= m_clockState.clockSampleDuration)) {
		// Reset the progress
		m_clockState.sampleProgress = 0;
		m_reset = false;

		// If there is a positive wobble sample offset, we'll have to delay the gate signal
		if (m_clockState.currentWobbleSampleOffset > 0) {
			m_clockState.wobbleDelay = m_clockState.currentWobbleSampleOffset;
		} else {
			m_clockState.wobbleDelay = 0;
		}

		// If wobbleDelay is 0, the gate must go high now (unless it is already high)
		if ((!m_clockState.gateHigh) && (m_clockState.wobbleDelay == 0)) {
			triggerMainClock();
		}

		// Generate new wonkiness
		m_wonkiness.determineWonkiness(m_inputData);
		m_listener->wanderChanged(m_wonkiness.getWanderAmount(), m_inputData.wanderAmount);
		m_listener->waverChanged(m_wonkiness.getWaverAmount(), m_inputData.waverAmount);
		m_listener->wobbleChanged(m_wonkiness.getWobbleAmount(), m_inputData.wobbleAmount);

		// Prepare the new clock data
		m_clockState.clockSampleDuration = static_cast<int>(m_clockState.clockDuration);
		// Determine drift to account for mismatches between sample rate and clock rate
		m_clockState.wonkyDrift += m_clockState.clockDrift;
		if (m_clockState.wonkyDrift >= 1.f) {
			m_clockState.clockSampleDuration++;
			m_clockState.wonkyDrift--;
		}
		if (m_wonkiness.isWonky()) {
			// If there is wonkiness, apply it
			updateWonkiness();
		} else {
			// No wonkiness, so set all the other clock parameters to a simple clock with a half-duration gate
			m_clockState.gateDuration = m_clockState.clockSampleDuration / 2;
			m_clockState.currentWobbleSampleOffset = 0;
		}
	} else if ((m_clockState.gateHigh) && (m_clockState.sampleProgress >= m_clockState.gateDuration)) {
		// We're past the halfway mark of the clock, so the gate goes low
		m_clockState.gateHigh = false;
		m_listener->clockGateChanged(-1, false);
	}
}

void WonkyCore::reset() {
	// Make sure the clock will retrigger on the next progress
	m_reset = true;
	// Remove any collected drift
	m_clockState.wonkyDrift = 0.;
	// And reset the high gate if needed
	if (m_clockState.gateHigh) {
		m_clockState.gateHigh = false;
		m_listener->clockGateChanged(-1, false);
	}
}

void WonkyCore::updateBpm(int bpm) {
	double samplesPerMinute = (double) m_sampleRateReader->getSampleRate() * 60;
	m_clockState.clockDuration = samplesPerMinute / bpm;
	m_clockState.clockSampleDuration = static_cast<int>(m_clockState.clockDuration);
	m_clockState.clockDrift = m_clockState.clockDuration - m_clockState.clockSampleDuration;

	// Reset the accumulated drift since the interal clock changed
	m_clockState.wonkyDrift = 0.;
}

void WonkyCore::updateWonkiness() {
	// If there is a wander amount, apply it to the duration of the clock signal
	if (m_wonkiness.getWanderAmount() != 0.f) {
		m_clockState.clockSampleDuration *= 1.f + (m_wonkiness.getWanderAmount() / 100.f);
	}

	// First apply the waver amount, since it moves the whole clock forward or backwards
	if (m_wonkiness.getWaverAmount() != 0.f) {
		m_clockState.clockSampleDuration += (float) m_clockState.clockSampleDuration * m_wonkiness.getWaverAmount() / 100.f;
	}

	// Then calculate how much wobble is to be applied
	if (m_wonkiness.getWobbleAmount() != 0.f) {
		m_clockState.currentWobbleSampleOffset = static_cast<int>((float) m_clockState.clockSampleDuration * m_wonkiness.getWobbleAmount() / 100.f);
	}

	// Now we can determine the amount of time the gate should remain high based on the impact of waver and wobble
	m_clockState.gateDuration = (m_clockState.clockSampleDuration + m_clockState.currentWobbleSampleOffset) / 2;
}

void WonkyCore::updateSubClocks(bool bpmChanged, bool clocksChanged) {
	// If the clocks themselves changed, re-group the clocks and determine the family and sub-clocks hierarchy
	if (clocksChanged) {
		// Group the incoming clocks into their respective families (if they belong to a to-be-processed family),
		// determine the max multiplication of each family and the overal max
		std::array<std::vector<const ClockRatioData*>, 4> familyClockRatios{};
		std::array<int, 4> highestFamilyMultiplications{};
		int highestMultiplication = 0;
		m_divSubClocks.clear();
		for (const ClockRatioData* clockRate : m_inputData.clockRates) {
			// Check which index to use for the family (if it is part of a to-be-processed family)
			int familyIndex = clockRatioFamilyToIndex(clockRate->family);
			if (familyIndex != -1) {
				// It's a multiplying clock, check that the ratio isn't in that family group yet
				if (std::find(familyClockRatios[familyIndex].begin(), familyClockRatios[familyIndex].end(), clockRate) == familyClockRatios[familyIndex].end()) {
					familyClockRatios[familyIndex].push_back(clockRate);
					if (clockRate->ratio > highestFamilyMultiplications[familyIndex]) {
						highestFamilyMultiplications[familyIndex] = clockRate->ratio;
					}
					if (clockRate->ratio > highestMultiplication) {
						highestMultiplication = clockRate->ratio;
					}
				}
			} else {
				// It's a dividing clock, so check that it isn't in the list of dividing clocks yet, and add if needed
				if (std::find_if(m_divSubClocks.begin(), m_divSubClocks.end(), [clockRate](WonkySubClockState& state) { return state.clockData->ratio == clockRate->ratio; }) == m_divSubClocks.end()) {
					m_divSubClocks.push_back(WonkySubClockState(clockRate, clockRate->ratio, m_clockState.dividedClockProgress % clockRate->ratio));
				}
			}
		}

		for (int i = 0; i < 4; i++) {
			int multiplication = highestFamilyMultiplications[i];
			m_multFamilies[i].highestMultiplication = multiplication;

			if (multiplication > 0) {
				// There are subclocks in this family, so update the family properties
				m_multFamilies[i].currentStep = m_clockState.sampleProgress / multiplication;

				// Update the sub-clock properties
				m_multSubClocks[i].clear();
				for (unsigned int j = 0; j < familyClockRatios[i].size(); j++) {
					const ClockRatioData* clockData = familyClockRatios[i][j];
					int familyTickPerClockTick = multiplication / clockData->ratio;
					int currentFamilyTick = m_multFamilies[i].currentStep % familyTickPerClockTick;
					m_multSubClocks[i].push_back(WonkySubClockState(clockData, familyTickPerClockTick, currentFamilyTick));
				}
			}
		}
	}

	// If the BPM changed, update timing related properties
	if (bpmChanged) {
		for (int i = 0; i < 4; i++) {
			int multiplication = m_multFamilies[i].highestMultiplication;
			for (int j = 0; j < multiplication; j++) {
				m_multFamilies[i].offsets[j] = static_cast<int>(m_clockState.clockSampleDuration / multiplication * (j + 1));
			}
		}
	}
}

void WonkyCore::triggerMainClock() {
	// Trigger the main clock
	m_clockState.gateHigh = true;
	m_clockState.dividedClockProgress = (m_clockState.dividedClockProgress + 1) % 64;
	m_listener->clockGateChanged(-1, true);

	// Run through the active divided clocks and progress them
	for (WonkySubClockState& subClockState : m_divSubClocks) {
		subClockState.currentFamilyTick++;
		if (subClockState.currentFamilyTick >= subClockState.familyTickPerClockTick) {
			// TODO: send out the gate and manage gate high/low (no wobble anymore?)
		}
	}
}
