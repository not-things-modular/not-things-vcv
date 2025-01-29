#include "components/ntscrew.hpp"


NTScrew::NTScrew() {
	m_ntLightSvg = lightSvg;
	m_ntDarkSvg = darkSvg;
}

void NTScrew::themeChanged(const ThemeId& themeId) {
	m_themeId = themeId;
	switch (themeId) {
		case ThemeId::LIGHT:
			setSvg(m_ntLightSvg, m_ntLightSvg);
			break;
		case ThemeId::DARK:
			setSvg(m_ntDarkSvg, m_ntDarkSvg);
			break;
		default:
			setSvg(m_ntLightSvg, m_ntDarkSvg);
			break;
	}
}
