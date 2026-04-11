#include "core/ramelig-core.hpp"


struct RameligUniformChanceGenerator : RameligChanceGenerator {
	RameligUniformChanceGenerator() {
		m_jumpLower = -1.f;
		m_jumpUpper = 1.f;
		m_jumpDistribution = std::uniform_real_distribution<float>(m_jumpLower, m_jumpUpper);
		m_actionLower = -1.f;
		m_actionUpper = 1.f;
		m_actionDistribution = std::uniform_real_distribution<float>(m_actionLower, m_actionUpper);

		m_generator.seed(std::random_device{}());
	}

	~RameligUniformChanceGenerator() {}

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
		randomShiftChance != other.randomShiftChance ||
		moveUpChance != other.moveUpChance ||
		stayChance != other.stayChance ||
		moveDownChance != other.moveDownChance ||
		moveTwoFactor != other.moveTwoFactor ||
		stayRepeatFactor != other.stayRepeatFactor;
}


RameligScale::RameligScale() {
	for (int i = 0; i < 12; i++) {
		m_notes[i] = (float) i / 12.f;
	}
	m_scale = { 0 };

	calculateQuantization();
}

void RameligScale::setScale(std::vector<int>& scale) {
	if (scale != m_scale) {
		m_scale = scale;
		calculateQuantization();
	}
}

void RameligScale::calculateQuantization() {
	// The quantizationValues contains a list of floats, so that you have to quantize to the index if:
	// - The list item at that index is lower or equal to the current value
	// - The list item at (index + 1) is higher than the current value
	m_quantizationValues.clear();

	if (m_scale.size() > 1) {
		// The first item is the lower limit, so it is the average of the first note in this scale and the last note of the previous one
		m_quantizationValues.push_back((m_notes[m_scale.back()] - 1.f + m_notes.front()) / 2);
		// Now loop through all items in the scale and the average of that item combined with the next one
		for (unsigned int i = 0; i < m_scale.size() - 1; i++) {
			m_quantizationValues.push_back((m_notes[m_scale[i]] + m_notes[m_scale[i + 1]]) / 2.f);
		}
		// Finally add the average of the last item in this scale and the first of the next scale, which is the upper boundry of the range for the last item in this scale
		m_quantizationValues.push_back((m_notes[m_scale.front()] + 1.f + m_notes[m_scale.back()]) / 2);
	} else if (m_scale.size() > 0) {
		// There is only one item in the scale, so just add the range it.
		m_quantizationValues.push_back(m_notes[m_scale[0]] - .5f);
		m_quantizationValues.push_back(m_notes[m_scale[0]]);
	}
}

std::pair<int, int> RameligScale::quantize(float value, float lowerLimit, float upperLimit) {
	float oct;
	int index = 0;
	float fract;
	if (value < 0) {
		oct = std::floor(value); // The amount we'll have to subtract from the end result, since we'll make the decimal part positive to easier compare with the quantization notes list
		fract = value - oct; // Results in the fractal part moved between 0 and 1
	} else {
		// For a positive value, we can just use modf to separate into octave and fractal part (for a negative value, it would result in negative oct and fract)
		fract = std::modf(value, &oct);
	}

	if (fract < m_quantizationValues.front()) {
		// If the value is below the first item in the quantization list, we'll have to quantize towards the last note of the previous octave
		oct--;
		index = m_scale.size() - 1;
	} else if (fract > m_quantizationValues.back()) {
		// If the value is above the last item in the quantization list, we'll have to quantize towards the first note of the next octave
		oct++;
		index = 0;
	} else {
		// Look for the position in the quantization values list where the next item in the list is above the current value
		for (int i = 0; i < (int) m_quantizationValues.size() - 1; i++) {
			if (fract <= m_quantizationValues[i + 1]) {
				// We found the index in the scale to quantize to, so take that index and undo the octave-up we did before.
				index = i;
				break;
			}
		}
	}

	// Make sure the quantized result remains within the limits
	while (oct + m_notes[m_scale[index]] < lowerLimit) {
		index++;
		if (index >= (int) m_scale.size()) {
			oct++;
			index = 0;
		}
	}

	while (oct + m_notes[m_scale[index]] > upperLimit) {
		index--;
		if (index < 0) {
			oct--;
			index = m_scale.size() - 1;
		}
	}

	return std::make_pair(oct, index);
}

std::pair<int, int> RameligScale::move(std::pair<int, int>& current, int movement) {
	int oct = current.first;
	int index = current.second + movement;
	if (index >= (int) m_scale.size()) {
		oct++;
		while (index >= (int) m_scale.size()) {
			index -= m_scale.size();
		}
	}
	while ((index < 0) && (m_scale.size() > 0)) {
		oct--;
		index = m_scale.size() + index;
	}

	return std::make_pair(oct, index);
}

