#include "modules/poly-same-diff.hpp"
#include "components/ntport.hpp"
#include <bitset>


// We can re-use the Solim value limiting function for Note mode comparison.
extern float limitValueIf(float value, float lowerLimit, float upperLimit);

float normalizeNoteValue(float note);
bool isMatch(float value1, float value2, float delta, bool noteMode);
int assignOutput(Output& output, float* values, int valueCount, std::bitset<16>& bits, bool bitFlag, float delta, bool noteMode, bool outputDuplicates, int offset = 0);


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

json_t *PolySameDiffModule::dataToJson() {
	json_t *rootJ = NTModule::dataToJson();
	json_object_set_new(rootJ, "ntPolySameDiffOutputDuplicates", json_boolean(m_outputDuplicates));
	return rootJ;
}

void PolySameDiffModule::dataFromJson(json_t *rootJ) {
	NTModule::dataFromJson(rootJ);

	json_t *ntPolySameDiffOutputDuplicates = json_object_get(rootJ, "ntPolySameDiffOutputDuplicates");
	if (ntPolySameDiffOutputDuplicates) {
		if (json_is_boolean(ntPolySameDiffOutputDuplicates)) {
			setOutputDuplicates(json_boolean_value(ntPolySameDiffOutputDuplicates));
		} else {
			setOutputDuplicates(false);
		}
	}
}

void PolySameDiffModule::process(const ProcessArgs& args) {
	// Variable setup and external parameters
	std::bitset<16> aBits;
	std::bitset<16> bBits;
	float* as = inputs[IN_A].getVoltages();
	float* bs = inputs[IN_B].getVoltages();
	int aChannels = inputs[IN_A].getChannels();
	int bChannels = inputs[IN_B].getChannels();
	bool noteMode = params[PARAM_MODE].getValue() == 1.f;
	float delta = std::max(params[PARAM_DELTA].getValue(), 0.00001f);

	// If requested, apply note normalization on the input voltages
	float* nas;
	float* nbs;
	if (noteMode) {
		nas = m_floatBuffA;
		nbs = m_floatBuffB;
		for (int i = 0; i < aChannels; i++) {
			nas[i] = normalizeNoteValue(as[i]);
		}
		for (int i = 0; i < bChannels; i++) {
			nbs[i] = normalizeNoteValue(bs[i]);
		}
	} else {
		nas = as;
		nbs = bs;
	}

	// Determine which voltages in the inputs also appear in the other input
	for (int a = 0; a < aChannels; a++) {
		for (int b = 0; b < bChannels; b++) {
			if (isMatch(nas[a], nbs[b], delta, noteMode)) {
				aBits.set(a);
				bBits.set(b);
			}
		}
	}

	assignOutput(outputs[OUT_A], as, aChannels, aBits, false, delta, noteMode, m_outputDuplicates);
	assignOutput(outputs[OUT_B], bs, bChannels, bBits, false, delta, noteMode, m_outputDuplicates);
	int count = assignOutput(outputs[OUT_AB], as, aChannels, aBits, true, delta, noteMode, m_outputDuplicates);
	if (m_outputDuplicates) {
		// If duplicates have to be added to the output, also add the B channels that had a match to the output.
		assignOutput(outputs[OUT_AB], bs, bChannels, bBits, true, delta, noteMode, m_outputDuplicates, count);
	}
}

bool PolySameDiffModule::getOutputDuplicates() {
	return m_outputDuplicates;
}

void PolySameDiffModule::setOutputDuplicates(bool outputDuplicates) {
	m_outputDuplicates = outputDuplicates;
}


