#include "modules/wonky-clock.hpp"
#include "components/leddisplay.hpp"
#include "components/ntknob.hpp"
#include "components/ntport.hpp"

WonkyClockModule::WonkyClockModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	ParamQuantity* pq = configParam(PARAM_BPM, 20.f, 400.f, 120.f, "Clock Speed (Beats per Minute)");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;

	configParam(PARAM_WOBBLE_AMOUNT, 0.f, 40.f, 9.f, "Wobble Amount");
	configParam(PARAM_WOBBLE_PROBABILITY, 0.f, 100.f, 50.f, "Wobble Probability");
	configParam(PARAM_WANDER_AMOUNT, 0.f, 40.f, 5.f, "Wander Amount");
	configParam(PARAM_WANDER_PROBABILITY, 0.f, 100.f, 25.f, "Wander Probability");

	configSwitch(PARAM_LINK, 0.f, 1.f, 1.f, "Link", { "Unlinked", "Linked" });

	configParam(PARAM_WEIGHT, 0.f, 100.f, 10.f, "Distribution Weight");

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
	NTKnob50* bpmKnob = createParamCentered<NTKnob50>(Vec(30.f, 40.f), module, WonkyClockModule::PARAM_BPM);
	bpmKnob->setAngles(-.85 * M_PI, .85 * M_PI);
	addParam(bpmKnob);

	addParam(createParamCentered<NTKnob35>(Vec(30.f, 90.f), module, WonkyClockModule::PARAM_WOBBLE_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(70.f, 90.f), module, WonkyClockModule::PARAM_WOBBLE_PROBABILITY));

	addParam(createParamCentered<NTKnob35>(Vec(30.f, 140.f), module, WonkyClockModule::PARAM_WANDER_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(70.f, 140.f), module, WonkyClockModule::PARAM_WANDER_PROBABILITY));

	addParam(createParamCentered<CKSS>(Vec(30.f, 200.f), module, WonkyClockModule::PARAM_LINK));

	addParam(createParamCentered<Trimpot>(Vec(30.f, 240.f), module, WonkyClockModule::PARAM_WEIGHT));

	addInput(createInputCentered<NTPort>(Vec(30, 280), module, WonkyClockModule::IN_BPM));
	addInput(createInputCentered<NTPort>(Vec(30, 310), module, WonkyClockModule::IN_WOBBLE_AMOUNT));
	addInput(createInputCentered<NTPort>(Vec(60, 310), module, WonkyClockModule::IN_WOBBLE_PROBABILITY));
	addInput(createInputCentered<NTPort>(Vec(30, 340), module, WonkyClockModule::IN_WANDER_AMOUNT));
	addInput(createInputCentered<NTPort>(Vec(60, 340), module, WonkyClockModule::IN_WANDER_PROBABILITY));

	LEDDisplay* bmpLed = new LEDDisplay(nvgRGB(0xFF, 0x50, 0x50), nvgRGB(0x40, 0x40, 0x40), "888", 25, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, true);
	bmpLed->box.pos = Vec(64.f, 31.f);
	bmpLed->box.size = Vec(65.f, 32.f);
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