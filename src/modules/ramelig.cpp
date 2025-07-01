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
	REMAIN = 5,
	DOWN_ONE = 6,
	DOWN_TWO = 7
};

#define TO_MOVE_INDEX(stepAction) stepAction - UP_TWO

struct RameligData {
	std::array<std::vector<int>, 12> scales;
	unsigned int currentScale;

	int currentOctave;
	int currentScaleIndex;

	float randomJumpChance; // The chance that the melody line will jump for one note and then return to the original position
	float randomMoveChance; // The chance that the melody line will move to another position and stay there

	float moveUpChance; // How likely it is that a move will go up
	float remainChance; // How likely it is that the melody won't move
	float moveDownChance; // How likely it is that a move will go down

	float moveTwoChance; // How likely it is that a move will be two positions

	std::array<float, 3> stepDistribution; // The distribution of possible step actions: random jump, random move, or move around current position
	std::array<float, 5> moveDistribution; // The distribution of possible movements around the curren position: up two, up one, remain, down one and down two

	void calculateDistributions() {
		stepDistribution[RANDOM_JUMP] = randomJumpChance;
		stepDistribution[RANDOM_MOVE] = stepDistribution[RANDOM_JUMP] + randomMoveChance;
		stepDistribution[AROUND_CURRENT] = stepDistribution[RANDOM_MOVE] + moveUpChance + remainChance + moveDownChance;

		moveDistribution[TO_MOVE_INDEX(UP_TWO)] = moveUpChance * moveTwoChance / 10;
		moveDistribution[TO_MOVE_INDEX(UP_ONE)] = moveUpChance;
		moveDistribution[TO_MOVE_INDEX(REMAIN)] = moveUpChance + remainChance;
		moveDistribution[TO_MOVE_INDEX(DOWN_ONE)] = moveDistribution[TO_MOVE_INDEX(REMAIN)] + (moveDownChance * (10 - moveTwoChance) / 10);
		moveDistribution[TO_MOVE_INDEX(DOWN_TWO)] = moveDistribution[TO_MOVE_INDEX(REMAIN)] + moveDownChance;
	}

	RameligStepActions determineChance(std::minstd_rand &generator) {
		float chance = std::uniform_real_distribution<float>(0.f, stepDistribution[AROUND_CURRENT])(generator);

		RameligStepActions result = AROUND_CURRENT;
		for (unsigned int i = 0; i < 3; i++) {
			if (stepDistribution[i] > chance) {
				result = static_cast<RameligStepActions>(i);
				break;
			}
		}

		// TODO: Bias upwards or downwards when moving near the edges of the allowed values
		if (result == AROUND_CURRENT) {
			chance -= randomJumpChance;
			result = DOWN_TWO;
			for (unsigned int i = 0; i < 5; i++) {
				if (moveDistribution[i] > chance) {
					result = static_cast<RameligStepActions>(i + 3);
					break;
				}
			}
		}

		return result;
	}
};

float quantize(float value, std::vector<int>& indices, float lowerLimit, float upperLimit, std::vector<float>& notes) {
	float oct;
	float fract;
	if (value < 0) {
		oct = std::floor(value); // The amount we'll have to subtract from the end result, since we'll make the decimal part positive to easier compare with the quantization notes list
		fract = value - oct; // Results in the fractal part moved between 0 and 1
	} else {
		fract = std::modf(value, &oct);
	}

	float note = -1.f + notes.back();
	for (int i = 0; i < indices.size(); i++) {
		if (fract <= notes[indices[i]]) {
			note = notes[indices[i]];
		} else {
			break;
		}
	}

	return oct + note;
}


RameligModule::RameligModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	m_distribution = std::uniform_real_distribution<float>(0.5f, 3.f);

	configInput(IN_GATE, "Clock");
	configInput(IN_LOWER_LIMIT, "Lower limit");
	configInput(IN_UPPER_LIMIT, "Upper limit");

	configOutput(OUT_CV, "CV");
	configOutput(OUT_GATE, "Gate");

	for (int i = 0; i < 12; i++) {
		m_notes.push_back((float) i / 12);
	}

	m_data = new RameligData();
	m_data->scales[0] = { 0, 2, 4, 7, 9 };
	m_data->currentScale = 0;

	m_data->currentOctave = 0;
	m_data->currentScaleIndex = 0;

	m_data->randomJumpChance = 0.2f;
	m_data->randomMoveChance = 0.2f;
	m_data->moveUpChance = 0.9f;
	m_data->remainChance = 0.3f;
	m_data->moveDownChance = 0.9f;

	m_data->moveTwoChance = 0.3f;

	m_data->calculateDistributions();
}

RameligModule::~RameligModule() {
	delete m_data;
}

