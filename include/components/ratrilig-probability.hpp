#pragma once
#include <rack.hpp>
using namespace rack;

struct RatriligProbability : widget::Widget {
	void drawLayer(const DrawArgs& args, int layer) override;
	void onResize(const ResizeEvent& e) override;

	void setBidirectional(bool bidirectional);
	void setEnabled(bool enabled);
	void setDensity(float probability);
	void setDensityFactor(float factor);
	void setBias(float bias);

	private:
		bool m_bidirectional = false;
		bool m_enabled = false;
		float m_density = 0.f;
		float m_factor = 0.f;
		float m_bias = 0.f;

		int m_x = 0.f;
		int m_y = 0.f;

		float m_innerCircleRadius = 1.f;
		float m_centerCircleRadius = 1.f;
		float m_outerCircleRadius = 1.f;

		float m_outerFrom = 0.f;
		float m_outerTo = 0.f;
		float m_centerFrom = 0.f;
		float m_centerTarget = 0.f;
		float m_centerCurrent = 0.f;

		void calculatePositions();
		void calculateTargets();
};
