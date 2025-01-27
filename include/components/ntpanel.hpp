#pragma once
#include <rack.hpp>
using namespace rack;

#include "not-things.hpp"


struct NTPanel : ThemedSvgPanel, ThemeChangeListener {
    DrawListener *drawListener;
	std::shared_ptr<window::Svg> ntLightSvg;
	std::shared_ptr<window::Svg> ntDarkSvg;

    ThemeIds themeId;

    NTPanel(std::shared_ptr<window::Svg> lightSvg, std::shared_ptr<window::Svg> darkSvg, DrawListener *drawListener = nullptr);

    void themeChanged(const ThemeIds& theme) override;

    void draw(const DrawArgs& args) override;
    // void drawLayer(const DrawArgs& args, int layer) override;
};

NTPanel* createNTPanel(std::string lightSvgPath, std::string darkSvgPath, DrawListener *drawListener = nullptr);
