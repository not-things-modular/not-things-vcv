#include "core/ramelig-core.hpp"


#define QUANTIZED_TO_VOLTAGE(quantized, notes, scale) ((float) quantized.first + notes[scale[quantized.second]])


struct UniformChanceGenerator : ChanceGenerator {
	UniformChanceGenerator() {
		m_jumpLower = -1.f;
		m_jumpUpper = 1.f;
		m_jumpDistribution = std::uniform_real_distribution<float>(m_jumpLower, m_jumpUpper);
		m_actionLower = -1.f;
		m_actionUpper = 1.f;
		m_actionDistribution = std::uniform_real_distribution<float>(m_actionLower, m_actionUpper);
	}

	~UniformChanceGenerator() {}

	float generateJumpChance(float lower, float upper) override {
		if ((lower != m_jumpLower) || (upper != m_jumpUpper)) {
			m_jumpLower = lower;
			m_jumpUpper = upper;
			m_jumpDistribution.param(std::uniform_real_distribution<float>::param_type(m_jumpLower, m_jumpUpper));
		}

		return m_jumpDistribution(m_generator);
	}

	float generateActionChance(float lower, float upper) override {
		if ((lower != m_actionLower) || (upper != m_actionUpper)) {
			m_actionLower = lower;
			m_actionUpper = upper;
			m_actionDistribution.param(std::uniform_real_distribution<float>::param_type(m_actionLower, m_actionUpper));
		}

		return m_actionDistribution(m_generator);
	}

	private:
		std::minstd_rand m_generator;

		std::uniform_real_distribution<float> m_jumpDistribution;
		float m_jumpLower;
		float m_jumpUpper;

		std::uniform_real_distribution<float> m_actionDistribution;
		float m_actionLower;
		float m_actionUpper;
};


bool RameligDistributionData::operator==(const RameligDistributionData& other) const {
	return !(*this != other);
}

bool RameligDistributionData::operator!=(const RameligDistributionData& other) const {
	return randomJumpChance != other.randomJumpChance ||
		randomMoveChance != other.randomMoveChance ||
		moveUpChance != other.moveUpChance ||
		remainChance != other.remainChance ||
		moveDownChance != other.moveDownChance ||
		moveTwoFactor != other.moveTwoFactor ||
		remainRepeatFactor != other.remainRepeatFactor;
}


RameligCore::RameligCore() : RameligCore(std::make_shared<UniformChanceGenerator>()) {}
RameligCore::RameligCore(std::shared_ptr<ChanceGenerator> chanceGenerator) : m_chanceGenerator(chanceGenerator) {
	for (int i = 0; i < 12; i++) {
		m_notes[i] = (float) i / 12.f;
	}
}

float RameligCore::process(RameligCoreData& data, float lowerLimit, float upperLimit) {
	std::pair<int, int> quantized;

	// Update the distribution if needed
	if (m_state.data.distributionData != data.distributionData) {
		m_state.data.distributionData = data.distributionData;
		m_state.isDirty = true;
		calculateDistribution();
	}

	// Update the scale quantization values if needed
	if (m_state.data.scale != data.scale) {
		m_state.data.scale = data.scale;
		m_state.isDirty = true;
		calculateQuantization();
	}

	// If the state is dirty, quantize the lastResult to the current scale
	if (m_state.isDirty) {
		quantized = quantize(m_state.lastResult, lowerLimit, upperLimit);
		m_state.currentOctave = quantized.first;
		m_state.currentScaleIndex = quantized.second;
		m_state.lastResult = QUANTIZED_TO_VOLTAGE(quantized, m_notes, m_state.data.scale);
	}

	// Start from the previous result
	float result = m_state.lastResult;
	
	// Perform the next action
	RameligActions action = determineAction();
	if ((action == RANDOM_JUMP) || (action == RANDOM_MOVE)) {
		float randomValue = m_chanceGenerator->generateJumpChance(lowerLimit, upperLimit);
		quantized = quantize(randomValue, lowerLimit, upperLimit);
		if (action == RANDOM_MOVE) {
			m_state.currentOctave = quantized.first;
			m_state.currentScaleIndex = quantized.second;
		}
		result = QUANTIZED_TO_VOLTAGE(quantized, m_notes, m_state.data.scale);
		result = QUANTIZED_TO_VOLTAGE(quantized, m_notes, m_state.data.scale);
		quantized = quantize(randomValue, lowerLimit, upperLimit);
	} else if (action != REMAIN) {
		// Determine which movement we have to do
		int movement;
		if (action == UP_TWO) {
			movement = 2;
		} else if (action == UP_ONE) {
			movement = 1;
		} else if (action == DOWN_ONE) {
			movement = -1;
		} else if (action == DOWN_TWO) {
			movement = -2;
		}

		// Perform the movement
		quantized.first = m_state.currentOctave;
		quantized.second = m_state.currentScaleIndex;
		quantized = move(quantized, movement);
		result = QUANTIZED_TO_VOLTAGE(quantized, m_notes, m_state.data.scale);

		// If the movement pushed us outside of the limits, move in the other direction
		if (((movement > 0) && (result > upperLimit)) || ((movement < 0) && (result < lowerLimit))) {
			quantized.first = m_state.currentOctave;
			quantized.second = m_state.currentScaleIndex;
			quantized = move(quantized, -movement);
			result = QUANTIZED_TO_VOLTAGE(quantized, m_notes, m_state.data.scale);
		}

		m_state.currentOctave = quantized.first;
		m_state.currentScaleIndex = quantized.second;
		m_state.lastResult = result;
	}

	// Remember if we had a remain action in this cycle
	m_state.lastWasRemain = (action == REMAIN);
	// Processing is done, so clear the dirty flag
	m_state.isDirty = false;

	return result;
}

