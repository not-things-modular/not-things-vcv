#include "ramelig-core.hpp"


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


bool RameligCoreData::operator==(const RameligCoreData& other) const {
	return !(*this != other);
}

bool RameligCoreData::operator!=(const RameligCoreData& other) const {
	return randomJumpChance != other.randomJumpChance ||
		randomJumpStayOrRemainFactor != other.randomJumpStayOrRemainFactor ||
		moveUpChance != other.moveUpChance ||
		remainChance != other.remainChance ||
		moveDownChance != other.moveDownChance ||
		moveTwoFactor != other.moveTwoFactor ||
		remainRepeatFactor != other.remainRepeatFactor ||
		scale != other.scale;
}


void RameligCoreState::calculateDistribution() {
	actionDistribution[RameligActions::RANDOM_JUMP] = data.randomJumpChance * data.randomJumpStayOrRemainFactor;
	actionDistribution[RameligActions::RANDOM_MOVE] = data.randomJumpChance;
	actionDistribution[RameligActions::UP_TWO] = actionDistribution[RameligActions::RANDOM_MOVE] + data.moveUpChance * data.moveTwoFactor;
	actionDistribution[RameligActions::UP_ONE] = actionDistribution[RameligActions::RANDOM_MOVE] + data.moveUpChance;
	actionDistribution[RameligActions::DOWN_ONE] = actionDistribution[RameligActions::UP_ONE] + data.moveDownChance * (1 - data.moveTwoFactor);
	actionDistribution[RameligActions::DOWN_TWO] = actionDistribution[RameligActions::UP_ONE] + data.moveDownChance;
	actionDistribution[RameligActions::REMAIN] = actionDistribution[RameligActions::DOWN_TWO] + data.remainChance;
}

RameligActions RameligCoreState::determineAction(ChanceGenerator* chanceGenerator) {
	float upperLimit = actionDistribution[REMAIN];
	if (lastWasRemain) {
		upperLimit -= data.remainChance * (1 - data.remainRepeatFactor);
	}
	float chance = chanceGenerator->generateActionChance(0.f, upperLimit);

	RameligActions result = RameligActions::RANDOM_JUMP;
	for (unsigned int i = 0; i < 7; i++) {
		if (actionDistribution[i] >= chance) {
			result = static_cast<RameligActions>(i);
		}
	}

	return result;
}


RameligCore::RameligCore() : RameligCore(std::make_shared<UniformChanceGenerator>()) {}
RameligCore::RameligCore(std::shared_ptr<ChanceGenerator> chanceGenerator) : m_chanceGenerator(m_chanceGenerator) {}

void RameligCore::process(RameligCoreData& data, float upperLimit, float lowerLimit) {
	if (data != m_state.data) {
		m_state.data = data;
		m_state.isDirty = true;
		m_state.calculateDistribution();
	}

	RameligActions action = m_state.determineAction(m_chanceGenerator.get());
}