#include "modules/wonky-clock.hpp"
#include "components/leddisplay.hpp"
#include "components/lights.hpp"
#include "components/ntknob.hpp"
#include "components/ntport.hpp"

WonkyClockModule::WonkyClockModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	ParamQuantity* pq = configParam(PARAM_BPM, 20.f, 400.f, 120.f, "Clock Speed (Beats per Minute)");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;

	configInput(IN_BPM, "Bpm");
	configInput(IN_RUN, "Run");
	configInput(IN_RESET, "Reset");

	configButton(PARAM_RUN, "Run");
	configButton(PARAM_RESET, "Reset");

	configParam(PARAM_WOBBLE_AMOUNT, 0.f, 40.f, 9.f, "Wobble Amount");
	configParam(PARAM_WOBBLE_PROBABILITY, 0.f, 100.f, 50.f, "Wobble Probability");
	configParam(PARAM_WAVER_AMOUNT, 0.f, 40.f, 5.f, "Waver Amount");
	configParam(PARAM_WAVER_PROBABILITY, 0.f, 100.f, 25.f, "Waver Probability");
	configParam(PARAM_WANDER_AMOUNT, 0.f, 40.f, 5.f, "Wander Amount");
	configParam(PARAM_WANDER_RATE, 0.f, 100.f, 25.f, "Wander Rate");

	configSwitch(PARAM_LINK, 0.f, 1.f, 1.f, "Link", { "Unlinked", "Linked" });

	configParam(PARAM_WEIGHT, 0.f, 100.f, 10.f, "Distribution Weight");

	configOutput(OUT_CLOCK, "Clock");
	configOutput(OUT_RUN, "Run");
	configOutput(OUT_RESET, "Reset");

	m_bpm = 120;
}

void WonkyClockModule::draw(const widget::Widget::DrawArgs& args) {
	// // Look to the right to see if there is a Pipo Output module there (with possibility other Pipo Input modules in between)
	// // If not, we need to turn off our connected LED
	// Expander* expander = &getRightExpander();
	// while ((expander->module != nullptr) && (expander->module->getModel() == modelPipoInput)) {
	// 	expander = &expander->module->getRightExpander();
	// }
	// if ((expander->module != nullptr) && (expander->module->getModel() == modelPipoOutput)) {
	// 	lights[LightId::LIGHT_CONNECTED].setBrightness(1.f);
	// 	lights[LightId::LIGHT_NOT_CONNECTED].setBrightness(0.f);
	// } else {
	// 	lights[LightId::LIGHT_CONNECTED].setBrightness(0.f);
	// 	lights[LightId::LIGHT_NOT_CONNECTED].setBrightness(1.f);
	// }

	// for (int i = 0; i < 8; i++) {
	// 	m_ledDisplays[i]->setForegroundText(string::f("%d", std::max(inputs[InputId::IN_INPUTS + i].getChannels(), 1)));
	// }

	int bpm = params[PARAM_BPM].getValue();;

	if (bpm != m_bpm) {
		m_bpm = bpm;
		if (m_bmpLed != nullptr) {
			m_bmpLed->setForegroundText(string::f("%d", m_bpm));
		}
	}

}

WonkyClockWidget::WonkyClockWidget(WonkyClockModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "wonky-clock") {
	NTKnob50* bpmKnob = createParamCentered<NTKnob50>(Vec(42.f, 66.f), module, WonkyClockModule::PARAM_BPM);
	bpmKnob->setAngles(-.85 * M_PI, .85 * M_PI);
	addParam(bpmKnob);

	addParam(createParamCentered<NTKnob35>(Vec(113.5f, 166.f), module, WonkyClockModule::PARAM_WOBBLE_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(120.f, 210.f), module, WonkyClockModule::PARAM_WOBBLE_PROBABILITY));

	addParam(createParamCentered<NTKnob35>(Vec(113.5f - 80.f, 166.f), module, WonkyClockModule::PARAM_WAVER_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(120.f - 80.f, 210.f), module, WonkyClockModule::PARAM_WAVER_PROBABILITY));

	addParam(createParamCentered<NTKnob35>(Vec(113.5f, 166.f - 112.4f), module, WonkyClockModule::PARAM_WANDER_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(120.f, 210.f - 112.4f), module, WonkyClockModule::PARAM_WANDER_RATE));

	addParam(createParamCentered<CKSSThreeHorizontal>(Vec(80.5f, 252.f), module, WonkyClockModule::PARAM_LINK));

	addParam(createParamCentered<Trimpot>(Vec(118.f, 293.5f), module, WonkyClockModule::PARAM_WEIGHT));

	addInput(createInputCentered<NTPort>(Vec(28.f, 333.f), module, WonkyClockModule::IN_RUN));
	addParam(createLightParamCentered<LEDLightBezel<RedLight>>(Vec(28.f, 285.f), module, WonkyClockModule::PARAM_RUN, WonkyClockModule::LIGHT_RUN));
	addInput(createInputCentered<NTPort>(Vec(70.f, 333.f), module, WonkyClockModule::IN_RESET));
	addParam(createLightParamCentered<LEDLightBezel<DimmedLight<RedLight>>>(Vec(70.f, 285.f), module, WonkyClockModule::PARAM_RESET, WonkyClockModule::LIGHT_RESET));

	addOutput(createOutputCentered<NTPort>(Vec(136.f, 333.f), module, WonkyClockModule::OUT_CLOCK));

	LEDDisplay* bmpLed = new LEDDisplay(nvgRGB(0xFF, 0x50, 0x50), nvgRGB(0x40, 0x40, 0x40), "888", 20, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, true);
	bmpLed->box.pos = Vec(17.f, 93.f);
	bmpLed->box.size = Vec(50.f, 25.f);
	bmpLed->setForegroundText("120");
	addChild(bmpLed);
	if (module) {
		module->m_bmpLed = bmpLed;
	}

	// for (int i = 0; i < 8; i++) {
	// 	// addInput(createInputCentered<NTPort>(Vec(xIn, y), module, PipoInputModule::IN_INPUTS + i));
	// 	// y += yDelta;

	// 	LEDDisplay* pDisplay = new LEDDisplay(nvgRGB(0xFF, 0x50, 0x50), nvgRGB(0x40, 0x40, 0x40), "18", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, true);
	// 	pDisplay->box.pos = Vec(3.f, 52.f + (yDelta * i));
	// 	pDisplay->box.size = Vec(13.f, 12.5f);
	// 	pDisplay->setForegroundText("1");
	// 	addChild(pDisplay);

	// 	if (module) {
	// 		module->m_ledDisplays[i] = pDisplay;
	// 	}
	// }

	// addChild(createLightCentered<TinyLight<DimmedLight<GreenRedLight>>>(Vec(40.f, 20.f), module, PipoInputModule::LIGHT_CONNECTED));
}


Model* modelWonkyClock = createModel<WonkyClockModule, WonkyClockWidget>("wonky-clock");