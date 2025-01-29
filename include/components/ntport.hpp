#pragma once
#include <rack.hpp>
using namespace rack;

#include "not-things.hpp"


struct NTPort : ThemedPJ301MPort, ThemeChangeListener {
	NTPort();

	void themeChanged(const ThemeId& theme) override;

	private:
		std::shared_ptr<window::Svg> m_ntLightSvg;
		std::shared_ptr<window::Svg> m_ntDarkSvg;

		ThemeId m_themeId;
};
