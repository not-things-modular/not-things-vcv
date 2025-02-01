#include "components/ntport.hpp"


NTPort::NTPort() {
	m_ntLightSvg = lightSvg;
	m_ntDarkSvg = darkSvg;
}

void NTPort::themeChanged(const ThemeId& themeId) {
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
