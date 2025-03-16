#include "components/leddisplay.hpp"

LEDDisplay::LEDDisplay(NVGcolor foregroundColor, NVGcolor backgroundColor, std::string backgroundText, int fontSize, int align, bool lighted) {
	m_foregroundColor = foregroundColor;
	m_backgroundColor = backgroundColor;
	m_backgroundText = backgroundText;
	m_fontSize = fontSize;
	m_align = align;
	m_lighted = lighted;
	m_backgroundFill = false;
}

LEDDisplay::LEDDisplay(NVGcolor foregroundColor, NVGcolor backgroundColor, NVGcolor backgroundFillColor, std::string backgroundText, int fontSize, int align, bool lighted) {
	m_foregroundColor = foregroundColor;
	m_backgroundColor = backgroundColor;
	m_backgroundFillColor = backgroundFillColor;
	m_backgroundText = backgroundText;
	m_fontSize = fontSize;
	m_align = align;
	m_lighted = lighted;
	m_backgroundFill = true;
}

std::string LEDDisplay::getForegroundText() {
	return m_foregroundText;
}

void LEDDisplay::setForegroundText(std::string text) {
	m_foregroundText = text;
}

void LEDDisplay::drawLayer(const DrawArgs& args, int layer) {
	if ((m_lighted && layer != 1) || (!m_lighted && layer == 1)) {
		return;
	}

	std::shared_ptr<window::Font> font = APP->window->loadFont("res/fonts/DSEG7ClassicMini-Regular.ttf");
	if (font && font->handle >= 0) {
		nvgSave(args.vg);
		nvgScissor(args.vg, 0, 0, box.size.x, box.size.y);

		if (m_backgroundFill) {
			nvgBeginPath(args.vg);
			nvgFillColor(args.vg, m_backgroundFillColor);
			nvgRect(args.vg, 0, 0, getSize().x, getSize().y);
			nvgFill(args.vg);
		}

		nvgBeginPath(args.vg);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0.0);
		nvgFontSize(args.vg, m_fontSize);
		nvgTextAlign(args.vg, m_align);

		float x = (m_align & NVG_ALIGN_RIGHT) ? box.size.x : (m_align & NVG_ALIGN_CENTER) ? box.size.x / 2 : 0;
		float y = (m_align & NVG_ALIGN_BOTTOM) ? box.size.y : (m_align & NVG_ALIGN_MIDDLE) ? box.size.y / 2 : 0;

		nvgFillColor(args.vg, m_backgroundColor);
		nvgText(args.vg, x, y, m_backgroundText.c_str(), NULL);

		nvgFillColor(args.vg, m_foregroundColor);
		nvgText(args.vg, x, y, m_foregroundText.c_str(), NULL);
		nvgFill(args.vg);

		nvgResetScissor(args.vg);
		nvgRestore(args.vg);
	}
}
