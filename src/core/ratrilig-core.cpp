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

	float generateChance() override {
		float chance = m_distribution(m_generator);
		return chance;
	}

	private:
		std::minstd_rand m_generator;
		std::uniform_real_distribution<float> m_distribution;
};



RatriligCore::RatriligCore(int id, RatriligCoreListener* listener) : RatriligCore(id, listener, std::make_shared<RatriligUniformChanceGenerator>()) {}
RatriligCore::RatriligCore(int id, RatriligCoreListener* listener, std::shared_ptr<RatriligChanceGenerator> chanceGenerator) : m_id(id), m_listener(listener), m_chanceGenerator(chanceGenerator) {
}

void RatriligCore::process(RatriligData& data) {
	bool clusterStarted = false;
	bool phraseStarted = false;
	bool cycleStarted = false;

	// Check if we're starting a new cluster
	if (++m_state.clusterIndex >= data.clusterSize) {
		// Check if we're starting a new phrase
		if (++m_state.phraseIndex >= data.phraseSize) {
			// Check if we're starting a new cycle
			if (++m_state.cycleIndex >= data.cycleSize) {
				m_state.cycleIndex = 0;
				cycleStarted = true;
				m_state.cycleEnabled = m_chanceGenerator->generateChance() >= data.cycleSkipChance;
				if (m_state.cycleEnabled) {
					m_state.cycleDensityModifier = data.cycleDensityModifier * ((m_chanceGenerator->generateChance() - .5f) * 2);
				} else {
					m_state.cycleDensityModifier = 0.f;
				}
			}

			// Restart the phrase and check if it should be biased
			m_state.phraseIndex = 0;
			phraseStarted = true;

			if ((data.phraseBiasAmount > 0.f) && (isBiased(m_state.cycleIndex, data.cycleSize, data.phraseBiasPosition))) {
				// A biased phrase is always enabled
				m_state.phraseEnabled = true;
				m_state.phraseBiasAmount = data.phraseBiasAmount;
			} else {
				m_state.phraseEnabled = m_chanceGenerator->generateChance() >= data.phraseSkipChance;
				m_state.phraseBiasAmount = 0.f;
			}
			// If the phrase is enabled, determine its density
			if (m_state.phraseEnabled) {
				m_state.phraseDensityModifier = data.phraseDensityModifier * ((m_chanceGenerator->generateChance() - .5f) * 2);
			} else {
				m_state.phraseDensityModifier = 0.f;
			}
		}

		// Restart the cluster and check if it should be biased
		m_state.clusterIndex = 0;
		clusterStarted = true;

		if ((data.clusterBiasAmount > 0.f) && (isBiased(m_state.phraseIndex, data.phraseSize, data.clusterBiasPosition))) {
			// A biased cluster is always enabled
			m_state.clusterEnabled = true;
			m_state.clusterBiasAmount = data.clusterBiasAmount;
		} else {
			m_state.clusterEnabled = m_chanceGenerator->generateChance() >= data.clusterSkipChance;
			m_state.clusterBiasAmount = 0.f;
		}
		// If the cluster is enabled, determine its density
		if (m_state.clusterEnabled) {
			m_state.clusterDensityModifier = data.clusterDensityModifier * ((m_chanceGenerator->generateChance() - .5f) * 2);
		} else {
			m_state.clusterDensityModifier = 0.f;
		}
	}

	float bias = std::max(m_state.clusterBiasAmount, m_state.phraseBiasAmount) + (std::min(m_state.clusterBiasAmount, m_state.phraseBiasAmount) / 2);
	m_state.density = data.density + m_state.clusterDensityModifier + m_state.phraseDensityModifier + m_state.cycleDensityModifier + bias;

	m_state.high = false;
	float chance = 0.f;
	if ((m_state.cycleEnabled) && (m_state.phraseEnabled) && (m_state.clusterEnabled)) {
		chance = m_chanceGenerator->generateChance();
		m_state.high = chance < m_state.density;
	}

	if (m_listener != nullptr) {
		m_listener->valueChanged(m_id, m_state.cycleIndex, m_state.phraseIndex, m_state.clusterIndex, std::min(m_state.density, 1.f), chance, m_state.high);
		if (clusterStarted) {
			m_listener->clusterStarted(m_id);
		}
		if (phraseStarted) {
			m_listener->phraseStarted(m_id);
		}
		if (cycleStarted) {
			m_listener->cycleStarted(m_id);
		}

		m_listener->clusterStateChanged(m_id, m_state.clusterEnabled, m_state.clusterDensityModifier, m_state.clusterBiasAmount);
		m_listener->phraseStateChanged(m_id, m_state.phraseEnabled, m_state.phraseDensityModifier, m_state.phraseBiasAmount);
		m_listener->cycleStateChanged(m_id, m_state.cycleEnabled, m_state.cycleDensityModifier);
	}
}

void RatriligCore::reset() {
	m_state.clusterIndex = 1024;
	m_state.phraseIndex = 1024;
	m_state.cycleIndex = 1024;

	m_state.clusterEnabled = true;
	m_state.phraseEnabled = true;
	m_state.cycleEnabled = true;

	m_state.high = false;

	if (m_listener != nullptr) {
		m_listener->valueChanged(m_id, 0.f, 0.f, 0.f, std::min(m_state.density, 1.f), 0.f, m_state.high);
		m_listener->clusterStateChanged(m_id, m_state.clusterEnabled, m_state.clusterDensityModifier, m_state.clusterBiasAmount);
		m_listener->phraseStateChanged(m_id, m_state.phraseEnabled, m_state.phraseDensityModifier, m_state.phraseBiasAmount);
		m_listener->cycleStateChanged(m_id, m_state.cycleEnabled, m_state.cycleDensityModifier);
	}
}

bool RatriligCore::isHigh()  {
	return m_state.high;
}