float RameligScale::quantizedToVoltage(std::pair<int, int>& quantized) {
	return (float) quantized.first + m_notes[m_scale[quantized.second]];
}

RameligCore::RameligCore(int id, std::shared_ptr<RameligScale> rameligScale, RameligActionListener *actionListener) : RameligCore(id, rameligScale, actionListener, std::make_shared<RameligUniformChanceGenerator>()) {}
RameligCore::RameligCore(int id, std::shared_ptr<RameligScale> rameligScale, RameligActionListener *actionListener, std::shared_ptr<RameligChanceGenerator> chanceGenerator) : m_id(id), m_chanceGenerator(chanceGenerator), m_scale(rameligScale), m_actionListener(actionListener) {
}

void RameligCore::guideLast(float value) {
	m_state.lastResult = value;
	m_state.isDirty = true;
}

float RameligCore::process(RameligDistributionData& data, bool forceJump, bool forceShift, bool forceStay, float lowerLimit, float upperLimit) {
	std::pair<int, int> quantized;

	// Update the distribution if needed
	if (m_state.distributionData != data) {
		m_state.distributionData = data;
		m_state.isDirty = true;
		calculateDistribution();
	}

	// If the state is dirty, quantize the lastResult to the current scale
	if (m_state.isDirty) {
		quantized = m_scale->quantize(m_state.lastResult, lowerLimit, upperLimit);
		m_state.currentOctave = quantized.first;
		m_state.currentScaleIndex = quantized.second;
		m_state.lastResult = m_scale->quantizedToVoltage(quantized);
	}

	// Start from the previous result
	float result = m_state.lastResult;

	// Perform the next action
	RameligActions action;
	if (forceJump) {
		action = RameligActions::RANDOM_JUMP;
	} else if (forceShift) {
		action = RameligActions::RANDOM_SHIFT;
	} else if (forceStay) {
		action = RameligActions::STAY;
	} else {
		action = determineAction();
	}
	if ((action == RANDOM_JUMP) || (action == RANDOM_SHIFT)) {
		float randomValue = m_chanceGenerator->generateJumpChance(lowerLimit, upperLimit);
		quantized = m_scale->quantize(randomValue, lowerLimit, upperLimit);
		result = m_scale->quantizedToVoltage(quantized);
		if (action == RANDOM_SHIFT) {
			m_state.currentOctave = quantized.first;
			m_state.currentScaleIndex = quantized.second;
			m_state.lastResult = result;
		}
	} else if (action != STAY) {
		// Determine which movement we have to do
		int movement = 1;
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
		quantized = m_scale->move(quantized, movement);
		result = m_scale->quantizedToVoltage(quantized);

		// If the movement pushed us outside of the limits, move in the other direction
		if (((movement > 0) && (result > upperLimit)) || ((movement < 0) && (result < lowerLimit))) {
			quantized.first = m_state.currentOctave;
			quantized.second = m_state.currentScaleIndex;
			quantized = m_scale->move(quantized, -movement);
			result = m_scale->quantizedToVoltage(quantized);
		}

		m_state.currentOctave = quantized.first;
		m_state.currentScaleIndex = quantized.second;
		m_state.lastResult = result;
	}

	// Remember if we had a stay action in this cycle
	m_state.lastWasStay = (action == STAY);
	// Processing is done, so clear the dirty flag
	m_state.isDirty = false;

	// Notify the listener of the performed action
	if (m_actionListener != nullptr) {
		m_actionListener->rameligActionPerformed(m_id, action);
	}

	return result;
}

void RameligCore::calculateDistribution() {
	calculateDistribution(m_state.distributionData, m_state.actionDistribution);
}

void RameligCore::calculateDistribution(RameligDistributionData& data, std::array<float, 7>& distribution) {
	distribution[RameligActions::RANDOM_JUMP] = data.randomJumpChance;
	distribution[RameligActions::RANDOM_SHIFT] = distribution[RameligActions::RANDOM_JUMP] + data.randomShiftChance;
	distribution[RameligActions::UP_TWO] = distribution[RameligActions::RANDOM_SHIFT] + data.moveUpChance * data.moveTwoFactor;
	distribution[RameligActions::UP_ONE] = distribution[RameligActions::RANDOM_SHIFT] + data.moveUpChance;
	distribution[RameligActions::DOWN_ONE] = distribution[RameligActions::UP_ONE] + data.moveDownChance * (1 - data.moveTwoFactor);
	distribution[RameligActions::DOWN_TWO] = distribution[RameligActions::UP_ONE] + data.moveDownChance;
	distribution[RameligActions::STAY] = distribution[RameligActions::DOWN_TWO] + data.stayChance;
}

RameligActions RameligCore::determineAction() {
	float upperLimit = m_state.actionDistribution[STAY];
	if (m_state.lastWasStay) {
		upperLimit -= m_state.distributionData.stayChance * (1 - m_state.distributionData.stayRepeatFactor);
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
