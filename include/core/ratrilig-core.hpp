#pragma once

#include <array>
#include <vector>
#include <random>
#include <memory>


struct RatriligData {
	float density;

	int clusterSize;
	float clusterEnabledChance;
	float clusterDensityFactor;

	int groupSize;
	float groupEnabledChance;
	float groupDensityFactor;

	int phraseSize;
	float phraseEnabledChance;
	float phraseDensityFactor;

	float clusterBiasAmount;
	float clusterBiasDirection;
	float groupBiasAmount;
	float groupBiasDirection;
};

struct RatriligCoreState {
	int clusterIndex = 1024;
	int groupIndex = 1024;
	int phraseIndex = 1024;

	bool clusterEnabled = true;
	bool groupEnabled = true;
	bool phraseEnabled = true;

	float density = 0.f;
	float clusterDensityFactor = 0.f;
	float groupDensityFactor = 0.f;
	float phraseDensityFactor = 0.f;

	float clusterBiasAmount;
	float groupBiasAmount;

	bool high;
};

struct RatriligChanceGenerator {
	virtual ~RatriligChanceGenerator() {};
	virtual float generateChance() = 0;
};

struct RatriligCoreListener {
	virtual ~RatriligCoreListener() {};
	
	virtual void valueChanged(int channel, int phrase, int group, int cluster, float value, bool enabled) = 0;
	virtual void clusterStarted(int channel) = 0;
	virtual void groupStarted(int channel) = 0;
	virtual void phraseStarted(int channel) = 0;
};

struct RatriligCore {
	RatriligCore(RatriligCoreListener* listener);
	RatriligCore(RatriligCoreListener* listener, std::shared_ptr<RatriligChanceGenerator> chanceGenerator);

	void process(int channel, RatriligData& data);
	void reset(int channel);
	bool isHigh(int channel);

	private:
		RatriligCoreListener* m_listener;
		std::shared_ptr<RatriligChanceGenerator> m_chanceGenerator;
		RatriligCoreState m_state[16];
};
