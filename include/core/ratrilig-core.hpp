#pragma once

#include <array>
#include <vector>
#include <random>
#include <memory>


#define RATRILIG_INDEX_WRAP_ON_ADVANCE 1024

struct RatriligLayerData {
	int size;
	float skipChance;
	float densityModifier;
};

struct RatriligBiasedLayerData : RatriligLayerData {
	float biasAmount;
	float biasPosition;
};

struct RatriligData {
	float density;

	RatriligBiasedLayerData clusterData;
	RatriligBiasedLayerData phraseData;
	RatriligLayerData cycleData;
};

struct RatriligCoreLayerState {
	int index = RATRILIG_INDEX_WRAP_ON_ADVANCE;
	bool enabled = true;
	float densityModifier = 0.f;

	bool operator==(const RatriligCoreLayerState& other) const;
};

struct RatriligCoreBiasedLayerState : RatriligCoreLayerState {
	float biasAmount = 0.f;

	bool operator==(const RatriligCoreBiasedLayerState& other) const;
};

struct RatriligCoreState {
	RatriligCoreBiasedLayerState clusterState;
	RatriligCoreBiasedLayerState phraseState;
	RatriligCoreLayerState cycleState;

	bool high = false;

	bool operator==(const RatriligCoreState& other) const;
};

struct RatriligProcessorProgress {
	bool clusterStarted = false;
	bool phraseStarted = false;
	bool cycleStarted = false;

	float density = 0.f;
	float chance = 0.f;
};

struct RatriligChanceGenerator {
	virtual ~RatriligChanceGenerator() {};
	// Range of [0, 1] (skip to don't skip)
	virtual float generateSkipChance() = 0;
	// Range of [-1, 1] (negative to positive modifer)
	virtual float generateDensityModifier() = 0;
	// Range of [0, 1] (don't trigger to trigger)
	virtual float generateTrigger() = 0;
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

struct RatriligCoreProcessor {
	RatriligCoreProcessor(std::shared_ptr<RatriligChanceGenerator> chanceGenerator);
	virtual ~RatriligCoreProcessor();

	virtual void setState(std::shared_ptr<RatriligCoreState> state);

	virtual void advanceCluster(RatriligData& data, RatriligProcessorProgress& progress);
	virtual void advancePhrase(RatriligData& data, RatriligProcessorProgress& progress);
	virtual void advanceCycle(RatriligData& data, RatriligProcessorProgress& progress);

	virtual void determineClusterDensity(RatriligData& data, RatriligProcessorProgress& progress);
	virtual void determinePhraseDensity(RatriligData& data, RatriligProcessorProgress& progress);
	virtual void determineCycleDensity(RatriligData& data, RatriligProcessorProgress& progress);

	virtual void determineDensity(RatriligData& data, RatriligProcessorProgress& progress);
	virtual void determineHigh(RatriligProcessorProgress& progress);

	private:
		std::shared_ptr<RatriligChanceGenerator> m_chanceGenerator;
		std::shared_ptr<RatriligCoreState> m_state;

		void determineBiasedLayerDensity(RatriligBiasedLayerData& data, RatriligCoreBiasedLayerState& state, int parentLayerIndex, int parentLayerSize);
		void determineLayerDensity(RatriligLayerData& data, RatriligCoreLayerState& state);
		bool advanceLayer(RatriligCoreLayerState& state, RatriligLayerData& data);
};

struct RatriligCore {
	RatriligCore(int id, RatriligCoreListener* listener);
	RatriligCore(int id, RatriligCoreListener* listener, std::shared_ptr<RatriligCoreProcessor> processor);

	void process(RatriligData& data);
	void reset();
	bool isHigh();

	private:
		int m_id;

		RatriligCoreListener* m_listener;
		std::shared_ptr<RatriligCoreProcessor> m_processor;
		std::shared_ptr<RatriligCoreState> m_state;
};
