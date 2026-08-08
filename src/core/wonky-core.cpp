#include "core/wonky-core.hpp"
#include <limits>
#include <random>
#include <algorithm>

using namespace wonky;

constexpr float kMinTheta = 0.001f;
constexpr float kMaxTheta = 0.35f;
constexpr float kStdDevScale = 3.0f;

constexpr int family2Index = 0;
constexpr int family3Index = 1;
constexpr int family5Index = 2;
constexpr int family7Index = 3;

struct WonkySubClockHierarchyItem {
	WonkySubClockHierarchyItem(int index, int familyIndex, int parentStartTickIndex, int parentEndTickIndex, int parentTickDivisionCount, int parentTickDivisionIndex) : index(index), familyIndex(familyIndex), parentStartTickIndex(parentStartTickIndex), parentEndTickIndex(parentEndTickIndex), parentTickDivisionCount(parentTickDivisionCount), parentTickDivisionIndex(parentTickDivisionIndex), overlappingIndex(std::pair<int, int>(-1, -1)) {}
	WonkySubClockHierarchyItem(int index, int familyIndex, std::pair<int, int> overlappingIndex) : index(index), familyIndex(familyIndex), parentStartTickIndex(-1), parentEndTickIndex(-1), parentTickDivisionCount(0), parentTickDivisionIndex(0), overlappingIndex(overlappingIndex) {}

	// The index of this item within the family it belongs to
	const int index;
	// The family index of this item;
	const int familyIndex;
	// The index of the tick in this family that represents the starting point of this clock tick (or -1 if this tick is an overlapping tick with another family)
	const int parentStartTickIndex;
	// The index of the tick in this family that represents the ending point of this clock tick (or -1 if this tick is an overlapping tick with another family)
	// Once this tick has been positions, the halfwaypoint of the parentEndTickIndex item should be updated to be positioned at this tick
	const int parentEndTickIndex;
	// The number of tick divisions that have to be made in between the parent ticks
	const int parentTickDivisionCount;
	// The index of this tick within the parent tick divisions
	const int parentTickDivisionIndex;
	// The index of the item in another family that should be used as value for this tick
	// The first int is the family index, the second is the item within that family. Set to (-1, -1) if this tick is not overlapping with another family
	const std::pair<int, int> overlappingIndex;
};

struct WonkySubClockHierarchyLayer {
	WonkySubClockHierarchyLayer(int ratio, const std::vector<WonkySubClockHierarchyItem> items) : ratio(ratio), items(items) {}

	const int ratio;
	const std::vector<WonkySubClockHierarchyItem> items;
};

constexpr int startClick = -1;
constexpr int endClick = -2;

