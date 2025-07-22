#include "core/ratrilig-core.hpp"


bool isBiased(int index, int size, float biasDirection) {
	if ((index == 0) && (biasDirection == 0.f)) {
		return true;
	} else if (((float) index / size > biasDirection) && ((float) (index + 1) / size <= biasDirection)) {
		return true;
	} else {
		return false;
	}
}


struct RatriligUniformChanceGenerator : RatriligChanceGenerator {
	RatriligUniformChanceGenerator() {
		m_distribution = std::uniform_real_distribution<float>(0.f, 1.f);
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
	// Check if we're starting a new cluster
	if (++m_state[channel].clusterIndex >= data.clusterSize) {
		// Check if we're starting a new group
		if (++m_state[channel].groupIndex >= data.groupSize) {
			// Check if we're starting a new phrase
			if (++m_state[channel].phraseIndex >= data.phraseSize) {
				m_state[channel].phraseIndex = 0;
				m_state[channel].phraseEnabled = m_chanceGenerator->generateChance() < data.phraseEnabledChance;
				if (m_state[channel].phraseEnabled) {
					m_state[channel].phraseDensityFactor = data.phraseDensityFactor * ((m_chanceGenerator->generateChance() - .5f) * 2);
				}
			}

			// Restart the group and check if it should be biased
			m_state[channel].groupIndex = 0;

			if ((data.groupBiasAmount > 0.f) && (isBiased(m_state[channel].clusterIndex, data.clusterSize, data.groupBiasDirection))) {
				// A biased group is always enabled
				m_state[channel].groupEnabled = true;
				m_state[channel].groupBiasAmount = data.groupBiasAmount;
			} else {
				m_state[channel].groupEnabled = m_chanceGenerator->generateChance() < data.groupEnabledChance;
			}
			// If the group is enabled, determine its density
			if (m_state[channel].groupEnabled) {
				m_state[channel].groupDensityFactor = data.groupDensityFactor * ((m_chanceGenerator->generateChance() - .5f) * 2);
			}
		}

		// Restart the cluster and check if it should be biased
		m_state[channel].clusterIndex = 0;

		if ((data.clusterBiasAmount > 0.f) && (isBiased(m_state[channel].groupIndex, data.groupSize, data.clusterBiasDirection))) {
			// A biased cluster is always enabled
			m_state[channel].clusterEnabled = true;
			m_state[channel].clusterBiasAmount = data.clusterBiasAmount;
		} else {
			m_state[channel].clusterEnabled = m_chanceGenerator->generateChance() < data.clusterEnabledChance;
		}
		// If the cluster is enabled, determine its density
		if (m_state[channel].clusterEnabled) {
			m_state[channel].clusterDensityFactor = data.clusterDensityFactor * ((m_chanceGenerator->generateChance() - .5f) * 2);
		}

		float bias = std::max(m_state[channel].clusterBiasAmount, m_state[channel].groupBiasAmount) + (std::min(m_state[channel].clusterBiasAmount, m_state[channel].groupBiasAmount) / 2);
		m_state[channel].density = data.density + m_state[channel].clusterDensityFactor + m_state[channel].groupDensityFactor + m_state[channel].phraseDensityFactor + bias;
	}

	m_state[channel].high = false;
	if ((m_state[channel].phraseEnabled) && (m_state[channel].groupEnabled) && (m_state[channel].clusterEnabled)) {
		m_state[channel].high = m_chanceGenerator->generateChance() < m_state[channel].density;
	}

	if (m_listener != nullptr) {
		m_listener->valueChanged(channel, m_state[channel].phraseIndex, m_state[channel].groupIndex, m_state[channel].clusterIndex, m_state[channel].density, m_state[channel].high);
		if (m_state[channel].clusterIndex == 0) {
			m_listener->clusterStarted(channel);
		}
		if (m_state[channel].groupIndex == 0) {
			m_listener->groupStarted(channel);
		}
		if (m_state[channel].phraseIndex == 0) {
			m_listener->phraseStarted(channel);
		}
	}
}

void RatriligCore::reset(int channel) {
	m_state[channel].clusterIndex = 1024;
	m_state[channel].groupIndex = 1024;
	m_state[channel].phraseIndex = 1024;
	m_state[channel].high = false;
}

bool RatriligCore::isHigh(int channel)  {
	return m_state[channel].high;
}
