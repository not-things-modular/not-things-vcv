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
		case ThemeId::VCV:
			if (settings::preferDarkPanels) {
				bg->setSvg(m_ntBgDarkSvg);
				m_oldPreferDark = true;
			} else {
				bg->setSvg(m_ntBgLightSvg);
				m_oldPreferDark = false;
			}
			break;
		default:
			bg->setSvg(m_ntBgLightSvg);
			break;
	}
	fb->setDirty();
}

void NTKnobDark16::step() {
	SvgKnob::step();
	if ((m_themeId == ThemeId::VCV) && (m_oldPreferDark != settings::preferDarkPanels)) {
		themeChanged(ThemeId::VCV);
	}
}

NTKnob::NTKnob(std::string lightBackroundSvg, std::string lightForegroundSvg, std::string darkBackgroundSvg, std::string darkForegroundSvg) {
	minAngle = -0.75 * M_PI;
	maxAngle = 0.75 * M_PI;

	bg = new widget::SvgWidget;
	fb->addChildBelow(bg, tw);
	
	m_ntFgLightSvg = APP->window->loadSvg(asset::plugin(pluginInstance, lightForegroundSvg));
	m_ntFgDarkSvg = APP->window->loadSvg(asset::plugin(pluginInstance, darkForegroundSvg));
	m_ntBgLightSvg = APP->window->loadSvg(asset::plugin(pluginInstance, lightBackroundSvg));
	m_ntBgDarkSvg = APP->window->loadSvg(asset::plugin(pluginInstance, darkBackgroundSvg));
	setSvg(m_ntFgLightSvg);
	bg->setSvg(m_ntBgLightSvg);
}

void NTKnob::setAngles(float min, float max) {
	minAngle = min;
	maxAngle = max;
}

void NTKnob::themeChanged(const ThemeId& themeId) {
	m_themeId = themeId;
	switch (themeId) {
		case ThemeId::LIGHT:
			setSvg(m_ntFgLightSvg);
			bg->setSvg(m_ntBgLightSvg);
			break;
		case ThemeId::DARK:
			setSvg(m_ntFgDarkSvg);
			bg->setSvg(m_ntBgDarkSvg);
			break;
		case ThemeId::VCV:
			if (settings::preferDarkPanels) {
				setSvg(m_ntFgDarkSvg);
				bg->setSvg(m_ntBgDarkSvg);
				m_oldPreferDark = true;
			} else {
				setSvg(m_ntFgLightSvg);
				bg->setSvg(m_ntBgLightSvg);
				m_oldPreferDark = false;
			}
			break;
		default:
			setSvg(m_ntFgLightSvg);
			bg->setSvg(m_ntBgLightSvg);
			break;
	}
	fb->setDirty();
}

void NTKnob::step() {
	SvgKnob::step();
	if ((m_themeId == ThemeId::VCV) && (m_oldPreferDark != settings::preferDarkPanels)) {
		themeChanged(ThemeId::VCV);
	}
}

NTKnob32::NTKnob32() : NTKnob("res/knob-32-light-bg.svg", "res/knob-32-light.svg", "res/knob-32-dark-bg.svg", "res/knob-32-dark.svg") {}
NTKnob35::NTKnob35() : NTKnob("res/knob-35-light-bg.svg", "res/knob-35-light.svg", "res/knob-35-dark-bg.svg", "res/knob-35-dark.svg") {}
NTKnob40::NTKnob40() : NTKnob("res/knob-40-light-bg.svg", "res/knob-40-light.svg", "res/knob-40-dark-bg.svg", "res/knob-40-dark.svg") {}
