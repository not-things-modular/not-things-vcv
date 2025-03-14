#include "components/timeseq-display.hpp"
#include <algorithm>

void TimeSeqDisplay::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1) {
		return;
	}

	nvgSave(args.vg);

	nvgGlobalAlpha(args.vg, 1.f);
	std::shared_ptr<window::Font> font = APP->window->loadFont("res/fonts/DSEG7ClassicMini-Regular.ttf");
	if (font && font->handle >= 0) {
		nvgBeginPath(args.vg);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0.0);
		nvgFontSize(args.vg, 10);
		nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);

		if (layer == 1) {
			nvgFillColor(args.vg, nvgRGB(0xFF, 0x50, 0x50));
			nvgText(args.vg, 2, box.size.y - 2, m_time.c_str(), NULL);
		} else {
			nvgFillColor(args.vg, nvgRGB(0x40, 0x40, 0x40));
			nvgText(args.vg, 2, box.size.y - 2, "88:88", NULL);
		}
	}

	if (m_voltagePoints.size() > 0) {
		nvgGlobalAlpha(args.vg, 0.5f);
		nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());

		int rowCount = std::max((int) m_voltagePoints.size(), 4);
		float width = box.getWidth() / rowCount;
		float height = (box.getHeight() - 16.f) * .5f;

		if (layer == 1) {
			int count = 0;
			nvgBeginPath(args.vg);
			for (std::vector<TimeSeqVoltagePoints>::iterator it = m_voltagePoints.begin(); it != m_voltagePoints.end(); it++) {
				float v = std::min(std::max(it->voltage, -10.f), 10.f);
				nvgRect(args.vg, (width * count) + .5f, height + 1.f, width - 1, -(height * v / 10));
				count++;
			}
			nvgFillColor(args.vg, nvgRGB(255, 0, 0));
			nvgFill(args.vg);
		}
		nvgBeginPath(args.vg);
		for (int i = 0; i < m_voltagePoints.size(); i++) {
			float x = (width * i) + .5f;
			int v = -std::min(std::max(m_voltagePoints[i].voltage, -10.f), 10.f) + 10;
			int lower = std::min(v, 10);
			int upper = std::max(v, 10);
			for (int j = lower; j < upper; j++) {
				float y = 1.f + (height / 10 * (j + 1)) + .25;
				nvgMoveTo(args.vg, x, y);
				nvgLineTo(args.vg, x + width - 1, y);
			}
		}
		nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
		nvgStrokeWidth(args.vg, 1.f);
		nvgStroke(args.vg);

		nvgResetScissor(args.vg);
	}

	nvgRestore(args.vg);
}
