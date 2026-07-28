#pragma once
#include "not-things.hpp"


struct NTKnobDark16 : app::SvgKnob, ThemeChangeListener {
	NTKnobDark16();

	void themeChanged(const ThemeId& theme) override;
	void step() override;

	private:
		std::shared_ptr<window::Svg> m_ntBgLightSvg;
		std::shared_ptr<window::Svg> m_ntBgDarkSvg;

		widget::SvgWidget* bg;
		ThemeId m_themeId;
		bool m_oldPreferDark = false;
};

struct NTKnob : app::SvgKnob, ThemeChangeListener {
	NTKnob(std::string lightBackroundSvg, std::string lightForegroundSvg, std::string darkBackgroundSvg, std::string darkForegroundSvg);

	void setAngles(float minAngle, float maxAngle);

	void themeChanged(const ThemeId& theme) override;
	void step() override;

	private:
		std::shared_ptr<window::Svg> m_ntFgLightSvg;
		std::shared_ptr<window::Svg> m_ntFgDarkSvg;
		std::shared_ptr<window::Svg> m_ntBgLightSvg;
		std::shared_ptr<window::Svg> m_ntBgDarkSvg;

		widget::SvgWidget* bg;
		ThemeId m_themeId;
		bool m_oldPreferDark = false;
};

struct NTKnob32 : NTKnob {
	NTKnob32();
};

struct NTKnob35 : NTKnob {
	NTKnob35();
};

struct NTKnob40 : NTKnob {
	NTKnob40();
};
