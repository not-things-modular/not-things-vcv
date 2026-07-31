#pragma once

namespace wonky {

struct SampleRateReader {
	virtual float getSampleRate() const = 0;
};

struct WonkyListener {
	virtual void clockGateChanged(bool high) = 0;
	virtual void wanderChanged(float wander, float max) = 0;
	virtual void waverChanged(float waver, float max) = 0;
	virtual void wobbleChanged(float wobble, float max) = 0;
};

struct Randomizer {
	virtual ~Randomizer() {};
	virtual float randomize(float lower, float upper) = 0;
	virtual float randomizeGaussian(float mean, float stddev) = 0;
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

struct WonkyClockData {
	// The core clock duration
	double clockDuration = 0;
	// The amount of drift on the core clock due to the current sample rate
	double clockDrift = 0.;

	// The number of samples that have passed in the current clock beat
	int sampleProgress = 0;

	// The number of samples that have to have passed for the current (internal) clock to have completed one beat
	int clockSampleDuration = 0;
	// The impact of wobble on the start of the gate-high for the next clock beat, relative to the clockSampleDuration:
	// - negative if the gate should go high before the (internal) clock
	// - positive if the gate should go high after the (internal) clock
	int currentWobbleSampleOffset = 0;
	// The number of samples that are still remaining from the wobble of the last clock signal since it was a positive offset,
	// i.e. the gate-high signal must be delayed with this amount. Will be decreased each time a sample passes.
	int wobbleDelay = 0;

	// The number of samples that the gate should remain high. Already takes wander, waver and wobble into account.
	int gateDuration = 0;
	// Flag to indicate if the gate is currently high or not
	bool gateHigh = false;

	// If no wander, waver or wobble are applied, the number of drift that is introduced in the stable clock due the accuracy within the current sample rate
	double wonkyDrift = 0.;
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
		WonkyClockData m_clockData;

		bool m_reset = false;

		void updateBpm(int bpm);
		void updateWonkiness();
};

};