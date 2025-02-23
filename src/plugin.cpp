#include <rack.hpp>
using namespace rack;

extern Model* modelSolim;
extern Model* modelSolimInput;
extern Model* modelSolimInputOctaver;
extern Model* modelSolimOutput;
extern Model* modelSolimOutputOctaver;
extern Model* modelSolimRandom;

extern Model* modelPipoOutput;
extern Model* modelPipoInput;

extern Model* modelPolySameDiff;

Plugin* pluginInstance;

void init(Plugin* p) {
	pluginInstance = p;

	p->addModel(modelSolim);
	p->addModel(modelSolimInput);
	p->addModel(modelSolimInputOctaver);
	p->addModel(modelSolimOutput);
	p->addModel(modelSolimOutputOctaver);
	p->addModel(modelSolimRandom);
	p->addModel(modelPipoInput);
	p->addModel(modelPipoOutput);
	p->addModel(modelPolySameDiff);
}
