#include "modules/wonky-clock-output-expander.hpp"
#include "components/leddisplay.hpp"
#include "components/ntport.hpp"
#include "components/ntknob.hpp"

#include "core/wonky-core.hpp"

using namespace wonky;

struct WonkyRatioParam : ParamQuantity {
	float getDisplayValue() override {
		int value = static_cast<int>(getValue());
		float displayValue = wonkyClockRatios[value].data.ratio;
		if ((wonkyClockRatios[value].data.ratio != 1) && (wonkyClockRatios[value].data.family == ClockRatioFamily::FAMILY_1)) {
			displayValue = -displayValue;
		}
		return displayValue;
	}

	void setDisplayValue(float displayValue) override {
		int index;
		if (displayValue < 0.f) {
			index = 0;
			for (int i = wonkyClockRatioCount - 1; i >= 0; i--) {
				if (wonkyClockRatios[i].data.family == ClockRatioFamily::FAMILY_1 && wonkyClockRatios[i].data.ratio != 1.f) {
					if (wonkyClockRatios[i].data.ratio <= displayValue) {
						index = i;
						break;
					}
				}
			}
		} else {
			index = wonkyClockRatioCount - 1;
			for (int i = 0; i < wonkyClockRatioCount; i++) {
				if (wonkyClockRatios[i].data.family != ClockRatioFamily::FAMILY_1 || wonkyClockRatios[i].data.ratio == 1.f) {
					if (wonkyClockRatios[i].data.ratio >= displayValue) {
						index = i;
						break;
					}
				}
			}
		}
		setValue(index);
	}

	std::string getUnit() override {
		int index = static_cast<int>(getValue());
		if (wonkyClockRatios[index].data.family != ClockRatioFamily::FAMILY_1 || wonkyClockRatios[index].data.ratio == 1.f) {
			return std::string(" x");
		} else {
			return std::string(" ÷");
		}
	}
};

WonkyClockOutputExpanderModule::WonkyClockOutputExpanderModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	for (int i = 0; i < 8; i++) {
		configParam<WonkyRatioParam>(PARAM_RATIOS + i, 0.f, wonkyClockRatioCount - 1, 11.f, string::f("Clock Ratio %d", (i + 1)));
		paramQuantities[PARAM_RATIOS + i]->snapEnabled = true;
		configOutput(OUT_CLOCKS + i, string::f("Clock %d", (i + 1)));

		m_displayedRatios[i] = -1;
		m_ratioLeds[i] = nullptr;
	}

	lights[LightId::LIGHT_WOBBLE].setBrightness(m_wobble);
}

void WonkyClockOutputExpanderModule::draw(const widget::Widget::DrawArgs& args) {
	for (int i = 0; i < 8; i++) {
		int ratio = static_cast<int>(getParam(PARAM_RATIOS + i).getValue());
		if ((ratio != m_displayedRatios[i]) && (m_ratioLeds[i] != nullptr)) {
			m_ratioLeds[i]->setForegroundText(wonkyClockRatios[ratio].display);
		}
	}
}

WonkyClockOutputExpanderWidget::WonkyClockOutputExpanderWidget(WonkyClockOutputExpanderModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "wonky-clock-output-expander") {
	for (int i = 0; i < 8; i++) {
		addParam(createParamCentered<NTKnob35>(Vec(32.f, 41.5f + (40.f * i)), module, WonkyClockOutputExpanderModule::PARAM_RATIOS + i));

		LEDDisplay* ratioLed = new LEDDisplay(nvgRGB(0xFF, 0x50, 0x50), nvgRGB(0x40, 0x40, 0x40), "", 15, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, true);
		ratioLed->box.pos = Vec(60.5f, 31.f + (40.f * i));
		ratioLed->box.size = Vec(36.f, 20.f);
		ratioLed->setForegroundText(wonkyClockRatios[static_cast<int>(i * 2.5) + 1].display);
		addChild(ratioLed);
		if (module) {
			module->m_ratioLeds[i] = ratioLed;
		}

		addOutput(createOutputCentered<NTPort>(Vec(123.f, 41.5f + (40.f * i)), module, WonkyClockOutputExpanderModule::OUT_CLOCKS + i));
	}
}


Model* modelWonkyClockOutputExpander = createModel<WonkyClockOutputExpanderModule, WonkyClockOutputExpanderWidget>("wonky-clock-output-expander");