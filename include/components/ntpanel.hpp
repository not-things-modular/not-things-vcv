#pragma once
#include <rack.hpp>
using namespace rack;

#include "not-things.hpp"


struct NTPanel : ThemedSvgPanel, ThemeChangeListener {
	NTPanel(std::shared_ptr<window::Svg> lightSvg, std::shared_ptr<window::Svg> darkSvg, DrawListener *drawListener = nullptr);

	void themeChanged(const ThemeId& theme) override;

	void draw(const DrawArgs& args) override;

	private:
		DrawListener *m_drawListener;
		std::shared_ptr<window::Svg> m_ntLightSvg;
		std::shared_ptr<window::Svg> m_ntDarkSvg;

		ThemeId m_themeId;
};

NTPanel* createNTPanel(std::string lightSvgPath, std::string darkSvgPath, DrawListener *drawListener = nullptr);
