#pragma once
#include <rack.hpp>
using namespace rack;

#include <array>

struct RatriligProgress : widget::Widget {
	RatriligProgress();

	void drawLayer(const DrawArgs& args, int layer) override;

	void setClusterCount(int clusterCount);
	void setGroupCount(int groupCount);
	void setPhraseCount(int phraseCount);
	void setPositionValue(int position, float value);

	private:
		int m_clusterCount = 8;
		int m_groupCount = 4;
		int m_phraseCount = 4;

		int m_position = 20;

		std::array<float, 32*32*32> m_values;
};
