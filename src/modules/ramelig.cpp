#include "modules/ramelig.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"


extern Model* modelRamelig;

enum RameligStepActions {
	RANDOM_JUMP = 0,
	RANDOM_MOVE = 1,
	AROUND_CURRENT = 2,

	UP_TWO = 3,
	UP_ONE = 4,
	DOWN_ONE = 5,
	DOWN_TWO = 6,
	REMAIN = 7
};

#define TO_MOVE_INDEX(stepAction) stepAction - UP_TWO

struct RameligData {
	std::array<std::vector<int>, 12> scales;
	unsigned int currentScale;

	int currentOctave;
	int currentScaleIndex;
	bool lastWasRemain;

	float randomJumpChance; // The chance that the melody line will jump for one note and then return to the original position
	float randomMoveChance; // The chance that the melody line will move to another position and stay there

	float moveUpChance; // How likely it is that a move will go up
	float remainChance; // How likely it is that the melody won't move
	float moveDownChance; // How likely it is that a move will go down

	float moveTwoChance; // How likely it is that a move will be two positions
	float remainRepeatFactor; // If the previous step was a remain, how much of the remainChance should be used to for this step

	std::array<float, 3> stepDistribution; // The distribution of possible step actions: random jump, random move, or move around current position
	std::array<float, 5> moveDistribution; // The distribution of possible movements around the curren position: up two, up one, remain, down one and down two

	std::array<int, 8> results;

	void calculateDistributions() {
		stepDistribution[RANDOM_JUMP] = randomJumpChance;
		stepDistribution[RANDOM_MOVE] = stepDistribution[RANDOM_JUMP] + randomMoveChance;
		stepDistribution[AROUND_CURRENT] = stepDistribution[RANDOM_MOVE] + moveUpChance + remainChance + moveDownChance;

		moveDistribution[TO_MOVE_INDEX(UP_TWO)] = moveUpChance * moveTwoChance;
		moveDistribution[TO_MOVE_INDEX(UP_ONE)] = moveUpChance;
		moveDistribution[TO_MOVE_INDEX(DOWN_ONE)] = moveUpChance + (moveDownChance * (1 - moveTwoChance));
		moveDistribution[TO_MOVE_INDEX(DOWN_TWO)] = moveUpChance + moveDownChance;
		moveDistribution[TO_MOVE_INDEX(REMAIN)] = moveDistribution[TO_MOVE_INDEX(DOWN_TWO)] + remainChance;
	}

	RameligStepActions determineChance(std::minstd_rand &generator) {
		float upperLimit = stepDistribution[AROUND_CURRENT];
		if (lastWasRemain) {
			upperLimit -= remainChance * (1 - remainRepeatFactor);
		}
		float chance = std::uniform_real_distribution<float>(0.f, upperLimit)(generator);

		RameligStepActions result = AROUND_CURRENT;
		for (unsigned int i = 0; i < 3; i++) {
			if (stepDistribution[i] > chance) {
				result = static_cast<RameligStepActions>(i);
				break;
			}
		}

		// TODO: Bias upwards or downwards when moving near the edges of the allowed values
		if (result == AROUND_CURRENT) {
			chance -= stepDistribution[RANDOM_MOVE];
			result = DOWN_TWO;
			for (unsigned int i = 0; i < 5; i++) {
				if (moveDistribution[i] > chance) {
					result = static_cast<RameligStepActions>(i + 3);
					break;
				}
			}
		}

		results[result] = results[result] + 1;
		return result;
	}
};

