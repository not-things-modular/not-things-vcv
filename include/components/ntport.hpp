#pragma once
#include <rack.hpp>
using namespace rack;

#include "not-things.hpp"


struct NTPort : ThemedPJ301MPort, ThemeChangeListener {
	std::shared_ptr<window::Svg> ntLightSvg;
	std::shared_ptr<window::Svg> ntDarkSvg;

	ThemeId themeId;

	NTPort();

	void themeChanged(const ThemeId& theme) override;
};
