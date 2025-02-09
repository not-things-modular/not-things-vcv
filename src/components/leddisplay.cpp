#include "components/leddisplay.hpp"

LEDDisplay::LEDDisplay(NVGcolor foregroundColor, NVGcolor backgroundColor, std::string backgroundText, int fontSize, int align, bool lighted) {
	m_foregroundColor = foregroundColor;
	m_backgroundColor = backgroundColor;
	m_backgroundText = backgroundText;
	m_fontSize = fontSize;
	m_align = align;
	m_lighted = lighted;
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
		nvgBeginPath(args.vg);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0.0);
		nvgFontSize(args.vg, m_fontSize);
		nvgTextAlign(args.vg, m_align);

		nvgFillColor(args.vg, m_backgroundColor);
		nvgText(args.vg, box.size.y, 0, m_backgroundText.c_str(), NULL);

		nvgFillColor(args.vg, m_foregroundColor);
		nvgText(args.vg, box.size.y, 0, m_foregroundText.c_str(), NULL);
	}
}
