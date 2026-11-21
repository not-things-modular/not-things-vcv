#include "modules/wonky-clock-cv-expander.hpp"
#include "components/leddisplay.hpp"
#include "components/ntport.hpp"
#include "components/wonkyclock-display.hpp"

WonkyClockCVExpanderModule::WonkyClockCVExpanderModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configInput(IN_WOBBLE_AMOUNT, "Wobble Amount");
	configInput(IN_WOBBLE_RATE, "Wobble Rate");
	configInput(IN_WIGGLE_AMOUNT, "Wiggle Amount");
	configInput(IN_WIGGLE_RATE, "Wiggle Rate");
	configInput(IN_WANDER_AMOUNT, "Wander Amount");
	configInput(IN_WANDER_RATE, "Wander Rate");
}

WonkyClockCVExpanderWidget::WonkyClockCVExpanderWidget(WonkyClockCVExpanderModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "wonky-clock-cv-expander") {
	addInput(createInputCentered<NTPort>(Vec(37.5f, 52.f), module, WonkyClockCVExpanderModule::IN_WOBBLE_AMOUNT));
	addInput(createInputCentered<NTPort>(Vec(37.5f, 97.f), module, WonkyClockCVExpanderModule::IN_WOBBLE_RATE));
	addInput(createInputCentered<NTPort>(Vec(37.5f, 169.5f), module, WonkyClockCVExpanderModule::IN_WIGGLE_AMOUNT));
	addInput(createInputCentered<NTPort>(Vec(37.5f, 214.5f), module, WonkyClockCVExpanderModule::IN_WIGGLE_RATE));
	addInput(createInputCentered<NTPort>(Vec(37.5f, 287.f), module, WonkyClockCVExpanderModule::IN_WANDER_AMOUNT));
	addInput(createInputCentered<NTPort>(Vec(37.5f, 332.f), module, WonkyClockCVExpanderModule::IN_WANDER_RATE));
}


Model* modelWonkyClockCVExpander = createModel<WonkyClockCVExpanderModule, WonkyClockCVExpanderWidget>("wonky-clock-cv-expander");