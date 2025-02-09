#pragma once
#include "not-things.hpp"


struct NTKnobDark16 : app::SvgKnob, ThemeChangeListener {
	NTKnobDark16();

	void themeChanged(const ThemeId& theme) override;

	private:
		std::shared_ptr<window::Svg> m_ntBgLightSvg;
		std::shared_ptr<window::Svg> m_ntBgDarkSvg;

		widget::SvgWidget* bg;
		ThemeId m_themeId;
};