std::pair<float, int> quantize(float value, std::vector<int>& indices, float lowerLimit, float upperLimit, std::vector<float>& notes) {
	float oct;
	float fract;
	if (value < 0) {
		oct = std::floor(value); // The amount we'll have to subtract from the end result, since we'll make the decimal part positive to easier compare with the quantization notes list
		fract = value - oct; // Results in the fractal part moved between 0 and 1
	} else {
		fract = std::modf(value, &oct);
	}

	// If the following loop doesn't find a result, it means we'll have to quantize downwards
	int index = indices.size() - 1;
	oct--;

	// Look for the first note in the scale that is above the fractal part of the input value
	for (int i = 0; i < (int) indices.size(); i++) {
		if (fract <= notes[indices[i]]) {
			// We found a note in the scale to quantize to, so take that note and undo the octave down we did before.
			index = i;
			oct++;
			break;
		}
	}

	// Make sure the quantized result remains within the limits
	while (oct + notes[index] < lowerLimit) {
		index++;
		if (index >= indices.size()) {
			oct++;
			index = 0;
		}
	}

	while (oct + notes[index] > upperLimit) {
		index--;
		if (index < 0) {
			oct--;
			index = indices.size() - 1;
		}
	}

	return std::make_pair(oct, index);
}


RameligModule::RameligModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	m_distribution = std::uniform_real_distribution<float>(0.5f, 3.f);

	configInput(IN_GATE, "Clock");
	configInput(IN_LOWER_LIMIT, "Lower limit CV");
	configInput(IN_UPPER_LIMIT, "Upper limit CV");
	configInput(IN_CHANCE_RANDOM_JUMP, "Random jump CV");
	configInput(IN_CHANCE_MOVE_UP, "Move up chance CV");
	configInput(IN_CHANCE_REMAIN, "Remain chance CV");
	configInput(IN_CHANCE_MOVE_DOWN, "Move down chance CV");
	configInput(IN_SCALE, "Scale CV");

	configOutput(OUT_CV, "CV");
	configOutput(OUT_GATE, "Gate");
	configOutput(OUT_RANDOM, "Random triggered");

	configParam(PARAM_LOWER_LIMIT, -10.f, 10.f, -1.f, "Lower limit", " V");
	configParam(PARAM_UPPER_LIMIT, -10.f, 10.f, 1.f, "Upper limit", " V");

	configParam(PARAM_CHANCE_RANDOM_JUMP, 0.f, 10.f, 5.f, "Random jump chance");
	configButton(PARAM_TRIG_RANDOM_JUMP, "Random jump trigger");
	configParam(PARAM_FACTOR_JUMP_REMAIN, 0.f, 10.f, 7.f, "Random jump: remain to return factor");

	configParam(PARAM_CHANCE_MOVE_UP, 0.f, 10.f, 9.f, "Move up chance");
	configParam(PARAM_CHANCE_REMAIN, 0.f, 10.f, 2.f, "Remain chance");
	configParam(PARAM_CHANCE_MOVE_DOWN, 0.f, 10.f, 9.f, "Move down chance");
	configParam(PARAM_FACTOR_MOVE_TWO, 0.f, 10.f, 7.f, "Move by one or two steps factor");
	configParam(PARAM_FACTOR_REMAIN_REPEAT, 0.f, 10.f, 7.f, "Remain repeat factor");

	configParam(PARAM_SCALE, 0.f, 10.f, 0.f, "Scale");
	configParam(PARAM_SCALE_NOTES + 0, 0.f, 10.f, 0.f, "Scale");

	for (int i = 0; i < 12; i++) {
		m_notes.push_back((float) i / 12);
	}

	m_data = new RameligData();
	m_data->scales[0] = { 0, 2, 4, /*5,*/ 7, 9/*, 11*/ };
	m_data->currentScale = 0;

	m_data->currentOctave = 0;
	m_data->currentScaleIndex = 0;
	m_data->lastWasRemain = false;

	m_data->randomJumpChance = 0.35f;
	m_data->randomMoveChance = 0.3f;
	m_data->moveUpChance = 0.9f;
	m_data->remainChance = 0.2f;
	m_data->moveDownChance = 0.9f;

	m_data->moveTwoChance = 0.3f;
	m_data->remainRepeatFactor = 0.3f;

	m_data->results = { 0, 0, 0, 0, 0, 0, 0, 0 };

	m_data->calculateDistributions();
}

