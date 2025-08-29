#pragma once
#include <array>
#include <rack.hpp>
using namespace rack;

struct RameligDistributionData;

struct RameligDistribution : widget::Widget {
	RameligDistribution();

	void drawLayer(const DrawArgs& args, int layer) override;

	void setDistribution(std::array<float, 7>& distribution);

	void setLastAction(int lastAction);
	void setLastValue(float value, float lower, float upper);

	private:
		std::array<float, 7> m_distribution;

		int m_lastAction;
		float m_lastValue;
		float m_lastLower;
		float m_lastUpper;
};
