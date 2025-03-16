#include "components/timeseq-display.hpp"
#include <algorithm>

void TimeSeqDisplay::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1) {
		return;
	}

	nvgSave(args.vg);

	if (m_voltagePoints.size() > 0) {
		nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);

		nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());
		nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);

		int offset = 0;
		for (std::vector<TimeSeqVoltagePoints>::iterator it = m_voltagePoints.begin(); it != m_voltagePoints.end(); it++) {
			float v = std::min(std::max(it->voltage, -10.f), 10.f);
			float factor = it->age < 31 ? (30.f - it->age) / 30.f : 0.f;

			nvgStrokeColor(args.vg, nvgRGBA(0xFF, 0x50, 0x50, 0x55 + 0x55 * factor));
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0, 0, 0x20 + 0x20 * factor));

			nvgBeginPath(args.vg);
			nvgStrokeWidth(args.vg, 1.f);
			nvgRoundedRect(args.vg, 1.f, 166.f - offset, 37.f, 9.f, 2.f);
			nvgStroke(args.vg);

			nvgBeginPath(args.vg);
			nvgStrokeWidth(args.vg, 0.f);
			nvgRoundedRect(args.vg, 19.5f, 167.f - offset, 17.5f * (v / 10.f), 7.f, 1.f);
			nvgFill(args.vg);
			nvgStroke(args.vg);

			offset += 11;
		}

		nvgResetScissor(args.vg);
	}

	nvgRestore(args.vg);
}
