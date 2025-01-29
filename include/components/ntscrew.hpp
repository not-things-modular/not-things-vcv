#pragma once
#include <rack.hpp>
using namespace rack;

#include "not-things.hpp"


struct NTScrew : ThemedScrew, ThemeChangeListener {
	NTScrew();

	void themeChanged(const ThemeId& theme) override;

	private:
		std::shared_ptr<window::Svg> m_ntLightSvg;
		std::shared_ptr<window::Svg> m_ntDarkSvg;

		ThemeId m_themeId;
};
