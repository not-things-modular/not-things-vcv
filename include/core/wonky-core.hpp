#pragma once

#include <array>
#include <vector>

namespace wonky {

struct SampleRateReader {
	virtual float getSampleRate() const = 0;
};

struct WonkyListener {
	virtual void clockGateChanged(int index, bool high) = 0;
	virtual void wanderChanged(float wander, float max) = 0;
	virtual void waverChanged(float waver, float max) = 0;
	virtual void wobbleChanged(float wobble, float max) = 0;
};

struct Randomizer {
	virtual ~Randomizer() {};
	virtual float randomize(float lower, float upper) = 0;
	virtual float randomizeGaussian(float mean, float stddev) = 0;
};

enum ClockRatioFamily {
	// Set the value of the enum to the number of main clock ticks in one tick of the family
	// so that we can use it as a divisor during processing
	FAMILY_0 = 0, // the main clock (i.e x1)
	FAMILY_1 = 1, // all divider ratios
	FAMILY_2 = 2, // x2, x4, x8 and x16
	FAMILY_3 = 3, // x3, x6 and x12
	FAMILY_5 = 5, // x5
	FAMILY_7 = 7  // x7
};

enum ClockRatioType {
	RATIO_DIVIDE, // The clock is slower then the main clock
	RATIO_MULTIPLY, // The clock is faster then the main clock
};

enum ClockRatioId {
	RATE_DIV_64,
	RATE_DIV_32,
	RATE_DIV_16,
	RATE_DIV_12,
	RATE_DIV_8,
	RATE_DIV_7,
	RATE_DIV_6,
	RATE_DIV_5,
	RATE_DIV_4,
	RATE_DIV_3,
	RATE_DIV_2,
	RATE_DIV_1,
	RATE_MULT_2,
	RATE_MULT_3,
	RATE_MULT_4,
	RATE_MULT_5,
	RATE_MULT_6,
	RATE_MULT_7,
	RATE_MULT_8,
	RATE_MULT_12,
	RATE_MULT_16,

	RATE_COUNT,
	NO_RATE,
	MIN_MULT_RATE = RATE_MULT_2,
	MAX_MULT_RATE = RATE_MULT_16,
};

struct ClockRatioData {
	constexpr ClockRatioData(ClockRatioId id, ClockRatioType type, ClockRatioFamily family, int ratio) : id(id), type(type), family(family), ratio(ratio) {};

	ClockRatioId id;
	ClockRatioType type;
	ClockRatioFamily family;

	int ratio;

	bool operator==(const ClockRatioData& other) const;
	bool operator!=(const ClockRatioData& other) const;
};

struct WonkyInputData {
	float bpm = -1.f;

	float wobbleAmount = 0.f;
	float wobbleProbability = 0.f;

	float waverAmount = 0.f;
	float waverProbability = 0.f;

	float wanderAmount = 0.f;
	float wanderRate = 0.f;

	bool linked = false;

	std::array<const ClockRatioData*, 8> clockRates;

	bool operator==(const WonkyInputData& other) const;
	bool operator!=(const WonkyInputData& other) const;
};

struct Wonkiness {
	Wonkiness(Randomizer* randomizer);

	float getWanderAmount() const;
	float getWaverAmount() const;
	float getWobbleAmount() const;

	bool isWonky() const;
	void determineWonkiness(const WonkyInputData& inputData);

	private:
		Randomizer* m_randomizer;

		float m_wanderAmount = 0.f;
		float m_waverAmount = 0.f;
		float m_wobbleAmount = 0.f;
};

// The current processing state for the main clock
struct WonkyClockState {
	// The core clock duration
	double clockDuration = 0;
	// The amount of drift on the core clock due to the current sample rate
	double clockDrift = 0.;

	// The number of samples that have passed in the current clock beat
	int sampleProgress = 0;

	// The number of samples that have to have passed for the current (internal, stable) clock to have completed one beat
	int clockSampleDuration = 0;
	// The impact of wobble on the start of the gate-high for the next clock beat, relative to the clockSampleDuration:
	// - negative if the gate should go high before the (internal) clock
	// - positive if the gate should go high after the (internal) clock
	int currentWobbleSampleOffset = 0;
	// The number of samples that are still remaining from the wobble of the last clock signal since it had a positive offset,
	// i.e. the gate-high signal must be delayed with this amount. Will be decreased each time a sample passes.
	int wobbleDelay = 0;

	// The number of samples that the gate should remain high. Already takes wander, waver and wobble into account.
	int gateDuration = 0;
	// Flag to indicate if the gate is currently high or not
	bool gateHigh = false;

	// If no wander, waver or wobble are applied, the number of drift that is introduced in the stable clock due the accuracy within the current sample rate
	double wonkyDrift = 0.;

	// How many ticks of a 64-divider clock have already passed (to allow re-calculation of changed output clock settings)
	int dividedClockProgress = 0;
};

struct WonkySubClockState {
	WonkySubClockState();

	// Flag to indicate that subclocks changed in the input, so they have to be re-evaluated on the start of the next main clock
	bool clocksChanged = false;

	// Flag to indicate for each subclock family if there are any clocks active for it
	std::array<bool, 4> hasSubClock;

	// The active divided clocks (i.e. slower then the main clock)
	std::vector<ClockRatioId> activeSlowClocks;
	// The ticks for each family of clock multiplications
	std::array<std::vector<int>, 4> familyClockTicksOffsets;
	// The currently active sub clock tick in each family
	std::array<int, 4> familyTickProgress;
};

struct WonkyCore {
	WonkyCore(const SampleRateReader* sampleRateReader, WonkyListener* listener);
	WonkyCore(const SampleRateReader* sampleRateReader, Randomizer* randomizer, WonkyListener* listener);
	~WonkyCore();

	void process(const WonkyInputData& inputData);
	void reset();

	private:
		const SampleRateReader* m_sampleRateReader;
		WonkyListener* m_listener;
		Randomizer* m_randomizer;

		WonkyInputData m_inputData;
		Wonkiness m_wonkiness;
		WonkyClockState m_clockState;
		WonkySubClockState m_subClockState;

		bool m_reset = false;

		void updateBpm(int bpm);
		void updateWonkiness();

		void detectSubClocks();
		void distributeSubClocks();

		void triggerMainClock();
		void triggerSubClocks(const std::vector<ClockRatioId>& highRatioIds, const std::vector<ClockRatioId>& lowRatioIds);
};

};