void RameligModule::process(const ProcessArgs& args) {
	if (m_trigger.process(inputs[IN_GATE].getVoltage(0), 0.f, 1.f)) {
		float upperLimit = inputs[IN_UPPER_LIMIT].getVoltage();
		float lowerLimit = inputs[IN_LOWER_LIMIT].getVoltage();

		switch (m_data->determineChance(m_generator)) {
			case RANDOM_JUMP:
			case RANDOM_MOVE: {
				float value = std::uniform_real_distribution<float>(lowerLimit, upperLimit)(m_generator);
				value = quantize(value, m_data->scales[m_data->currentScale], lowerLimit, upperLimit, m_notes);
				outputs[OUT_CV].setVoltage(value);

				float whole;
				float fract = std::modf(value, &whole);
				m_data->currentOctave = (int) whole;
				m_data->currentScaleIndex = 0;
				for (int i = 0; i < m_data->scales[m_data->currentScaleIndex].size(); i++) {
					if (m_notes[m_data->scales[m_data->currentScaleIndex][i]] == fract) {
						m_data->currentScaleIndex = i;
						break;
					}
				}
				break;
			}
			case UP_TWO: {
				int oct = m_data->currentOctave;
				int index = m_data->currentScaleIndex + 1;
				if (index >= m_data->scales[m_data->currentScale].size()) {
					oct++;
					index = 0;
				}
				float result = (float) oct + m_notes[m_data->scales[m_data->currentScale][index]];
				if (result > upperLimit) {
					result = (float) m_data->currentOctave + m_notes[m_data->scales[m_data->currentScale][m_data->currentScaleIndex]];
				} else {
					m_data->currentOctave = oct;
					m_data->currentScaleIndex = index;
				}
			}
			case UP_ONE: {
				int oct = m_data->currentOctave;
				int index = m_data->currentScaleIndex + 1;
				if (index >= m_data->scales[m_data->currentScale].size()) {
					oct++;
					index = 0;
				}
				float result = (float) oct + m_notes[m_data->scales[m_data->currentScale][index]];
				if (result > upperLimit) {
					result = (float) m_data->currentOctave + m_notes[m_data->scales[m_data->currentScale][m_data->currentScaleIndex]];
				} else {
					m_data->currentOctave = oct;
					m_data->currentScaleIndex = index;
				}
				break;
			}
			case REMAIN: {
				break;
			}
			case DOWN_TWO: {
				int oct = m_data->currentOctave;
				int index = m_data->currentScaleIndex - 1;
				if (index  < 0) {
					oct--;
					index = m_data->scales[m_data->currentScale].size() - 1;
				}
				float result = (float) oct + m_notes[m_data->scales[m_data->currentScale][index]];
				if (result < lowerLimit) {
					result = (float) m_data->currentOctave + m_notes[m_data->scales[m_data->currentScale][m_data->currentScaleIndex]];
				} else {
					m_data->currentOctave = oct;
					m_data->currentScaleIndex = index;
				}
			}
			case DOWN_ONE: {
				int oct = m_data->currentOctave;
				int index = m_data->currentScaleIndex - 1;
				if (index  < 0) {
					oct--;
					index = m_data->scales[m_data->currentScale].size() - 1;
				}
				float result = (float) oct + m_notes[m_data->scales[m_data->currentScale][index]];
				if (result < lowerLimit) {
					result = (float) m_data->currentOctave + m_notes[m_data->scales[m_data->currentScale][m_data->currentScaleIndex]];
				} else {
					m_data->currentOctave = oct;
					m_data->currentScaleIndex = index;
				}
			}
		}

		float result = (float) m_data->currentOctave + m_notes[m_data->scales[m_data->currentScale][m_data->currentScaleIndex]];
		outputs[OUT_CV].setVoltage(result);
		
		// if (std::uniform_real_distribution<float>(0.f, 100.f)(m_generator) < m_data->randChance) {
		// 	float value = std::uniform_real_distribution<float>(lowerLimit, upperLimit)(m_generator);
		// 	value = quantize(value, m_data->indices);
		// 	outputs[OUT_CV].setVoltage(value);

		// 	float whole;
		// 	float fract = std::modf(value, &whole);
		// 	m_data->oct = (int) whole;
		// 	m_data->index = 0;
		// 	for (int i = 0; i < m_data->indices.size(); i++) {
		// 		if (notes[m_data->indices[i]] == fract) {
		// 			m_data->index = i;
		// 			break;
		// 		}
		// 	}
		// } else {
		// 	int oct = m_data->oct;
		// 	int index = m_data->index;

		// 	float dist = m_distribution(m_generator);

		// 	if (dist > 2.f) {
		// 		index++;
		// 		if (index >= m_data->indices.size()) {
		// 			oct++;
		// 			index = 0;
		// 		}
		// 	} else if (dist > 1.f) {
		// 		index--;
		// 		if (index < 0) {
		// 			oct--;
		// 			index = m_data->indices.size() - 1;
		// 		}
		// 	}

		// 	float result = (float) oct + notes[m_data->indices[index]];
		// 	if ((result > upperLimit) || (result < lowerLimit)) {
		// 		result = (float) m_data->oct + notes[m_data->indices[m_data->index]];
		// 	} else {
		// 		m_data->oct = oct;
		// 		m_data->index = index;
		// 	}

		// 	outputs[OUT_CV].setVoltage(result);
		// }
	}
}

RameligWidget::RameligWidget(RameligModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "ramelig") {
	addInput(createInputCentered<NTPort>(Vec(25, 35), module, RameligModule::IN_GATE));
	addInput(createInputCentered<NTPort>(Vec(25, 75), module, RameligModule::IN_LOWER_LIMIT));
	addInput(createInputCentered<NTPort>(Vec(25, 115), module, RameligModule::IN_UPPER_LIMIT));

	addOutput(createOutputCentered<NTPort>(Vec(65, 35), module, RameligModule::OUT_CV));
	addOutput(createOutputCentered<NTPort>(Vec(65, 75), module, RameligModule::OUT_GATE));

	// addChild(createLightCentered<TinyLight<DimmedLight<GreenRedLight>>>(Vec(40.f, 20.f), module, PipoInputModule::LIGHT_CONNECTED));
}


Model* modelRamelig = createModel<RameligModule, RameligWidget>("ramelig");