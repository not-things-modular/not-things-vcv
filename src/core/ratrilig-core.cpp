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



RatriligCore::RatriligCore(RatriligCoreListener* listener) : RatriligCore(listener, std::make_shared<RatriligUniformChanceGenerator>()) {}
RatriligCore::RatriligCore(RatriligCoreListener* listener, std::shared_ptr<RatriligChanceGenerator> chanceGenerator) : m_listener(listener), m_chanceGenerator(chanceGenerator) {
}

void RatriligCore::process(int channel, RatriligData& data) {
	bool clusterStarted = false;
	bool phraseStarted = false;
	bool cycleStarted = false;

	// Check if we're starting a new cluster
	if (++m_state[channel].clusterIndex >= data.clusterSize) {
		// Check if we're starting a new phrase
		if (++m_state[channel].phraseIndex >= data.phraseSize) {
			// Check if we're starting a new cycle
			if (++m_state[channel].cycleIndex >= data.cycleSize) {
				m_state[channel].cycleIndex = 0;
				cycleStarted = true;
				m_state[channel].cycleEnabled = m_chanceGenerator->generateChance() >= data.cycleSkipChance;
				if (m_state[channel].cycleEnabled) {
					m_state[channel].cycleDensityModifier = data.cycleDensityModifier * ((m_chanceGenerator->generateChance() - .5f) * 2);
				} else {
					m_state[channel].cycleDensityModifier = 0.f;
				}
			}

			// Restart the phrase and check if it should be biased
			m_state[channel].phraseIndex = 0;
			phraseStarted = true;

			if ((data.phraseBiasAmount > 0.f) && (isBiased(m_state[channel].cycleIndex, data.cycleSize, data.phraseBiasPosition))) {
				// A biased phrase is always enabled
				m_state[channel].phraseEnabled = true;
				m_state[channel].phraseBiasAmount = data.phraseBiasAmount;
			} else {
				m_state[channel].phraseEnabled = m_chanceGenerator->generateChance() >= data.phraseSkipChance;
				m_state[channel].phraseBiasAmount = 0.f;
			}
			// If the phrase is enabled, determine its density
			if (m_state[channel].phraseEnabled) {
				m_state[channel].phraseDensityModifier = data.phraseDensityModifier * ((m_chanceGenerator->generateChance() - .5f) * 2);
			} else {
				m_state[channel].phraseDensityModifier = 0.f;
			}
		}

		// Restart the cluster and check if it should be biased
		m_state[channel].clusterIndex = 0;
		clusterStarted = true;

		if ((data.clusterBiasAmount > 0.f) && (isBiased(m_state[channel].phraseIndex, data.phraseSize, data.clusterBiasPosition))) {
			// A biased cluster is always enabled
			m_state[channel].clusterEnabled = true;
			m_state[channel].clusterBiasAmount = data.clusterBiasAmount;
		} else {
			m_state[channel].clusterEnabled = m_chanceGenerator->generateChance() >= data.clusterSkipChance;
			m_state[channel].clusterBiasAmount = 0.f;
		}
		// If the cluster is enabled, determine its density
		if (m_state[channel].clusterEnabled) {
			m_state[channel].clusterDensityModifier = data.clusterDensityModifier * ((m_chanceGenerator->generateChance() - .5f) * 2);
		} else {
			m_state[channel].clusterDensityModifier = 0.f;
		}
	}

	float bias = std::max(m_state[channel].clusterBiasAmount, m_state[channel].phraseBiasAmount) + (std::min(m_state[channel].clusterBiasAmount, m_state[channel].phraseBiasAmount) / 2);
	m_state[channel].density = data.density + m_state[channel].clusterDensityModifier + m_state[channel].phraseDensityModifier + m_state[channel].cycleDensityModifier + bias;

	m_state[channel].high = false;
	float chance = 0.f;
	if ((m_state[channel].cycleEnabled) && (m_state[channel].phraseEnabled) && (m_state[channel].clusterEnabled)) {
		chance = m_chanceGenerator->generateChance();
		m_state[channel].high = chance < m_state[channel].density;
	}

	if (m_listener != nullptr) {
		m_listener->valueChanged(channel, m_state[channel].cycleIndex, m_state[channel].phraseIndex, m_state[channel].clusterIndex, std::min(m_state[channel].density, 1.f), chance, m_state[channel].high);
		if (clusterStarted) {
			m_listener->clusterStarted(channel);
		}
		if (phraseStarted) {
			m_listener->phraseStarted(channel);
		}
		if (cycleStarted) {
			m_listener->cycleStarted(channel);
		}

		m_listener->clusterStateChanged(channel, m_state[channel].clusterEnabled, m_state[channel].clusterDensityModifier, m_state[channel].clusterBiasAmount);
		m_listener->phraseStateChanged(channel, m_state[channel].phraseEnabled, m_state[channel].phraseDensityModifier, m_state[channel].phraseBiasAmount);
		m_listener->cycleStateChanged(channel, m_state[channel].cycleEnabled, m_state[channel].cycleDensityModifier);
	}
}

void RatriligCore::reset(int channel) {
	m_state[channel].clusterIndex = 1024;
	m_state[channel].phraseIndex = 1024;
	m_state[channel].cycleIndex = 1024;

	m_state[channel].clusterEnabled = true;
	m_state[channel].phraseEnabled = true;
	m_state[channel].cycleEnabled = true;
	
	m_state[channel].high = false;

	if (m_listener != nullptr) {
		m_listener->valueChanged(channel, 0.f, 0.f, 0.f, std::min(m_state[channel].density, 1.f), 0.f, m_state[channel].high);
		m_listener->clusterStateChanged(channel, m_state[channel].clusterEnabled, m_state[channel].clusterDensityModifier, m_state[channel].clusterBiasAmount);
		m_listener->phraseStateChanged(channel, m_state[channel].phraseEnabled, m_state[channel].phraseDensityModifier, m_state[channel].phraseBiasAmount);
		m_listener->cycleStateChanged(channel, m_state[channel].cycleEnabled, m_state[channel].cycleDensityModifier);
	}
}

bool RatriligCore::isHigh(int channel)  {
	return m_state[channel].high;
}
