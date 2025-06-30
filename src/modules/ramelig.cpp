#include "modules/ramelig.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"


extern Model* modelRamelig;

enum RameligChanceDistributionIndex {
	JUMP = 0,
	MOVE = 1,
	UP_ONE = 2,
	UP_TWO = 3,
	REMAIN = 4,
	DOWN_ONE = 5,
	DOWN_TWO = 6
};

struct RameligData {
	std::array<std::vector<int>, 12> scales;
	unsigned int currentScale;

	int currentOctave;
	int currentScaleIndex;

	float randomJumpChance; // The chance that the melody line will jump for one note and then return to the original position
	float randomMoveChance; // The chance that the melody line will move to another position and stay there

	float moveUpChance; // How likely it is that a move will go up
	float moveDownChance; // How likely it is that a move will go down
	float remainChance; // How likely it is that the melody won't move

	float moveOneChance; // How likely it is that a move will be one position
	float moveTwoChance; // How likely it is that a move will be two pisitions

	std::array<float, 7> chanceDistribution; // The distibution of the different chance outcomes, indexed by RameligChanceDistributionIndex

	void calculateDistribution() {
		chanceDistribution[JUMP] = randomJumpChance;
		chanceDistribution[MOVE] = chanceDistribution[JUMP] + randomMoveChance;
		
		chanceDistribution[UP_ONE] = chanceDistribution[MOVE] + (moveUpChance * 10 / moveOneChance);
		chanceDistribution[UP_TWO] = chanceDistribution[MOVE] + moveUpChance;

		chanceDistribution[REMAIN] = chanceDistribution[UP_TWO] + remainChance;

		chanceDistribution[DOWN_ONE] = chanceDistribution[REMAIN] + (moveDownChance * 10 / moveOneChance);
		chanceDistribution[DOWN_TWO] = chanceDistribution[REMAIN] + moveDownChance;
	}

	RameligChanceDistributionIndex determineChance(std::minstd_rand &generator) {
		float chance = std::uniform_real_distribution<float>(0.f, chanceDistribution[7])(generator);
		RameligChanceDistributionIndex result = DOWN_TWO;

		for (unsigned int i = 0; i < 7; i++) {
			if (chanceDistribution[i] >= chance) {
				result = static_cast<RameligChanceDistributionIndex>(i);
			}
		}

		return result;
	}
};

float quantize(float value, std::vector<int> indices, float lowerLimit, float upperLimit) {
	float oct;
	float fract;
	if (value < 0) {
		oct = std::floor(value); // The amount we'll have to subtract from the end result, since we'll make the decimal part positive to easier compare with the quantization notes list
		fract = value - oct; // Results in the fractal part moved between 0 and 1
	} else {
		fract = std::modf(value, &oct);
	}

	float q = -1.f;
	for (int i = 0; i < indices.size(); i++) {
		if (fract <= notes[indices[i]]) {
			q = notes[indices[i]];
		} else {
			break;
		}
	}

	if (q == -1.f) {
		whole--;
		q = notes[indices.back()];
	}

	return whole + q;
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

	m_data->randomJumpChance = 0.1f;
	m_data->randomMoveChance = 0.1f;
	m_data->moveUpChance = 0.9f;
	m_data->remainChance = 0.2f;
	m_data->moveDownChance = 0.9f;

	m_data->moveOneChance = 0.7f;
	m_data->moveOneChance = 0.3f;

	m_data->calculateDistribution();
}

RameligModule::~RameligModule() {
	delete m_data;
}

void RameligModule::process(const ProcessArgs& args) {
	if (m_trigger.process(inputs[IN_GATE].getVoltage(0), 0.f, 1.f)) {
		float upperLimit = inputs[IN_UPPER_LIMIT].getVoltage();
		float lowerLimit = inputs[IN_LOWER_LIMIT].getVoltage();

		switch (m_data->determineChance(m_generator)) {
			case JUMP:
			case MOVE:
			case UP_ONE:
			case UP_TWO:
			case REMAIN:
			case DOWN_ONE:
			case DOWN_TWO:
		}
		
		if (std::uniform_real_distribution<float>(0.f, 100.f)(m_generator) < m_data->randChance) {
			float value = std::uniform_real_distribution<float>(lowerLimit, upperLimit)(m_generator);
			value = quantize(value, m_data->indices);
			outputs[OUT_CV].setVoltage(value);

			float whole;
			float fract = std::modf(value, &whole);
			m_data->oct = (int) whole;
			m_data->index = 0;
			for (int i = 0; i < m_data->indices.size(); i++) {
				if (notes[m_data->indices[i]] == fract) {
					m_data->index = i;
					break;
				}
			}
		} else {
			int oct = m_data->oct;
			int index = m_data->index;

			float dist = m_distribution(m_generator);

			if (dist > 2.f) {
				index++;
				if (index >= m_data->indices.size()) {
					oct++;
					index = 0;
				}
			} else if (dist > 1.f) {
				index--;
				if (index < 0) {
					oct--;
					index = m_data->indices.size() - 1;
				}
			}

			float result = (float) oct + notes[m_data->indices[index]];
			if ((result > upperLimit) || (result < lowerLimit)) {
				result = (float) m_data->oct + notes[m_data->indices[m_data->index]];
			} else {
				m_data->oct = oct;
				m_data->index = index;
			}

			outputs[OUT_CV].setVoltage(result);
		}
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