const std::array<std::vector<WonkySubClockHierarchyLayer>, 4> clockSubClockHierarchy = {{
	// The 2-based family: for each increasingly faster clock, determine teh middle positions in between the existing ones
	{
		{ WonkySubClockHierarchyLayer(2, { WonkySubClockHierarchyItem(7, family2Index, startClick, endClick, 2, 1) }) },
		{ WonkySubClockHierarchyLayer(4, { WonkySubClockHierarchyItem(3, family2Index, startClick, 7, 2, 1), WonkySubClockHierarchyItem(11, family2Index, 7, endClick, 2, 1) }) },
		{ WonkySubClockHierarchyLayer(8, { WonkySubClockHierarchyItem(1, family2Index, startClick, 3, 2, 1), WonkySubClockHierarchyItem(5, family2Index, 3, 7, 2, 1), WonkySubClockHierarchyItem(9, family2Index, 7, 11, 2, 1), WonkySubClockHierarchyItem(13, family2Index, 11, endClick, 2, 1) }) },
		{ WonkySubClockHierarchyLayer(16, { WonkySubClockHierarchyItem(0, family2Index, startClick, 1, 2, 1), WonkySubClockHierarchyItem(2, family2Index, 1, 3, 2, 1), WonkySubClockHierarchyItem(4, family2Index, 3, 5, 2, 1), WonkySubClockHierarchyItem(6, family2Index, 5, 7, 2, 1), WonkySubClockHierarchyItem(8, family2Index, 7, 9, 2, 1), WonkySubClockHierarchyItem(10, family2Index, 9, 11, 2, 1), WonkySubClockHierarchyItem(12, family2Index, 11, 13, 2, 1), WonkySubClockHierarchyItem(14, family2Index, 13, endClick, 2, 1) }) }
	},
	// The 3-based family, the fastest clock determines which single set of calculations to perform:
	// - 3x: divide the full clock in 3 parts
	// - 6x: use the 2x clock tick to determine position of the 5th tick, and divide the two resulting sections in 3 parts
	// - 12x: use the 4x clock ticks to determine the 2nd, 5th and 8th ticks, and divide each of the resulting four sections in 3 parts
	{
		{ WonkySubClockHierarchyLayer(3, { WonkySubClockHierarchyItem(3, family3Index, startClick, endClick, 3, 1), WonkySubClockHierarchyItem(7, family3Index, startClick, endClick, 3, 2) }) },
		{ WonkySubClockHierarchyLayer(6, {
			// First use the 2x clock tick as middle click for the 6x clock
			WonkySubClockHierarchyItem(5, family3Index, std::pair<int, int>(2, 7)),
			// Then divide the two resulting sections into three parts
			WonkySubClockHierarchyItem(1, family3Index, startClick, 5, 3, 1),
			WonkySubClockHierarchyItem(3, family3Index, startClick, 5, 3, 2),
			WonkySubClockHierarchyItem(7, family3Index, 5, endClick, 3, 1),
			WonkySubClockHierarchyItem(9, family3Index, 5, endClick, 3, 2)
		}) },
		{ WonkySubClockHierarchyLayer(12, {
			// First use the 1st tick (index 3) of the 4x clock tick and use it as 2nd tick of this clock
			WonkySubClockHierarchyItem(2, family3Index, std::pair<int, int>(2, 3)),
			// Then divide the first section of this tick into three parts
			WonkySubClockHierarchyItem(0, family3Index, startClick, 2, 3, 1),
			WonkySubClockHierarchyItem(1, family3Index, startClick, 2, 3, 2),
			// Then use the 2nd tick (index 7) of the 4x clock tick and use it as 5th tick of this clock
			WonkySubClockHierarchyItem(5, family3Index, std::pair<int, int>(2, 7)),
			// Then divide the second section of this tick into three parts
			WonkySubClockHierarchyItem(3, family3Index, 2, 5, 3, 1),
			WonkySubClockHierarchyItem(4, family3Index, 2, 5, 3, 2),
			// Then use the 3st tick (index 11) of the 4x clock tick and use it as 8th tick of this clock
			WonkySubClockHierarchyItem(8, family3Index, std::pair<int, int>(2, 11)),
			// Then divide the third and fourth sections of this tick into three parts
			WonkySubClockHierarchyItem(6, family3Index, 5, 8, 3, 1),
			WonkySubClockHierarchyItem(7, family3Index, 5, 8, 3, 2),
			WonkySubClockHierarchyItem(9, family3Index, 8, endClick, 3, 1),
			WonkySubClockHierarchyItem(10, family3Index, 8, endClick, 3, 2)
		}) }
	},
	// The 5-family divides everything into 5 parts without internal or external influence
	{
		{ WonkySubClockHierarchyLayer(5, {
			WonkySubClockHierarchyItem(0, family5Index, startClick, endClick, 5, 1),
			WonkySubClockHierarchyItem(1, family5Index, startClick, endClick, 5, 2),
			WonkySubClockHierarchyItem(2, family5Index, startClick, endClick, 5, 3),
			WonkySubClockHierarchyItem(3, family5Index, startClick, endClick, 5, 4)
		}) }
	},
	// The 7-family divides everything into 7 parts without internal or external influence
	{
		{ WonkySubClockHierarchyLayer(7, {
			WonkySubClockHierarchyItem(0, family7Index, startClick, endClick, 7, 1),
			WonkySubClockHierarchyItem(1, family7Index, startClick, endClick, 7, 2),
			WonkySubClockHierarchyItem(2, family7Index, startClick, endClick, 7, 3),
			WonkySubClockHierarchyItem(3, family7Index, startClick, endClick, 7, 4),
			WonkySubClockHierarchyItem(4, family7Index, startClick, endClick, 7, 5),
			WonkySubClockHierarchyItem(5, family7Index, startClick, endClick, 7, 6)
		}) }
	}
}};

