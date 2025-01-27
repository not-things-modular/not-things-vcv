#pragma once
#include "not-things.hpp"

extern Model* modelSolim;
extern Model* modelSolimRandom;
extern Model* modelSolimInput;
extern Model* modelSolimInputOctaver;
extern Model* modelSolimOutput;
extern Model* modelSolimOutputOctaver;

enum ExpanderSide {
    LEFT=0,
    RIGHT=1
};

struct ExpanderData {
    ExpanderData(Module& expanderModule, Model* expanderModel, ExpanderSide side);

    Module& expanderModule;
    Model* expanderModel;
    ExpanderSide side;
};

void registerExpanderModule(Module& mainModule, ExpanderData expanderData);
void registerExpanderModules(Module& mainModule, std::vector<ExpanderData> expanders);

void setInputVoltages(Input& input, std::vector<float> voltages);