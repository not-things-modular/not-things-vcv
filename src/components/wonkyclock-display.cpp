#include "components/wonkyclock-display.hpp"
#include <algorithm>
#include <array>
#include <cmath>

WonkyClockDisplay::WonkyClockDisplay() {

}

void WonkyClockDisplay::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1) {
		return;
	}

	if (m_current != m_wonkiness) {
		if (m_wonkiness > m_current) {
			m_current = m_current + (m_wonkiness - m_current) / 2;
			// m_current = std::min(m_wonkiness, m_current + 4);
		} else {
			m_current = m_current + (m_wonkiness - m_current) / 2;
			// m_current = std::max(m_wonkiness, m_current - 4);
		}
	}

	nvgSave(args.vg);
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
	nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());

	nvgBeginPath(args.vg);
	nvgRect(args.vg, 1.f, (box.getHeight() / 2) - 1.25f, box.getWidth() - 2, 2.5f);
	nvgFillColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xFF));
	nvgFill(args.vg);

	float centre = box.getHeight() * 0.5f;
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
	nvgRect(args.vg, 1.f, offset, box.getWidth() - 2, height);
	nvgFillColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xFF));
	nvgFill(args.vg);

	nvgResetScissor(args.vg);
	nvgRestore(args.vg);
}

void WonkyClockDisplay::setWonkiness(float wonkiness, float max) {
	m_wonkiness = wonkiness;
	m_max = max;
}
