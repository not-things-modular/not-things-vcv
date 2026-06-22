#pragma once
#include <rack.hpp>
using namespace rack;

struct RatriligBias : widget::Widget {
	RatriligBias();

	void drawLayer(const DrawArgs& args, int layer) override;

	void setBias(float bias, int count);

	private:
		float m_bias = 0.f;
		int m_count = 0;
};
