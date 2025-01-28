#include "components/ntscrew.hpp"


NTScrew::NTScrew() {
	this->ntLightSvg = this->lightSvg;
	this->ntDarkSvg = this->darkSvg;
}

void NTScrew::themeChanged(const ThemeIds& themeId) {
	this->themeId = themeId;
	switch (themeId) {
		case ThemeIds::LIGHT:
			setSvg(this->ntLightSvg, this->ntLightSvg);
			break;
		case ThemeIds::DARK:
			setSvg(this->ntDarkSvg, this->ntDarkSvg);
			break;
		default:
			setSvg(this->ntLightSvg, this->ntDarkSvg);
			break;
	}
}