PolySameDiffWidget::PolySameDiffWidget(PolySameDiffModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "poly-same-diff") {
	float x = 22.5;
	float y = 41.5;
	float yDelta = 40;

	addInput(createInputCentered<NTPort>(Vec(x, y), module, PolySameDiffModule::IN_A));
	addInput(createInputCentered<NTPort>(Vec(x, y + yDelta), module, PolySameDiffModule::IN_B));

	addParam(createParamCentered<RoundSmallBlackKnob>(Vec(x, 140.f), module, PolySameDiffModule::PARAM_DELTA));
	addParam(createParamCentered<CKSS>(Vec(x, 195.f), module, PolySameDiffModule::PARAM_MODE));

	addOutput(createOutputCentered<NTPort>(Vec(x, y + (yDelta * 5)), module, PolySameDiffModule::OUT_A));
	addOutput(createOutputCentered<NTPort>(Vec(x, y + (yDelta * 6)), module, PolySameDiffModule::OUT_AB));
	addOutput(createOutputCentered<NTPort>(Vec(x, y + (yDelta * 7)), module, PolySameDiffModule::OUT_B));
}

void PolySameDiffWidget::appendContextMenu(Menu* menu) {
	NTModuleWidget::appendContextMenu(menu);

	bool outputDuplicates = getModule() ? dynamic_cast<PolySameDiffModule *>(getModule())->getOutputDuplicates() : false;
	menu->addChild(createCheckMenuItem("Output duplicate voltages", "", [outputDuplicates]() { return outputDuplicates; }, [this]() { switchOutputDuplicates(); }));
}

void PolySameDiffWidget::switchOutputDuplicates() {
	PolySameDiffModule* polySameDiffModule = dynamic_cast<PolySameDiffModule *>(getModule());
	if (polySameDiffModule != nullptr) {
		polySameDiffModule->setOutputDuplicates(!polySameDiffModule->getOutputDuplicates());
	}
}



float normalizeNoteValue(float note) {
	float normalized = limitValueIf(note, 0.f, 1.f);
	// There is a small chance that the note value was just on/under/above the 0.f or 1.f boundary, so make sure that we get a value below 1.f and above or equal to 0.f
	if (normalized < 0.f) {
		normalized = normalized + 1;
	} else if (normalized >= 1.f) {
		normalized = normalized - 1;
	}
	return normalized;
}

bool isMatch(float value1, float value2, float delta, bool noteMode) {
	if (std::fabs(value1 - value2) <= delta) {
		return true;
	} else if (noteMode) {
		// If one of the values is near 0.f, and the other is near 1.f, they might still be within the delta range when we're working on noteMode.
		// Add one voltage to the lowest value, and re-check to see if they are within range now.
		bool aLower = value1 < value2;
		float lower = (aLower ? value1 : value2) + 1.f;
		float upper = aLower ? value2 : value1;
		if (std::fabs(lower - upper) <= delta) {
			return true;
		}
	}

	return false;
}

bool containsMatch(float* targets, int targetCount, float input, float delta, bool noteMode) {
	float normalizedInput = noteMode ? normalizeNoteValue(input) : input;
	for (int i = 0; i < targetCount; i++) {
		float normalizedOutput = noteMode ? normalizeNoteValue(targets[i]) : targets[i];
		if (isMatch(normalizedOutput, normalizedInput, delta, noteMode)) {
			return true;
		}
	}
	return false;
}

int assignOutput(Output& output, float* values, int valueCount, std::bitset<16>& bits, bool bitFlag, float delta, bool noteMode, bool outputDuplicates, int offset) {
	int outputCount = offset;
	float* outputs = output.getVoltages();

	for (int i = 0; i < valueCount && outputCount < 16; i++) {
		if (bits[i] == bitFlag) {
			if ((outputDuplicates) || (!containsMatch(outputs, outputCount, values[i], delta, noteMode))) {
				outputs[outputCount] = values[i];
				outputCount++;
			}
		}
	}

	// Setting a connected port to 0 channels or going back from 0 to more channels can not be done through the setChannels method, so do it directly instead.
	output.channels = outputCount;
	
	return outputCount;
}


Model* modelPolySameDiff = createModel<PolySameDiffModule, PolySameDiffWidget>("poly-same-diff");
