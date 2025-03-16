#include "modules/pipo-input.hpp"
#include "components/ntport.hpp"
#include "components/leddisplay.hpp"

extern Model* modelPipoOutput;
extern Model* modelPipoInput;

PipoInputModule::PipoInputModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configInput(IN_INPUTS + i, string::f("Input %d", i + 1));
	}
}

void PipoInputModule::draw(const widget::Widget::DrawArgs& args) {
	// Look to the right to see if there is a Pipo Output module there (with possibility other Pipo Input modules in between)
	// If not, we need to turn off our connected LED
	Expander* expander = &getRightExpander();
	while ((expander->module != nullptr) && (expander->module->getModel() == modelPipoInput)) {
		expander = &expander->module->getRightExpander();
	}
	if ((expander->module != nullptr) && (expander->module->getModel() == modelPipoOutput)) {
		lights[LightId::LIGHT_CONNECTED].setBrightness(1.f);
		lights[LightId::LIGHT_NOT_CONNECTED].setBrightness(0.f);
	} else {
		lights[LightId::LIGHT_CONNECTED].setBrightness(0.f);
		lights[LightId::LIGHT_NOT_CONNECTED].setBrightness(1.f);
	}

	for (int i = 0; i < 8; i++) {
		m_ledDisplays[i]->setForegroundText(string::f("%d", std::max(inputs[InputId::IN_INPUTS + i].getChannels(), 1)));
	}
}

PipoInputWidget::PipoInputWidget(PipoInputModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "pipo-input") {
	float xIn = 25;
	float y = 41.5;
	float yDelta = 40;
	for (int i = 0; i < 8; i++) {
		addInput(createInputCentered<NTPort>(Vec(xIn, y), module, PipoInputModule::IN_INPUTS + i));
		y += yDelta;

		LEDDisplay* pDisplay = new LEDDisplay(nvgRGB(0xFF, 0x50, 0x50), nvgRGB(0x40, 0x40, 0x40), "18", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, true);
		pDisplay->box.pos = Vec(3.f, 52.f + (yDelta * i));
		pDisplay->box.size = Vec(13.f, 12.5f);
		pDisplay->setForegroundText("1");
		addChild(pDisplay);

		if (module) {
			module->m_ledDisplays[i] = pDisplay;
		}
	}

	addChild(createLightCentered<TinyLight<GreenRedLight>>(Vec(40.f, 20.f), module, PipoInputModule::LIGHT_CONNECTED));
}


Model* modelPipoInput = createModel<PipoInputModule, PipoInputWidget>("pipo-input");