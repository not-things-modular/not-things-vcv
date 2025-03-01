#include "vcv-registration.hpp"

ExpanderData::ExpanderData(Module& expanderModule, Model* expanderModel, ExpanderSide side) : expanderModule(expanderModule), expanderModel(expanderModel), side(side) {
}

void registerExpanderModule(Module& mainModule, ExpanderData expanderData) {
	if (expanderData.side == ExpanderSide::LEFT) {
		mainModule.leftExpander.module = &expanderData.expanderModule;
	} else {
		mainModule.rightExpander.module = &expanderData.expanderModule;
	}

	expanderData.expanderModule.model = expanderData.expanderModel;
}

void registerExpanderModules(Module& mainModule, std::vector<ExpanderData> expanders) {
	if (expanders.size() > 0) {
		registerExpanderModule(mainModule, expanders[0]);
	}
	for (size_t i = 1; i < expanders.size(); i++) {
		registerExpanderModule(expanders[i - 1].expanderModule, expanders[i]);
	}
}

void setPortVoltages(Port& port, std::vector<float> voltages) {
	port.channels = voltages.size();
	for (size_t i = 0; i < voltages.size(); i++) {
		port.setVoltage(voltages[i], i);
	}
}
