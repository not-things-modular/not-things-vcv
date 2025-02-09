#include "components/ntknob.hpp"


extern Plugin* pluginInstance;

NTKnobDark16::NTKnobDark16() {
	minAngle = -0.75 * M_PI;
	maxAngle = 0.75 * M_PI;

	bg = new widget::SvgWidget;
	fb->addChildBelow(bg, tw);
	
	m_ntBgLightSvg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob-dark-16-bg.svg"));
	m_ntBgDarkSvg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob-dark-16-bg-dark.svg"));
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob-dark-16.svg")));
	bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob-dark-16-bg.svg")));
}

void NTKnobDark16::themeChanged(const ThemeId& themeId) {
	m_themeId = themeId;
	switch (themeId) {
		case ThemeId::LIGHT:
			bg->setSvg(m_ntBgLightSvg);
			break;
		case ThemeId::DARK:
			bg->setSvg(m_ntBgDarkSvg);
			break;
		default:
			bg->setSvg(m_ntBgLightSvg);
			break;
	}
	fb->setDirty();
}
