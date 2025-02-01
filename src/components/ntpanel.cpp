#include "components/ntpanel.hpp"
#include <math.h>


NTPanel::NTPanel(std::shared_ptr<window::Svg> lightSvg, std::shared_ptr<window::Svg> darkSvg, DrawListener *drawListener) {
	m_drawListener = drawListener;
	m_ntLightSvg = lightSvg;
	m_ntDarkSvg = darkSvg;
	setBackground(lightSvg, darkSvg);
}

void NTPanel::themeChanged(const ThemeId& themeId) {
	m_themeId = themeId;
	switch (themeId) {
		case ThemeId::LIGHT:
			setBackground(m_ntLightSvg, m_ntLightSvg);
			break;
		case ThemeId::DARK:
			setBackground(m_ntDarkSvg, m_ntDarkSvg);
			break;
		default:
			setBackground(m_ntLightSvg, m_ntDarkSvg);
			break;
	}
}

void NTPanel::draw(const DrawArgs& args) {
	if (m_drawListener) {
		m_drawListener->draw(args);
	}
	ThemedSvgPanel::draw(args);
}

NTPanel* createNTPanel(std::string lightSvgPath, std::string darkSvgPath, DrawListener *drawListener) {
	return new NTPanel(window::Svg::load(lightSvgPath), window::Svg::load(darkSvgPath), drawListener);
}
