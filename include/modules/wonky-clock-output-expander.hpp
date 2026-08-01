#pragma once
#include <array>
#include "not-things.hpp"

struct LEDDisplay;

enum WonkyClockRatioFamily {
	FAMILY_1, // x1 and all slower-then-one ratios
	FAMILY_2, // x2, x4, x8 and x16
	FAMILY_3, // x3, x6 and x12
	FAMILY_5, // x5
	FAMILY_7  // x7
};

struct WonkyClockRatio {
	constexpr WonkyClockRatio(int ratio, WonkyClockRatioFamily family, const char (&display)[5]) : ratio(ratio), family(family), display{display[0], display[1], display[2], display[3], display[4]} {}

	int ratio;
	WonkyClockRatioFamily family;
	const char display[5];
};

constexpr int wonkyClockRatioCount = 21;
constexpr std::array<WonkyClockRatio, wonkyClockRatioCount> wonkyClockRatios{{
	WonkyClockRatio(64, FAMILY_1, "/ 64"),
	WonkyClockRatio(32, FAMILY_1, "/ 32"),
	WonkyClockRatio(16, FAMILY_1, " /16"),
	WonkyClockRatio(12, FAMILY_1, " /12"),
	WonkyClockRatio(8, FAMILY_1, " / 8"),
	WonkyClockRatio(7, FAMILY_1, " / 7"),
	WonkyClockRatio(6, FAMILY_1, " / 6"),
	WonkyClockRatio(5, FAMILY_1, " / 5"),
	WonkyClockRatio(4, FAMILY_1, " / 4"),
	WonkyClockRatio(3, FAMILY_1, " / 3"),
	WonkyClockRatio(2, FAMILY_1, " / 2"),
	WonkyClockRatio(1, FAMILY_1, "  *1"),
	WonkyClockRatio(2, FAMILY_2, " * 2"),
	WonkyClockRatio(3, FAMILY_3, " * 3"),
	WonkyClockRatio(4, FAMILY_2, " * 4"),
	WonkyClockRatio(5, FAMILY_5, " * 5"),
	WonkyClockRatio(6, FAMILY_3, " * 6"),
	WonkyClockRatio(7, FAMILY_7, " * 7"),
	WonkyClockRatio(8, FAMILY_2, " * 8"),
	WonkyClockRatio(12, FAMILY_3, " *12"),
	WonkyClockRatio(16, FAMILY_2, " *16")
}};

struct WonkyClockOutputExpanderModule : NTModule, DrawListener {
	enum ParamId {
		PARAM_WOBBLE,
		ENUMS(PARAM_RATIOS, 8),
		NUM_PARAMS
	};
	enum InputId {
		NUM_INPUTS
	};
	enum OutputId {
		ENUMS(OUT_CLOCKS, 8),
		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_WOBBLE,
		NUM_LIGHTS
	};

	LEDDisplay* m_ratioLeds[8];

	WonkyClockOutputExpanderModule();

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;

	private:
		int m_displayedRatios[8];

		dsp::BooleanTrigger m_wobbleTrigger;
		bool m_wobble = true;
};

struct WonkyClockOutputExpanderWidget : NTModuleWidget {
	WonkyClockOutputExpanderWidget(WonkyClockOutputExpanderModule* module);
};