#include "components/ntpanel.hpp"
#include <math.h>


NTPanel::NTPanel(std::shared_ptr<window::Svg> lightSvg, std::shared_ptr<window::Svg> darkSvg, DrawListener *drawListener) {
	this->drawListener = drawListener;
	this->ntLightSvg = lightSvg;
	this->ntDarkSvg = darkSvg;
	setBackground(lightSvg, darkSvg);
}

void NTPanel::themeChanged(const ThemeId& themeId) {
	this->themeId = themeId;
	switch (themeId) {
		case ThemeId::LIGHT:
			setBackground(this->ntLightSvg, this->ntLightSvg);
			break;
		case ThemeId::DARK:
			setBackground(this->ntDarkSvg, this->ntDarkSvg);
			break;
		default:
			setBackground(this->ntLightSvg, this->ntDarkSvg);
			break;
	}
}

void NTPanel::draw(const DrawArgs& args) {
	if (drawListener) {
		drawListener->draw(args);
	}
	ThemedSvgPanel::draw(args);
}

NTPanel* createNTPanel(std::string lightSvgPath, std::string darkSvgPath, DrawListener *drawListener) {
	return new NTPanel(window::Svg::load(lightSvgPath), window::Svg::load(darkSvgPath), drawListener);
}
