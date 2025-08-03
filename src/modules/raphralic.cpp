#include "modules/raphralic.hpp"


RaphralicModule::RaphralicModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
}

void RaphralicModule::process(const ProcessArgs& args) {

}

RaphralicWidget::RaphralicWidget(RaphralicModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "raphralic") {

}



Model* modelRaphralic = createModel<RaphralicModule, RaphralicWidget>("raphralic");