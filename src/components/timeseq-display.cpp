#include "components/timeseq-display.hpp"

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

		nvgFillColor(args.vg, nvgRGB(0x40, 0x40, 0x40));
		nvgText(args.vg, 2, box.size.y - 2, "88:88", NULL);

		nvgFillColor(args.vg, nvgRGB(0xFF, 0x50, 0x50));
		nvgText(args.vg, 2, box.size.y - 2, "12:34", NULL);
	}

	nvgStrokeColor(args.vg, nvgRGBA(0xff, 0x00, 0x00, 0xFF));
	nvgGlobalAlpha(args.vg, 0.35f);
	Rect b = Rect(Vec(0, 0), box.size);
	nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
	
	nvgBeginPath(args.vg);
	for (std::vector<TimeSeqVoltagePoints>::iterator it = m_voltagePoints.begin(); it != m_voltagePoints.end(); it++) {
		bool drawing = false;

		for (int i = 0; i < TIMESEQ_SAMPLE_COUNT; i++) {
			if (it->points[i] != 0xFFFF) {
				if (drawing && !(m_currentVoltagePointIndex == i)) {
					nvgLineTo(args.vg, i * .5f, it->points[i] * 195 / 20 + 97.5f);
				} else {
					nvgMoveTo(args.vg, i * .5f, it->points[i] * 195 / 20 + 97.5f);
				}
				drawing = true;
			} else {
				drawing = false;
			}
		}
	}
	
	nvgLineCap(args.vg, NVG_ROUND);
	nvgMiterLimit(args.vg, 2.0);
	nvgStrokeWidth(args.vg, 1.5);
	nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
	nvgStroke(args.vg);

	nvgResetScissor(args.vg);
	nvgRestore(args.vg);
}
