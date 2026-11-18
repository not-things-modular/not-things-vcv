#pragma once
#include <rack.hpp>
using namespace rack;

struct WonkyClockDisplay : widget::Widget {
	WonkyClockDisplay();
	WonkyClockDisplay(bool rotated);

	void drawLayer(const DrawArgs& args, int layer) override;

	void setWonkiness(float wonkiness, float max);

	private:
		bool m_rotated = false;
		float m_wonkiness = 0.f;
		float m_max = 0;

		float m_current = 0.f;
};