RameligModule::~RameligModule() {
	delete m_data;
}

void RameligModule::process(const ProcessArgs& args) {
	if (m_trigger.process(inputs[IN_GATE].getVoltage(0), 0.f, 1.f)) {
		float upperLimit = inputs[IN_UPPER_LIMIT].getVoltage();
		float lowerLimit = inputs[IN_LOWER_LIMIT].getVoltage();

		float value = std::uniform_real_distribution<float>(lowerLimit, upperLimit)(m_generator);
		RameligStepActions action = m_data->determineChance(m_generator);
		outputs[OUT_GATE].setVoltage(action);

		if ((action == RANDOM_JUMP) || (action == RANDOM_MOVE)) {
			std::pair<float, int> quantized = quantize(value, m_data->scales[m_data->currentScale], lowerLimit, upperLimit, m_notes);
			outputs[OUT_CV].setVoltage(quantized.first + m_notes[m_data->scales[m_data->currentScale][quantized.second]]);
			quantized = quantize(value, m_data->scales[m_data->currentScale], lowerLimit, upperLimit, m_notes);
			if (action == RANDOM_MOVE) {
				m_data->currentOctave = (int) quantized.first;
				m_data->currentScaleIndex = quantized.second;
			}
		} else if (action != REMAIN) {
			int change;
			if (action == UP_TWO) {
				change = 2;
			} else if (action == UP_ONE) {
				change = 1;
			} else if (action == DOWN_ONE) {
				change = -1;
			} else if (action == DOWN_TWO) {
				change = -2;
			}

			float result;
			int oct = m_data->currentOctave;
			int index = m_data->currentScaleIndex + change;
			if (index >= (int) m_data->scales[m_data->currentScale].size()) {
				oct++;
				index = 0;
			}
			if (index < 0) {
				oct--;
				index = m_data->scales[m_data->currentScale].size() - 1;
			}
			result = oct + m_notes[m_data->scales[m_data->currentScale][index]];

			if ((result > upperLimit) || (result < lowerLimit)) {
				oct = m_data->currentOctave;
				index = m_data->currentScaleIndex - change;
				if (index >= (int) m_data->scales[m_data->currentScale].size()) {
					oct++;
					index = 0;
				}
				if (index < 0) {
					oct--;
					index = m_data->scales[m_data->currentScale].size() - 1;
				}
			}

			result = oct + m_notes[m_data->scales[m_data->currentScale][index]];
			m_data->currentOctave = oct;
			m_data->currentScaleIndex = index;

			outputs[OUT_CV].setVoltage(result);
		}

		m_data->lastWasRemain = (action == REMAIN);
	}
}

