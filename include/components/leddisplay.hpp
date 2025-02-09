#pragma once
#include <rack.hpp>
using namespace rack;


struct LEDDisplay : widget::Widget {
	LEDDisplay(NVGcolor foregroundColor, NVGcolor backgroundColor, std::string backgroundText, int fontSize, int align, bool lighted);

	std::string getForegroundText();
	void setForegroundText(std::string text);
	void drawLayer(const DrawArgs& args, int layer) override;

	private:
		NVGcolor m_foregroundColor;
		NVGcolor m_backgroundColor;
		std::string m_foregroundText = "";
		std::string m_backgroundText = "";
		int m_fontSize;
		int m_align;
		bool m_lighted;
};
