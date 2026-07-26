#pragma once
#include <rack.hpp>
using namespace rack;

struct WonkyClockDisplay : widget::Widget {
	WonkyClockDisplay();

	void drawLayer(const DrawArgs& args, int layer) override;

	void setWonkiness(float wonkiness, float max);

	private:
		float m_wonkiness = 0.f;
		float m_max = 0;
};
