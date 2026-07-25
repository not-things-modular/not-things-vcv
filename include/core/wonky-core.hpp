#pragma once

namespace wonky {

struct WonkyInputData {
	int bpm = -1;

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

struct WonkyCore {
	WonkyCore(const SampleRateReader* sampleRateReader, WonkyListener* listener);
	
	void process(const WonkyInputData& inputData);
	void reset();

	private:
		const SampleRateReader* m_sampleRateReader;
		WonkyListener* m_listener;

		WonkyInputData m_inputData;
		WonkyClockData m_clockData;
};

};