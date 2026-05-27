#include "core/ratrilig-core.hpp"
#include <chrono>

bool static isBiased(int index, int size, float biasDirection) {
	int biasIndex = biasDirection >= 1.f ? size - 1 : size * biasDirection;

	return biasIndex == index;
}


struct RatriligUniformChanceGenerator : RatriligChanceGenerator {
	RatriligUniformChanceGenerator() {
		m_distribution = std::uniform_real_distribution<float>(0.f, 1.f);
		m_generator.seed(std::random_device{}());
	}

	~RatriligUniformChanceGenerator() {}

	float generateSkipChance() override {
		return m_distribution(m_generator);
	}

	float generateDensityModifier() override {
		return (m_distribution(m_generator) - 0.5f) * 2;
	}

	float generateTrigger() override {
		return m_distribution(m_generator);
	}

	private:
		std::minstd_rand m_generator;
		std::uniform_real_distribution<float> m_distribution;
};

bool RatriligCoreLayerState::operator==(const RatriligCoreLayerState& other) const {
	return index == other.index &&
		enabled == other.enabled &&
		densityModifier == other.densityModifier;
}

bool RatriligCoreBiasedLayerState::operator==(const RatriligCoreBiasedLayerState& other) const {
	return index == other.index &&
		enabled == other.enabled &&
		densityModifier == other.densityModifier &&
		biasAmount == other.biasAmount;
}

bool RatriligCoreState::operator==(const RatriligCoreState& other) const {
	return clusterState == other.clusterState &&
		phraseState == other.phraseState &&
		cycleState == other.cycleState &&
		high == other.high;
}

RatriligCoreProcessor::RatriligCoreProcessor(std::shared_ptr<RatriligChanceGenerator> chanceGenerator) : m_chanceGenerator(chanceGenerator) {}
RatriligCoreProcessor::~RatriligCoreProcessor() {}

void RatriligCoreProcessor::setState(std::shared_ptr<RatriligCoreState> state) {
	m_state = state;
}

void RatriligCoreProcessor::advanceCluster(RatriligData& data, RatriligProcessorProgress& progress) {
	progress.clusterStarted = advanceLayer(m_state->clusterState, data.clusterData);
}

void RatriligCoreProcessor::determineClusterDensity(RatriligData& data, RatriligProcessorProgress& progress) {
	// Update the density when a new cluster started
	if (progress.clusterStarted) {
		m_state->clusterState.enabled = false;
		determineBiasedLayerDensity(data.clusterData, m_state->clusterState, m_state->phraseState.index, data.phraseData.size);
	}
}

void RatriligCoreProcessor::advancePhrase(RatriligData& data, RatriligProcessorProgress& progress) {
	// The phrase is advanced when a new cluster started
	if (progress.clusterStarted) {
		progress.phraseStarted = advanceLayer(m_state->phraseState, data.phraseData);
	} else {
		progress.phraseStarted = false;
	}
}

void RatriligCoreProcessor::determinePhraseDensity(RatriligData& data, RatriligProcessorProgress& progress) {
	// Update the density when a new phrase started
	if (progress.phraseStarted) {
		m_state->phraseState.enabled = false;
		determineBiasedLayerDensity(data.phraseData, m_state->phraseState, m_state->cycleState.index, data.cycleData.size);
	}
}

void RatriligCoreProcessor::advanceCycle(RatriligData& data, RatriligProcessorProgress& progress) {
	// A cycle advances when a new phrase starts
	if (progress.phraseStarted) {
		progress.cycleStarted = advanceLayer(m_state->cycleState, data.cycleData);
	} else {
		progress.cycleStarted = false;
	}
}

void RatriligCoreProcessor::determineCycleDensity(RatriligData& data, RatriligProcessorProgress& progress) {
	// Update the density when a new cycle started
	if (progress.cycleStarted) {
		m_state->cycleState.enabled = false;
		determineLayerDensity(data.cycleData, m_state->cycleState);
	}
}

void RatriligCoreProcessor::determineDensity(RatriligData& data, RatriligProcessorProgress& progress) {
	float bias = std::max(m_state->clusterState.biasAmount, m_state->phraseState.biasAmount) + (std::min(m_state->clusterState.biasAmount, m_state->phraseState.biasAmount) / 2);
	progress.density = data.density + m_state->clusterState.densityModifier + m_state->phraseState.densityModifier + m_state->cycleState.densityModifier + bias;
}

void RatriligCoreProcessor::determineHigh(RatriligProcessorProgress& progress) {
	m_state->high = false;
	progress.chance = 0.f;

	if ((m_state->cycleState.enabled) && (m_state->phraseState.enabled) && (m_state->clusterState.enabled)) {
		progress.chance = m_chanceGenerator->generateTrigger();
		m_state->high = progress.chance <= progress.density;
	}
}

