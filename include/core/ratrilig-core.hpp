#pragma once

#include <array>
#include <vector>
#include <random>
#include <memory>


struct RatriligData {
	float density;

	int clusterSize;
	float clusterSkipChance;
	float clusterDensityModifier;

	int phraseSize;
	float phraseSkipChance;
	float phraseDensityModifier;

	int cycleSize;
	float cycleSkipChance;
	float cycleDensityModifier;

	float clusterBiasAmount;
	float clusterBiasPosition;
	float phraseBiasAmount;
	float phraseBiasPosition;
};

struct RatriligCoreState {
	int clusterIndex = 1024;
	int phraseIndex = 1024;
	int cycleIndex = 1024;

	bool clusterEnabled = true;
	bool phraseEnabled = true;
	bool cycleEnabled = true;

	float density = 0.f;
	float clusterDensityModifier = 0.f;
	float phraseDensityModifier = 0.f;
	float cycleDensityModifier = 0.f;

	float clusterBiasAmount = 0.f;
	float phraseBiasAmount = 0.f;

	bool high = false;
};

struct RatriligChanceGenerator {
	virtual ~RatriligChanceGenerator() {};
	virtual float generateChance() = 0;
};

struct RatriligCoreListener {
	virtual ~RatriligCoreListener() {};

	virtual void clusterStateChanged(int coreId, bool enabled, float density, float bias) = 0;
	virtual void phraseStateChanged(int coreId, bool enabled, float density, float bias) = 0;
	virtual void cycleStateChanged(int coreId, bool enabled, float density) = 0;

	virtual void valueChanged(int coreId, int cycle, int phrase, int cluster, float target, float value, bool enabled) = 0;
	virtual void clusterStarted(int coreId) = 0;
	virtual void phraseStarted(int coreId) = 0;
	virtual void cycleStarted(int coreId) = 0;
};

struct RatriligCore {
	RatriligCore(int id, RatriligCoreListener* listener);
	RatriligCore(int id, RatriligCoreListener* listener, std::shared_ptr<RatriligChanceGenerator> chanceGenerator);

	void process(RatriligData& data);
	void reset();
	bool isHigh();

	private:
		int m_id;

		RatriligCoreListener* m_listener;
		std::shared_ptr<RatriligChanceGenerator> m_chanceGenerator;
		RatriligCoreState m_state;
};