float getWobbleAmount(const WonkyInputData& inputData, Randomizer* randomizer) {
	float wobbleAmount = 0.f;

	if ((inputData.wobbleAmount > 0.f) && (inputData.wobbleProbability > 0.f)) {
		// Check if a wobble should occur (either because it is at 100% or because the randomizer said so)
		if ((inputData.wobbleProbability == 100.f) || (randomizer->randomize(0.f, 100.f) <= inputData.wobbleProbability)) {
			// Generate the wobble amount
			wobbleAmount = randomizer->randomize(-inputData.wobbleAmount, inputData.wobbleAmount);
		}
	}

	return wobbleAmount;
}

int clockRatioFamilyToIndex(ClockRatioFamily family) {
    switch (family) {
        case FAMILY_2: return family2Index;
        case FAMILY_3: return family3Index;
        case FAMILY_5: return family5Index;
        case FAMILY_7: return family7Index;
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

void WonkySubClockTickState::initialize(int tickEndPosition, int wobbleSampleOffset, int gateLowPosition) {
	this->tickEndPosition = tickEndPosition;
	this->wobbleSampleOffset = wobbleSampleOffset;
	this->wobbleDelay = 0;
	this->gateLowPosition = gateLowPosition;
	this->gateHigh = false;
}

void WonkySubClockTickState::initialize(const WonkySubClockTickState& state) {
	initialize(state.tickEndPosition, state.wobbleSampleOffset, state.gateLowPosition);
}

void WonkySubClockTickState::reset() {
	this->tickEndPosition = -1;
	this->wobbleSampleOffset = 0;
	this->wobbleDelay = 0;
	this->gateLowPosition = -1;
	this->gateHigh = false;
}

WonkyCore::WonkyCore(const SampleRateReader* sampleRateReader, WonkyListener* listener) : WonkyCore(sampleRateReader, new WonkyRandomizer(), listener) {}

WonkyCore::WonkyCore(const SampleRateReader* sampleRateReader, Randomizer* randomizer, WonkyListener* listener) : m_sampleRateReader(sampleRateReader), m_listener(listener), m_randomizer(randomizer), m_wonkiness(randomizer) {
	// Allocate the ticks for each of the ratio families
	m_subClockTickStates[clockRatioFamilyToIndex(ClockRatioFamily::FAMILY_2)].resize(15); // 15 total ticks plus the final main clock tick in the 2-family to allow triggering of the 16x clock
	m_subClockTickStates[clockRatioFamilyToIndex(ClockRatioFamily::FAMILY_3)].resize(11); // 11 total ticks plus the final main clock tick in the 3-family to allow triggering of the 12x clock
	m_subClockTickStates[clockRatioFamilyToIndex(ClockRatioFamily::FAMILY_5)].resize(4); // 4 total ticks plus the final main clock tick in the 5-family to allow triggering of the 5x clock
	m_subClockTickStates[clockRatioFamilyToIndex(ClockRatioFamily::FAMILY_7)].resize(6); // 6 total ticks plus the final main clock tick in the 7-family to allow triggering of the 7x clock
	// By default, no clocks are present
	m_hasSubClock.fill(false);
}

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

void initializeClockTickState(WonkySubClockTickState& clockTickState, int tickStartPosition, int tickEndPosition, const WonkyInputData& inputData, Randomizer* randomizer) {
	float wobbleAmount = getWobbleAmount(inputData, randomizer);
	int wobbleSampleOffset = static_cast<int>(wobbleAmount * tickEndPosition / 100.f);
	clockTickState.initialize(tickEndPosition, wobbleSampleOffset, (tickEndPosition + wobbleAmount - tickStartPosition) / 2);
}

void applySubClockHierarchyItemToClockTickState(const WonkySubClockHierarchyItem& item, std::array<std::vector<WonkySubClockTickState>, 4>& subClockTickStates, int mainClockDuration) {
	if (item.parentStartTickIndex > -1) {
		// Calculate relative to the parent ticks within the same family
		std::vector<WonkySubClockTickState>& familyTicks = subClockTickStates[item.familyIndex];
		WonkySubClockTickState& tickState = familyTicks[item.index];

		int parentStartPosition = (item.parentStartTickIndex != startClick) ? familyTicks[item.parentStartTickIndex].tickEndPosition : 0;
		int parentEndPosition = (item.parentEndTickIndex != endClick) ? familyTicks[item.parentEndTickIndex].tickEndPosition : mainClockDuration;
		int tickDuration = (parentEndPosition - parentStartPosition) * item.parentTickDivisionIndex / item.parentTickDivisionCount;
		tickState.tickEndPosition = parentStartPosition + tickDuration;
		tickState.gateLowPosition = tickDuration / 2;
	} else {
		// Re-use the ticks of another family
		subClockTickStates[family2Index][item.index].initialize(subClockTickStates[item.overlappingIndex.first][item.overlappingIndex.second]);
	}
}

void WonkyCore::updateSubClocks(bool bpmChanged, bool clocksChanged) {
	// TODO: there are errors in here...
	// If the clocks themselves changed, re-group the clocks and determine the family and sub-clocks hierarchy
	if (clocksChanged) {
		// Check which multiplier families have active clocks and determine the highest division for each family
		m_hasSubClock.fill(false);
		m_highestFamilyMultiplications.fill(0);
		for (const ClockRatioData* clockRate : m_inputData.clockRates) {
			int familyIndex = clockRatioFamilyToIndex(clockRate->family);
			if (familyIndex != -1) {
				m_hasSubClock[familyIndex] = true;
				if (clockRate->ratio > m_highestFamilyMultiplications[familyIndex]) {
					m_highestFamilyMultiplications[familyIndex] = clockRate->ratio;
				}
			} else {
				// TODO: add the division clock to the main-clock-dependant list
			}
		}
	}

	if (clocksChanged || bpmChanged) {
		// Re-usable looper to apply sub clock hierarchy items
		auto subClockHierarchyItemApplier = [this](const WonkySubClockHierarchyLayer& layer) {
			for (const WonkySubClockHierarchyItem& item : layer.items) {
				applySubClockHierarchyItemToClockTickState(item, m_subClockTickStates, m_clockState.clockSampleDuration);
			}
		};

		// If there are clocks in the 2-based family, or there are 3-based clocks that have overlapping clicks with the 2-based family, generate the appropriate ticks for the 2-based family
		if ((m_hasSubClock[family2Index]) || (m_highestFamilyMultiplications[family3Index] > 3)) {
			int highestMultiplication = std::max(m_highestFamilyMultiplications[family2Index], m_highestFamilyMultiplications[family3Index] / 3);

			// Loop through the multiplications and apply those that are below or equal to the highest multiplication
			for (unsigned int i = 0; i < clockSubClockHierarchy[family2Index].size() && clockSubClockHierarchy[family2Index][i].ratio <= highestMultiplication; i++) {
				subClockHierarchyItemApplier(clockSubClockHierarchy[family2Index][i]);
			}
		}
		// If there are clocks in the 3-based family, determine their ticks as needed
		if (m_hasSubClock[family3Index]) {
			// Loop through the multiplications and only those that are for the highest multiplication
			for (unsigned int i = 0; i < clockSubClockHierarchy[family3Index].size(); i++) {
				if (clockSubClockHierarchy[family3Index][i].ratio == m_highestFamilyMultiplications[family3Index]) {
				subClockHierarchyItemApplier(clockSubClockHierarchy[family3Index][i]);
				}
			}
		}
		// - Finally create run through the 5- and 7-based clocks as needed
		if (m_hasSubClock[family5Index]) {
			// Loop through the multiplications and only those that are for the highest multiplication
			for (unsigned int i = 0; i < clockSubClockHierarchy[family5Index].size(); i++) {
				subClockHierarchyItemApplier(clockSubClockHierarchy[family5Index][i]);
			}
		}
		if (m_hasSubClock[family7Index]) {
			// Loop through the multiplications and only those that are for the highest multiplication
			for (unsigned int i = 0; i < clockSubClockHierarchy[family7Index].size(); i++) {
				subClockHierarchyItemApplier(clockSubClockHierarchy[family7Index][i]);
			}
		}
	}
}

void WonkyCore::triggerMainClock() {
	// Trigger the main clock
	m_clockState.gateHigh = true;
	m_clockState.dividedClockProgress = (m_clockState.dividedClockProgress + 1) % 64;
	m_listener->clockGateChanged(-1, true);

	// // Run through the active divided clocks and progress them
	// for (WonkySubClockState& subClockState : m_divSubClocks) {
	// 	subClockState.currentFamilyTick++;
	// 	if (subClockState.currentFamilyTick >= subClockState.familyTickPerClockTick) {
	// 		// TODO: send out the gate and manage gate high/low (no wobble anymore?)
	// 	}
	// }
}