void RatriligCoreProcessor::determineBiasedLayerDensity(RatriligBiasedLayerData& data, RatriligCoreBiasedLayerState& state, int parentLayerIndex, int parentLayerSize) {
	// Check for bias, and determine if the layer should be enabled.
	if ((data.biasAmount > 0.f) && (isBiased(parentLayerIndex, parentLayerSize, data.biasPosition))) {
		// A biased layer is always enabled.
		state.enabled = true;
		state.biasAmount = data.biasAmount;
	} else {
		// The layer is not biased, let normal density detection check the enabled chance.
		state.biasAmount = 0.f;
	}

	determineLayerDensity(data, state);
}

void RatriligCoreProcessor::determineLayerDensity(RatriligLayerData& data, RatriligCoreLayerState& state) {
	// If this is a biased layer, the bias density determination will already have enabled it. So only do this for a layer that is not yet enabled.
	if (!state.enabled) {
		state.enabled = m_chanceGenerator->generateSkipChance() >= data.skipChance;
	}

	// If the layer is enabled, determine its density
	if (state.enabled) {
		state.densityModifier = data.densityModifier * m_chanceGenerator->generateDensityModifier();
	} else {
		state.densityModifier = 0.f;
	}
}

bool RatriligCoreProcessor::advanceLayer(RatriligCoreLayerState& state, RatriligLayerData& data) {
	// Advance the layer
	state.index++;

	// Check if we're starting a new cycle
	bool started = (state.index >= data.size);
	if (started) {
		state.index = 0;
	}

	return started;
}


RatriligCore::RatriligCore(int id, RatriligCoreListener* listener) : RatriligCore(id, listener, std::make_shared<RatriligCoreProcessor>(std::make_shared<RatriligUniformChanceGenerator>())) {}
RatriligCore::RatriligCore(int id, RatriligCoreListener* listener, std::shared_ptr<RatriligCoreProcessor> processor) : m_id(id), m_listener(listener), m_processor(processor) {
	m_state = std::make_shared<RatriligCoreState>();
	m_processor->setState(m_state);
}

void RatriligCore::process(RatriligData& data) {
	RatriligProcessorProgress progress;

	// First advance the different layers as needed
	m_processor->advanceCluster(data, progress);
	m_processor->advancePhrase(data, progress);
	m_processor->advanceCycle(data, progress);

	// Then determine the new densities of the layers that started a new iteration
	m_processor->determineClusterDensity(data, progress);
	m_processor->determinePhraseDensity(data, progress);
	m_processor->determineCycleDensity(data, progress);

	// Determine the overall density and trigger state
	m_processor->determineDensity(data, progress);
	m_processor->determineHigh(progress);

	// Notify the listener if needed
	if (m_listener != nullptr) {
		m_listener->valueChanged(m_id, m_state->cycleState.index, m_state->phraseState.index, m_state->clusterState.index, std::min(progress.density, 1.f), progress.chance, m_state->high);
		if (progress.clusterStarted) {
			m_listener->clusterStarted(m_id);
		}
		if (progress.phraseStarted) {
			m_listener->phraseStarted(m_id);
		}
		if (progress.cycleStarted) {
			m_listener->cycleStarted(m_id);
		}

		m_listener->clusterStateChanged(m_id, m_state->clusterState.enabled, m_state->clusterState.densityModifier, m_state->clusterState.biasAmount);
		m_listener->phraseStateChanged(m_id, m_state->phraseState.enabled, m_state->phraseState.densityModifier, m_state->phraseState.biasAmount);
		m_listener->cycleStateChanged(m_id, m_state->cycleState.enabled, m_state->cycleState.densityModifier);
	}
}

void RatriligCore::reset() {
	m_state->clusterState.index = RATRILIG_INDEX_WRAP_ON_ADVANCE;
	m_state->phraseState.index = RATRILIG_INDEX_WRAP_ON_ADVANCE;
	m_state->cycleState.index = RATRILIG_INDEX_WRAP_ON_ADVANCE;

	m_state->clusterState.enabled = true;
	m_state->phraseState.enabled = true;
	m_state->cycleState.enabled = true;

	m_state->high = false;

	if (m_listener != nullptr) {
		m_listener->valueChanged(m_id, 0.f, 0.f, 0.f, 0.f, 0.f, false);
		m_listener->clusterStateChanged(m_id, m_state->clusterState.enabled, m_state->clusterState.densityModifier, m_state->clusterState.biasAmount);
		m_listener->phraseStateChanged(m_id, m_state->phraseState.enabled, m_state->phraseState.densityModifier, m_state->phraseState.biasAmount);
		m_listener->cycleStateChanged(m_id, m_state->cycleState.enabled, m_state->cycleState.densityModifier);
	}
}

bool RatriligCore::isHigh()  {
	return m_state->high;
}
