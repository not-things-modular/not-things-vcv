#pragma once
#include <rack.hpp>
using namespace rack;

#include "not-things.hpp"


struct NTScrew : ThemedScrew, ThemeChangeListener {
	std::shared_ptr<window::Svg> ntLightSvg;
	std::shared_ptr<window::Svg> ntDarkSvg;

	ThemeId themeId;

	NTScrew();

	void themeChanged(const ThemeId& theme) override;
};
