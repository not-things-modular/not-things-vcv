#include "modules/wonky-clock-cv-expander.hpp"
#include "components/leddisplay.hpp"
#include "components/ntport.hpp"
#include "components/wonkyclock-display.hpp"

#include "core/wonky-core.hpp"

using namespace wonky;

WonkyClockCVExpanderModule::WonkyClockCVExpanderModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configInput(IN_BPM, "BPM");

	configInput(IN_WOBBLE_AMOUNT, "Wobble Amount");
	configInput(IN_WOBBLE_PROBABILITY, "Woble Probability");
	configInput(IN_WAVER_AMOUNT, "Waver Amount");
	configInput(IN_WAVER_PROBABILITY, "Waver Probability");
	configInput(IN_WANDER_AMOUNT, "Wander Amount");
	configInput(IN_WANDER_RATE, "Wander Rate");

	configOutput(OUT_RUN, "Run");
	configOutput(OUT_RESET, "Reset");

	m_displayedBpm = 120;
}

void WonkyClockCVExpanderModule::draw(const widget::Widget::DrawArgs& args) {
	// int bpm = params[PARAM_BPM].getValue();

	// if (bpm != m_displayedBpm) {
	// 	m_displayedBpm = bpm;
	// 	if (m_bpmLed != nullptr) {
	// 		m_bpmLed->setForegroundText(string::f("%d", m_displayedBpm));
	// 	}
	// }

	// lights[LightId::LIGHT_RESET].setBrightnessSmooth(0.f, .01f, 20.f);
}

WonkyClockCVExpanderWidget::WonkyClockCVExpanderWidget(WonkyClockCVExpanderModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "wonky-clock-cv-expander") {
	addOutput(createOutputCentered<NTPort>(Vec(27.5f, 45.f), module, WonkyClockCVExpanderModule::OUT_RUN));
	addOutput(createOutputCentered<NTPort>(Vec(77.5f, 45.f), module, WonkyClockCVExpanderModule::OUT_RESET));

	addInput(createInputCentered<NTPort>(Vec(27.5f, 106.f), module, WonkyClockCVExpanderModule::IN_BPM));
	addInput(createInputCentered<NTPort>(Vec(27.5f, 181.f), module, WonkyClockCVExpanderModule::IN_WOBBLE_AMOUNT));
	addInput(createInputCentered<NTPort>(Vec(77.5f, 181.f), module, WonkyClockCVExpanderModule::IN_WOBBLE_PROBABILITY));
	addInput(createInputCentered<NTPort>(Vec(27.5f, 256.f), module, WonkyClockCVExpanderModule::IN_WAVER_AMOUNT));
	addInput(createInputCentered<NTPort>(Vec(77.5f, 256.f), module, WonkyClockCVExpanderModule::IN_WAVER_PROBABILITY));
	addInput(createInputCentered<NTPort>(Vec(27.5f, 332.f), module, WonkyClockCVExpanderModule::IN_WANDER_AMOUNT));
	addInput(createInputCentered<NTPort>(Vec(77.5f, 332.f), module, WonkyClockCVExpanderModule::IN_WANDER_RATE));

	LEDDisplay* bpmLed = new LEDDisplay(nvgRGB(0xFF, 0x50, 0x50), nvgRGB(0x40, 0x40, 0x40), "888", 14, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, true);
	bpmLed->box.pos = Vec(59.5f, 96.f);
	bpmLed->box.size = Vec(35.f, 20.f);
	bpmLed->setForegroundText("120");
	addChild(bpmLed);
	if (module) {
		module->m_bpmLed = bpmLed;
	}
}


Model* modelWonkyClockCVExpander = createModel<WonkyClockCVExpanderModule, WonkyClockCVExpanderWidget>("wonky-clock-cv-expander");