void RameligCore::calculateDistribution() {
	m_state.actionDistribution[RameligActions::RANDOM_JUMP] = m_state.data.distributionData.randomJumpChance;
	m_state.actionDistribution[RameligActions::RANDOM_MOVE] = m_state.actionDistribution[RameligActions::RANDOM_JUMP] + m_state.data.distributionData.randomMoveChance;
	m_state.actionDistribution[RameligActions::UP_TWO] = m_state.actionDistribution[RameligActions::RANDOM_MOVE] + m_state.data.distributionData.moveUpChance * m_state.data.distributionData.moveTwoFactor;
	m_state.actionDistribution[RameligActions::UP_ONE] = m_state.actionDistribution[RameligActions::RANDOM_MOVE] + m_state.data.distributionData.moveUpChance;
	m_state.actionDistribution[RameligActions::DOWN_ONE] = m_state.actionDistribution[RameligActions::UP_ONE] + m_state.data.distributionData.moveDownChance * (1 - m_state.data.distributionData.moveTwoFactor);
	m_state.actionDistribution[RameligActions::DOWN_TWO] = m_state.actionDistribution[RameligActions::UP_ONE] + m_state.data.distributionData.moveDownChance;
	m_state.actionDistribution[RameligActions::REMAIN] = m_state.actionDistribution[RameligActions::DOWN_TWO] + m_state.data.distributionData.remainChance;
}

void RameligCore::calculateQuantization() {
	// The quantizationValues contains a list of floats, so that you have to quantize to the index if:
	// - The list item at that index is lower or equal to the current value
	// - The list item at (index + 1) is higher than the current value
	std::vector<int>& scale = m_state.data.scale;
	m_state.quantizationValues.clear();

	if (scale.size() > 1) {
		// The first item is the lower limit, so it is the average of the first note in this scale and the last note of the previous one
		m_state.quantizationValues.push_back((m_notes[scale.back()] - 1.f + m_notes.front()) / 2);
		// Now loop through all items in the scale and the average of that item combined with the next one
		for (unsigned int i = 0; i < scale.size() - 1; i++) {
			m_state.quantizationValues.push_back((m_notes[scale[i]] + m_notes[scale[i + 1]]) / 2.f);
		}
		// Finally add the average of the last item in this scale and the first of the next scale, which is the upper boundry of the range for the last item in this scale
		m_state.quantizationValues.push_back((m_notes[scale.front()] + 1.f + m_notes[scale.back()]) / 2);
	} else {
		// There is only one item in the scale, so just add the range it.
		m_state.quantizationValues.push_back(m_notes[scale[0]] - .5f);
		m_state.quantizationValues.push_back(m_notes[scale[0]]);
	}
}

RameligActions RameligCore::determineAction() {
	float upperLimit = m_state.actionDistribution[REMAIN];
	if (m_state.lastWasRemain) {
		upperLimit -= m_state.data.distributionData.remainChance * (1 - m_state.data.distributionData.remainRepeatFactor);
	}
	float chance = m_chanceGenerator->generateActionChance(0.f, upperLimit);

	RameligActions result = RameligActions::RANDOM_JUMP;
	for (unsigned int i = 0; i < 7; i++) {
		if (m_state.actionDistribution[i] >= chance) {
			result = static_cast<RameligActions>(i);
			break;
		}
	}

	return result;
}

std::pair<int, int> RameligCore::quantize(float value, float lowerLimit, float upperLimit) {
	float oct;
	int index;
	float fract;
	if (value < 0) {
		oct = std::floor(value); // The amount we'll have to subtract from the end result, since we'll make the decimal part positive to easier compare with the quantization notes list
		fract = value - oct; // Results in the fractal part moved between 0 and 1
	} else {
		fract = std::modf(value, &oct);
	}

	if (fract < m_state.quantizationValues.front()) {
		// If the value is below the first item in the quantization list, we'll have to quantize towards the last note of the previous octave
		oct--;
		index = m_state.data.scale.size() - 1;
	} else if (fract > m_state.quantizationValues.back()) {
		// If the value is above the last item in the quantization list, we'll have to quantize towards the first note of the next octave
		oct++;
		index = 0;
	} else {
		// Look for the position in the quantization values list where the next item in the list is above the current value
		for (int i = 0; i < (int) m_state.quantizationValues.size() - 1; i++) {
			if (fract <= m_state.quantizationValues[i + 1]) {
				// We found the index in the scale to quantize to, so take that index and undo the octave-up we did before.
				index = i;
				break;
			}
		}
	}

	// Make sure the quantized result remains within the limits
	while (oct + m_notes[m_state.data.scale[index]] < lowerLimit) {
		index++;
		if (index >= (int) m_state.data.scale.size()) {
			oct++;
			index = 0;
		}
	}

	while (oct + m_notes[m_state.data.scale[index]] > upperLimit) {
		index--;
		if (index < 0) {
			oct--;
			index = m_state.data.scale.size() - 1;
		}
	}

	return std::make_pair(oct, index);
}

std::pair<int, int> RameligCore::move(std::pair<int, int>& current, int movement) {
	int oct = current.first;
	int index = current.second + movement;
	if (index >= (int) m_state.data.scale.size()) {
		oct++;
		index = 0;
	}
	if (index < 0) {
		oct--;
		index = m_state.data.scale.size() - 1;
	}

	return std::make_pair(oct, index);
}