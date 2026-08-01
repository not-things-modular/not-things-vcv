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
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_64, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_1, 64), "/ 64"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_32, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_1, 32), "/ 32"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_16, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_1, 16), " /16"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_12, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_1, 12), " /16"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_8, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_1, 8), " / 8"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_7, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_1, 7), " / 7"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_6, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_1, 6), " / 6"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_5, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_1, 5), " / 5"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_4, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_1, 4), " / 4"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_3, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_1, 3), " / 3"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_2, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_1, 2), " / 2"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_DIV_1, wonky::ClockRatioType::RATIO_DIVIDE, wonky::ClockRatioFamily::FAMILY_0, 1), "  *1"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_2, wonky::ClockRatioType::RATIO_MULTIPLY, wonky::ClockRatioFamily::FAMILY_2, 2), " * 2"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_3, wonky::ClockRatioType::RATIO_MULTIPLY, wonky::ClockRatioFamily::FAMILY_3, 3), " * 3"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_4, wonky::ClockRatioType::RATIO_MULTIPLY, wonky::ClockRatioFamily::FAMILY_2, 4), " * 4"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_5, wonky::ClockRatioType::RATIO_MULTIPLY, wonky::ClockRatioFamily::FAMILY_5, 5), " * 5"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_6, wonky::ClockRatioType::RATIO_MULTIPLY, wonky::ClockRatioFamily::FAMILY_3, 6), " * 6"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_7, wonky::ClockRatioType::RATIO_MULTIPLY, wonky::ClockRatioFamily::FAMILY_7, 7), " * 7"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_8, wonky::ClockRatioType::RATIO_MULTIPLY, wonky::ClockRatioFamily::FAMILY_2, 8), " * 8"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_12, wonky::ClockRatioType::RATIO_MULTIPLY, wonky::ClockRatioFamily::FAMILY_3, 12), " *12"),
	WonkyClockRatio(wonky::ClockRatioData(wonky::ClockRatioId::RATE_MULT_16, wonky::ClockRatioType::RATIO_MULTIPLY, wonky::ClockRatioFamily::FAMILY_2, 16), " *16"),
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