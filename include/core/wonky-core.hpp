#pragma once

namespace wonky {

struct WonkyInputData {
	int bpm = -1;

	float wobbleAmount = 0.f;
	float wobbleProbability = 0.f;

	float waverAmount = 0.f;
	float waverProbability = 0.f;

	float wanderAmount = 0.f;
	float wanderRate = 0.f;

	bool operator==(const WonkyInputData& other) const;
	bool operator!=(const WonkyInputData& other) const;
};

struct WonkyClockData {
	double clockDuration = 0;
	double clockDrift = 0.;

	int sampleDuration = 0;
	int sampleProgress = 0;

	int gateDuration = 0;
	bool gateHigh = false;

	double wonkyDrift = 0.;
};

struct SampleRateReader {
	virtual float getSampleRate() const = 0;
};

struct WonkyListener {
	virtual void clockGateChanged(bool high) = 0;
};

struct Randomizer {
	virtual float randomize(float lower, float upper) const = 0;
};

struct WonkyCore {
	WonkyCore(const SampleRateReader* sampleRateReader, WonkyListener* listener);
	WonkyCore(const SampleRateReader* sampleRateReader, const Randomizer* randomizer, WonkyListener* listener);
	~WonkyCore();

	void process(const WonkyInputData& inputData);
	void reset();

	private:
		const SampleRateReader* m_sampleRateReader;
		WonkyListener* m_listener;
		const Randomizer* m_randomizer;

		WonkyInputData m_inputData;
		WonkyClockData m_clockData;

		void determineWonkiness();
};

};