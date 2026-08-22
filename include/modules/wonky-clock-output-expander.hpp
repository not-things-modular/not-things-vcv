#pragma once
#include <array>
#include "not-things.hpp"
#include "core/wonky-core.hpp"

struct LEDDisplay;

struct WonkyClockRatio {
	constexpr WonkyClockRatio(wonky::ClockRatioData data, const char (&display)[5]) : data(data), display{display[0], display[1], display[2], display[3], display[4]} {}

	const wonky::ClockRatioData data;
	const char display[5];
};

constexpr int wonkyClockRatioCount = wonky::ClockRatioId::RATE_COUNT;
constexpr std::array<WonkyClockRatio, wonkyClockRatioCount> wonkyClockRatios{{
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_64, wonky::ClockRatioType::RATIO_DIVIDE, 64), "/ 64"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_56, wonky::ClockRatioType::RATIO_DIVIDE, 56), "/ 56"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_49, wonky::ClockRatioType::RATIO_DIVIDE, 49), "/ 49"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_48, wonky::ClockRatioType::RATIO_DIVIDE, 48), "/ 48"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_40, wonky::ClockRatioType::RATIO_DIVIDE, 40), "/ 40"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_32, wonky::ClockRatioType::RATIO_DIVIDE, 32), "/ 32"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_28, wonky::ClockRatioType::RATIO_DIVIDE, 28), " /28"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_27, wonky::ClockRatioType::RATIO_DIVIDE, 27), " /27"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_25, wonky::ClockRatioType::RATIO_DIVIDE, 25), " /25"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_24, wonky::ClockRatioType::RATIO_DIVIDE, 24), " /24"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_20, wonky::ClockRatioType::RATIO_DIVIDE, 20), " /20"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_16, wonky::ClockRatioType::RATIO_DIVIDE, 16), " /16"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_14, wonky::ClockRatioType::RATIO_DIVIDE, 14), " /14"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_12, wonky::ClockRatioType::RATIO_DIVIDE, 12), " /12"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_10, wonky::ClockRatioType::RATIO_DIVIDE, 10), " /10"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_9, wonky::ClockRatioType::RATIO_DIVIDE, 9), " / 9"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_8, wonky::ClockRatioType::RATIO_DIVIDE, 8), " / 8"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_7, wonky::ClockRatioType::RATIO_DIVIDE, 7), " / 7"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_6, wonky::ClockRatioType::RATIO_DIVIDE, 6), " / 6"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_5, wonky::ClockRatioType::RATIO_DIVIDE, 5), " / 5"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_4, wonky::ClockRatioType::RATIO_DIVIDE, 4), " / 4"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_3, wonky::ClockRatioType::RATIO_DIVIDE, 3), " / 3"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_2, wonky::ClockRatioType::RATIO_DIVIDE, 2), " / 2"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_1, wonky::ClockRatioType::RATIO_DIVIDE, 1), "  *1"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_2, wonky::ClockRatioType::RATIO_MULTIPLY, 2), " * 2"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_3, wonky::ClockRatioType::RATIO_MULTIPLY, 3), " * 3"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_4, wonky::ClockRatioType::RATIO_MULTIPLY, 4), " * 4"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_5, wonky::ClockRatioType::RATIO_MULTIPLY, 5), " * 5"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_6, wonky::ClockRatioType::RATIO_MULTIPLY, 6), " * 6"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_7, wonky::ClockRatioType::RATIO_MULTIPLY, 7), " * 7"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_8, wonky::ClockRatioType::RATIO_MULTIPLY, 8), " * 8"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_9, wonky::ClockRatioType::RATIO_MULTIPLY, 9), " * 9"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_10, wonky::ClockRatioType::RATIO_MULTIPLY, 10), " *10"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_12, wonky::ClockRatioType::RATIO_MULTIPLY, 12), " *12"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_14, wonky::ClockRatioType::RATIO_MULTIPLY, 14), " *14"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_16, wonky::ClockRatioType::RATIO_MULTIPLY, 16), " *16"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_20, wonky::ClockRatioType::RATIO_MULTIPLY, 20), " *20"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_24, wonky::ClockRatioType::RATIO_MULTIPLY, 24), " *24"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_25, wonky::ClockRatioType::RATIO_MULTIPLY, 25), " *25"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_27, wonky::ClockRatioType::RATIO_MULTIPLY, 27), " *27"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_28, wonky::ClockRatioType::RATIO_MULTIPLY, 28), " *28"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_32, wonky::ClockRatioType::RATIO_MULTIPLY, 32), " *32"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_40, wonky::ClockRatioType::RATIO_MULTIPLY, 40), " *40"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_48, wonky::ClockRatioType::RATIO_MULTIPLY, 48), " *48"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_49, wonky::ClockRatioType::RATIO_MULTIPLY, 49), " *49"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_56, wonky::ClockRatioType::RATIO_MULTIPLY, 56), " *56"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_64, wonky::ClockRatioType::RATIO_MULTIPLY, 64), " *64"),
}};

struct WonkyClockOutputExpanderModule : NTModule, DrawListener {
	enum ParamId {
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
		NUM_LIGHTS
	};

	LEDDisplay* m_ratioLeds[8];

	WonkyClockOutputExpanderModule();

	void draw(const widget::Widget::DrawArgs& args) override;

	private:
		int m_displayedRatios[8];
};

struct WonkyClockOutputExpanderWidget : NTModuleWidget {
	WonkyClockOutputExpanderWidget(WonkyClockOutputExpanderModule* module);
};