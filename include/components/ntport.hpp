#pragma once
#include <rack.hpp>
using namespace rack;

#include "not-things.hpp"


struct NTPort : ThemedPJ301MPort, ThemeChangeListener {
	std::shared_ptr<window::Svg> ntLightSvg;
	std::shared_ptr<window::Svg> ntDarkSvg;

	ThemeIds themeId;

	NTPort();

	void themeChanged(const ThemeIds& theme) override;
};
