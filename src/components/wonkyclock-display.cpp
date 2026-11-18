#include "components/wonkyclock-display.hpp"
#include <algorithm>
#include <array>
#include <cmath>

WonkyClockDisplay::WonkyClockDisplay() {

}

WonkyClockDisplay::WonkyClockDisplay(bool rotated) {
	m_rotated = rotated;
}

void WonkyClockDisplay::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1) {
		return;
	}

	if (m_current != m_wonkiness) {
		m_current = m_current + (m_wonkiness - m_current) / 2;
		if (std::abs(m_current - m_wonkiness) < 0.05f) {
			m_current = m_wonkiness;
		}
	}

	float width = box.getWidth();
	float height = box.getHeight();
	nvgSave(args.vg);
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);

	if (m_rotated) {
		nvgTranslate(args.vg, width / 2.f, height / 2.f);
		nvgRotate(args.vg, NVG_PI / 2.f);
		nvgTranslate(args.vg, -height / 2.f, -width / 2.f);

		std::swap(width, height);
	}

	nvgScissor(args.vg, 0, 0, width, height);

	nvgBeginPath(args.vg);
	nvgRect(args.vg, 1.f, (height / 2) - 1.25f, width - 2, 2.5f);
	nvgFillColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xFF));
	nvgFill(args.vg);

	if (m_max != 0.f && m_current != 0.f) {
		float centre = height * 0.5f;
		float maxHeight = centre - 4.f;

		float offset, height;
		if (m_current < 0.f) {
			height = maxHeight * (-m_current) / m_max;
			offset = centre + 3.f;
		} else {
			height = maxHeight * m_current / m_max;
			offset = centre - 3.f - height;
		}
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 1.f, offset, width - 2, height);
		nvgFillColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xFF));
		nvgFill(args.vg);
	}

	nvgResetScissor(args.vg);
	nvgRestore(args.vg);
}

void WonkyClockDisplay::setWonkiness(float wonkiness, float max) {
	m_wonkiness = wonkiness;
	m_max = max;
}
