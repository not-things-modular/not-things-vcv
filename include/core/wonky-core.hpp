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

	// The number of samples that have passed in the current wonky clock beat
	int sampleProgress = 0;

	// The number of samples that have to have passed for the internal stable clock to have completed a beat
	int clockSampleDuration = 0;
	// The number of samples that have to have passed for the wonky clock to have completed its current beatr
	int wonkyClockSampleDuration = 0;

	// The amount of wobble samples that was carried over from the previous clock beat, i.e. moved the start position of the current clock:
	// - earlier if it's a negative offset
	// - later if it's a positive offset
	int startWobbleSampleOffset = 0;
	// The amount of wobble samples that will move the end of the current clock:
	// - earlier if it's a negative offset
	// - later if it's a positive offset
	int endWobbleSampleOffset = 0;
	// The duration of the gate-high signal, relative to the start position of the current wonky clock beat
	int gateDuration = 0;
	// If the gate is high or not for this clock beat
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

	// The indices on the inputs where slow clocks are active
	std::vector<int> slowClockIndices;
	// The ticks for each family of clock multiplications
	std::array<std::vector<int>, 4> familyClockTicksOffsets;
	// The currently active sub clock tick in each family
	std::array<unsigned int, 4> familyTickProgress;
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
		void updateWonkyClockDuration();

		void detectSubClocks();
		void distributeSubClocks();

		void updateMainClockState(bool high);
		void updateSubClockStates(const std::vector<ClockRatioId>& highRatioIds, const std::vector<ClockRatioId>& lowRatioIds);
};

};