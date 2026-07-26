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

	// float width = (box.getWidth() - 4) / m_count;

	nvgSave(args.vg);
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
	nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());

	nvgBeginPath(args.vg);
	nvgRect(args.vg, 1.f, (box.getHeight() / 2) - (box.getWidth() / 2) + 1, box.getWidth() - 2, box.getWidth() - 2);
	nvgFillColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xFF));
	nvgFill(args.vg);

	nvgBeginPath(args.vg);
	nvgRect(args.vg, 1.f, (box.getHeight() / 2), box.getWidth() - 2, box.getHeight() / 2 * m_wonkiness / m_max);
	nvgFillColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xFF));
	nvgFill(args.vg);

	nvgResetScissor(args.vg);
	nvgRestore(args.vg);
}

void WonkyClockDisplay::setWonkiness(float wonkiness, float max) {
	m_wonkiness = wonkiness;
	m_max = max;
}
