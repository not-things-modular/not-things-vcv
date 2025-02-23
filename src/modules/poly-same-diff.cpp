#include "modules/poly-same-diff.hpp"
#include "components/ntport.hpp"


PolySameDiffModule::PolySameDiffModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configInput(IN_A, "A");
	configInput(IN_B, "B");

	configParam(PARAM_DELTA, 0.f, 1.f, 0.f, "Delta tolerance", "V");
	configSwitch(PARAM_MODE, 0.f, 1.f, 0.f, "Mode", { "Voltage", "Note" });

	configOutput(OUT_A, "A Only");
	configOutput(OUT_AB, "A and B");
	configOutput(OUT_B, "B Only");


}

void PolySameDiffModule::process(const ProcessArgs& args) {

}


PolySameDiffWidget::PolySameDiffWidget(PolySameDiffModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "poly-same-diff") {
	float x = 22.5;
	float y = 41.5;
	float yDelta = 40;

	addInput(createInputCentered<NTPort>(Vec(x, y), module, PolySameDiffModule::IN_A));
	addInput(createInputCentered<NTPort>(Vec(x, y + yDelta), module, PolySameDiffModule::IN_B));

	addParam(createParamCentered<RoundSmallBlackKnob>(Vec(x, 140.f), module, PolySameDiffModule::PARAM_DELTA));
	addParam(createParamCentered<CKSS>(Vec(x-4.5f, 195.f), module, PolySameDiffModule::PARAM_MODE));

	addOutput(createOutputCentered<NTPort>(Vec(x, y + (yDelta * 5)), module, PolySameDiffModule::OUT_A));
	addOutput(createOutputCentered<NTPort>(Vec(x, y + (yDelta * 6)), module, PolySameDiffModule::OUT_AB));
	addOutput(createOutputCentered<NTPort>(Vec(x, y + (yDelta * 7)), module, PolySameDiffModule::OUT_B));
}


Model* modelPolySameDiff = createModel<PolySameDiffModule, PolySameDiffWidget>("poly-same-diff");