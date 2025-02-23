#include "modules/poly-same-diff.hpp"
#include "components/ntport.hpp"
#include <bitset>


// We can re-use the Solim value limiting function for Note mode comparison.
extern float limitValueIf(float value, float lowerLimit, float upperLimit);

float normalizeNoteValue(float note);
void assignOutput(Output& output, float* values, int valueCount, std::bitset<16>& bits, bool bitFlag, float delta, bool noteMode, bool outputDuplicates);


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
			if ((nas[a] > nbs[b] - delta) && (nas[a] < nbs[b] + delta)) {
				aBits.set(a);
				bBits.set(b);
			}
		}
	}

	// Update the outputs (can't detect if they are connected, because we may have set them to 0 channels - and thus disconnected - ourselves)
	assignOutput(outputs[OUT_A], as, aChannels, aBits, false, delta, noteMode, m_outputDuplicates);
	assignOutput(outputs[OUT_AB], as, aChannels, aBits, true, delta, noteMode, m_outputDuplicates);
	assignOutput(outputs[OUT_B], bs, bChannels, bBits, false, delta, noteMode, m_outputDuplicates);
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

bool containsMatch(float* targets, int targetCount, float input, float delta, bool noteMode) {
	float normalizedInput = noteMode ? normalizeNoteValue(input) : input;
	for (int i = 0; i < targetCount; i++) {
		float normalizedOutput = noteMode ? normalizeNoteValue(targets[i]) : targets[i];
		if ((normalizedOutput > normalizedInput - delta) && (normalizedOutput < normalizedInput + delta)) {
			return true;
		}
	}
	return false;
}

void assignOutput(Output& output, float* values, int valueCount, std::bitset<16>& bits, bool bitFlag, float delta, bool noteMode, bool outputDuplicates) {
	int outputCount = 0;
	float* outputs = output.getVoltages();

	for (int i = 0; i < valueCount; i++) {
		if (bits[i] == bitFlag) {
			if ((outputDuplicates) || (!containsMatch(outputs, outputCount, values[i], delta, noteMode))) {
				outputs[outputCount] = values[i];
				outputCount++;
			}
		}
	}

	// Assign the channels directly iso through setChannels because we may end up with 0 total channels
	output.channels = outputCount;
}


Model* modelPolySameDiff = createModel<PolySameDiffModule, PolySameDiffWidget>("poly-same-diff");