RameligWidget::RameligWidget(RameligModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "ramelig") {
	addParam(createParamCentered<Rogan1PWhite>(Vec(20, 40), module, RameligModule::PARAM_LOWER_LIMIT));
	addParam(createParamCentered<Rogan1PWhite>(Vec(100, 40), module, RameligModule::PARAM_UPPER_LIMIT));

	addParam(createParamCentered<Rogan1PWhite>(Vec(20, 80), module, RameligModule::PARAM_CHANCE_RANDOM_JUMP));
	addParam(createParamCentered<VCVButton>(Vec(100, 80), module, RameligModule::PARAM_TRIG_RANDOM_JUMP));
	addParam(createParamCentered<Trimpot>(Vec(140, 80), module, RameligModule::PARAM_FACTOR_JUMP_REMAIN));

	addParam(createParamCentered<Rogan1PWhite>(Vec(20, 120), module, RameligModule::PARAM_CHANCE_MOVE_UP));
	addParam(createParamCentered<Rogan1PWhite>(Vec(60, 160), module, RameligModule::PARAM_CHANCE_REMAIN));
	addParam(createParamCentered<Rogan1PWhite>(Vec(100, 120), module, RameligModule::PARAM_CHANCE_MOVE_DOWN));
	addParam(createParamCentered<Trimpot>(Vec(180, 120), module, RameligModule::PARAM_FACTOR_MOVE_TWO));
	addParam(createParamCentered<Trimpot>(Vec(100, 160), module, RameligModule::PARAM_FACTOR_REMAIN_REPEAT));

	addParam(createParamCentered<Trimpot>(Vec(20, 200), module, RameligModule::PARAM_SCALE));
	
	addInput(createInputCentered<NTPort>(Vec(20, 350), module, RameligModule::IN_GATE));

	addInput(createInputCentered<NTPort>(Vec(60, 40), module, RameligModule::IN_LOWER_LIMIT));
	addInput(createInputCentered<NTPort>(Vec(140, 40), module, RameligModule::IN_UPPER_LIMIT));

	addInput(createInputCentered<NTPort>(Vec(60, 80), module, RameligModule::IN_CHANCE_RANDOM_JUMP));
	addInput(createInputCentered<NTPort>(Vec(60, 120), module, RameligModule::IN_CHANCE_MOVE_UP));
	addInput(createInputCentered<NTPort>(Vec(20, 160), module, RameligModule::IN_CHANCE_REMAIN));
	addInput(createInputCentered<NTPort>(Vec(140, 120), module, RameligModule::IN_CHANCE_MOVE_DOWN));

	addInput(createInputCentered<NTPort>(Vec(60, 200), module, RameligModule::IN_SCALE));
	
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 200), module, RameligModule::PARAM_SCALE_NOTES + 11, RameligModule::LIGHT_SCALE_NOTES + 11));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 220), module, RameligModule::PARAM_SCALE_NOTES + 9, RameligModule::LIGHT_SCALE_NOTES + 9));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 240), module, RameligModule::PARAM_SCALE_NOTES + 7, RameligModule::LIGHT_SCALE_NOTES + 7));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(100, 260), module, RameligModule::PARAM_SCALE_NOTES + 5, RameligModule::LIGHT_SCALE_NOTES + 5));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 280), module, RameligModule::PARAM_SCALE_NOTES + 4, RameligModule::LIGHT_SCALE_NOTES + 4));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 300), module, RameligModule::PARAM_SCALE_NOTES + 2, RameligModule::LIGHT_SCALE_NOTES + 2));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(100, 320), module, RameligModule::PARAM_SCALE_NOTES + 0, RameligModule::LIGHT_SCALE_NOTES + 0));

	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(120, 210), module, RameligModule::PARAM_SCALE_NOTES + 10, RameligModule::LIGHT_SCALE_NOTES + 7));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(120, 230), module, RameligModule::PARAM_SCALE_NOTES + 8, RameligModule::LIGHT_SCALE_NOTES + 5));
	addParam(createLightParamCentered<VCVLightButton<LargeLight<RedLight>>>(Vec(120, 250), module, RameligModule::PARAM_SCALE_NOTES + 6, RameligModule::LIGHT_SCALE_NOTES + 4));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(120, 290), module, RameligModule::PARAM_SCALE_NOTES + 3, RameligModule::LIGHT_SCALE_NOTES + 2));
	addParam(createLightParamCentered<VCVLightButton<DimmedLight<LargeLight<RedLight>>>>(Vec(120, 310), module, RameligModule::PARAM_SCALE_NOTES + 1, RameligModule::LIGHT_SCALE_NOTES + 0));

	addOutput(createOutputCentered<NTPort>(Vec(100, 350), module, RameligModule::OUT_CV));
	addOutput(createOutputCentered<NTPort>(Vec(140, 350), module, RameligModule::OUT_GATE));
	addOutput(createOutputCentered<NTPort>(Vec(180, 350), module, RameligModule::OUT_RANDOM));
}


Model* modelRamelig = createModel<RameligModule, RameligWidget>("ramelig");