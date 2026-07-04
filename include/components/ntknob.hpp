#pragma once
#include "not-things.hpp"


struct NTKnobDark16 : app::SvgKnob, ThemeChangeListener {
	NTKnobDark16();

	void themeChanged(const ThemeId& theme) override;
	void step() override;

	private:
		std::shared_ptr<window::Svg> m_ntBgLightSvg;
		std::shared_ptr<window::Svg> m_ntBgDarkSvg;

		widget::SvgWidget* bg;
		ThemeId m_themeId;
		bool m_oldPreferDark = false;
};

struct NTKnob32 : app::SvgKnob, ThemeChangeListener {
	NTKnob32();

	void themeChanged(const ThemeId& theme) override;
	void step() override;

	private:
		std::shared_ptr<window::Svg> m_ntFgLightSvg;
		std::shared_ptr<window::Svg> m_ntFgDarkSvg;
		std::shared_ptr<window::Svg> m_ntBgLightSvg;
		std::shared_ptr<window::Svg> m_ntBgDarkSvg;

		widget::SvgWidget* bg;
		ThemeId m_themeId;
		bool m_oldPreferDark = false;
};
