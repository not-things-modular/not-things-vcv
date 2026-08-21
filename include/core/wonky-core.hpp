#pragma once

#include <array>
#include <vector>

namespace wonky {

struct SampleRateReader {
	virtual ~SampleRateReader() {};
	virtual float getSampleRate() const = 0;
};

struct WonkyListener {
	virtual ~WonkyListener() {};
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
	FAMILY_0, // The main clock rate
	FAMILY_1, // The slower-then-main clocks
	FAMILY_2, // Clocks that run at multiples of two of the main clock
	FAMILY_3_2, // Clocks that run at three times the speed of the main clock, each time multiplying by 2 (3, 6, 12, ...)
	FAMILY_5_2, // Clocks that run at five times the speed of the main clock, each time multiplying by 2 (5, 10, 20, ...)
	FAMILY_7_2, // Clocks that run at seven times the speed of the main clock, each time multiplying by 2 (7, 14, 28, ...)
	FAMILY_3_3, // Clocks that run at three times the speed of the main clock, each time multiplying by 3 (3, 9, 27, ...)
	FAMILY_5_5, // Clocks that run at five times the speed of the main clock, each time multiplying by 5 (5, 25)
	FAMILY_7_7 // Clocks that run at seven times the speed of the main clock, each time multiplying by 7 )49)
};

enum ClockRatioType {
	RATIO_DIVIDE, // The clock is slower then (or equal to) the main clock
	RATIO_MULTIPLY, // The clock is faster then the main clock
};

// Included clock ratios:
// - Multiples of two (2, 4, 8, 16, ...)
// - Triplets, each time doubling in amount (3, 6, 12, ...)
// - triplets, multiplying by 3 (3, 9, 18, ...)
// - Five-based clocks, each time doubling in amount (5, 10, 20, ...)
// - Five-based clocks, each time multiplying by 5 (5, 25)
// - Seven-based clocks, each time mutliplying by 7 (7, 49)
// All with a maximum of 64
enum ClockRatioId {
	RATE_DIV_64,
	RATE_DIV_56,
	RATE_DIV_49,
	RATE_DIV_48,
	RATE_DIV_40,
	RATE_DIV_32,
	RATE_DIV_28,
	RATE_DIV_27,
	RATE_DIV_25,
	RATE_DIV_24,
	RATE_DIV_20,
	RATE_DIV_16,
	RATE_DIV_14,
	RATE_DIV_12,
	RATE_DIV_10,
	RATE_DIV_9,
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
	RATE_MULT_9,
	RATE_MULT_10,
	RATE_MULT_12,
	RATE_MULT_14,
	RATE_MULT_16,
	RATE_MULT_20,
	RATE_MULT_24,
	RATE_MULT_25,
	RATE_MULT_27,
	RATE_MULT_28,
	RATE_MULT_32,
	RATE_MULT_40,
	RATE_MULT_48,
	RATE_MULT_49,
	RATE_MULT_56,
	RATE_MULT_64,

	RATE_COUNT,
	NO_RATE,
};

struct ClockRatioData {
	constexpr ClockRatioData(ClockRatioId id, ClockRatioType type, ClockRatioFamily family, unsigned int ratio) : id(id), type(type), family(family), ratio(ratio) {};

	ClockRatioId id;
	ClockRatioType type;
	ClockRatioFamily family;

	unsigned int ratio;

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
	unsigned int sampleProgress = 0;

	// The number of samples that have to have passed for the internal stable clock to have completed a beat
	unsigned int clockSampleDuration = 0;
	// The number of samples that have to have passed for the wonky clock to have completed its current beatr
	unsigned int wonkyClockSampleDuration = 0;

	// The amount of wobble samples that was carried over from the previous clock beat, i.e. moved the start position of the current clock:
	// - earlier if it's a negative offset
	// - later if it's a positive offset
	int startWobbleSampleOffset = 0;
	// The amount of wobble samples that will move the end of the current clock:
	// - earlier if it's a negative offset
	// - later if it's a positive offset
	int endWobbleSampleOffset = 0;
	// The duration of the gate-high signal, relative to the start position of the current wonky clock beat
	unsigned int gateDuration = 0;
	// If the gate is high or not for this clock beat
	bool gateHigh = false;

	// If no wander, waver or wobble are applied, the number of drift that is introduced in the stable clock due the accuracy within the current sample rate
	double wonkyDrift = 0.;

	// How many ticks of a 64-divider clock have already passed (to allow re-calculation of changed output clock settings)
	unsigned int dividedClockProgress = 0;
};

struct WonkySubClockState {
	// Flag to indicate that subclocks changed in the input, so they have to be re-evaluated on the start of the next main clock
	bool clocksChanged = false;

	// The indices on the inputs where slow clocks are active
	std::vector<int> slowClockIndices;

	std::array<unsigned int, 8> fastClockNextSamplePosition;
	std::array<unsigned int, 8> fastClockProgress;
};

struct WonkyCore {
	WonkyCore(const SampleRateReader* sampleRateReader, WonkyListener* listener);
	WonkyCore(const SampleRateReader* sampleRateReader, Randomizer* randomizer, WonkyListener* listener);
	~WonkyCore();

	void process(const WonkyInputData& inputData);
	void reset();
	void sampleRateChanged();

	private:
		const SampleRateReader* m_sampleRateReader;
		WonkyListener* m_listener;
		Randomizer* m_randomizer;

		WonkyInputData m_inputData;
		Wonkiness m_wonkiness;
		WonkyClockState m_clockState;
		WonkySubClockState m_subClockState;

		bool m_reset = false;
		bool m_sampleRateChanged = false;

		void updateBpm(float bpm);
		void updateWonkyClockDuration();

		void detectSubClocks();
		void distributeSubClocks();

		void updateMainClockState(bool high);
		void updateSubClockStates(const std::vector<ClockRatioId>& highRatioIds, const std::vector<ClockRatioId>& lowRatioIds);
};

};