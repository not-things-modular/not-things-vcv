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

struct RatriligCore {
	RatriligCore();
	RatriligCore(std::shared_ptr<RatriligChanceGenerator> chanceGenerator);

	void process(int channel, RatriligData& data);
	void reset(int channel);
	bool isHigh(int channel);

	private:
		std::shared_ptr<RatriligChanceGenerator> m_chanceGenerator;
		RatriligCoreState m_state[16];
};
