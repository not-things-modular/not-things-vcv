#pragma once

#include <array>
#include <vector>
#include <random>
#include <memory>


enum RameligActions {
	RANDOM_JUMP = 0,
	RANDOM_MOVE = 1,
	UP_TWO = 2,
	UP_ONE = 3,
	DOWN_ONE = 4,
	DOWN_TWO = 5,
	REMAIN = 6
};

struct RameligCoreData {
	std::vector<int> scale;

	float randomJumpChance;
	float randomJumpStayOrRemainFactor;

	float moveUpChance;
	float remainChance;
	float moveDownChance;

	float moveTwoFactor;
	float remainRepeatFactor;

	bool operator==(const RameligCoreData& other) const;
	bool operator!=(const RameligCoreData& other) const;
};

struct RameligCoreState {
	std::array<float, 7> actionDistribution;

	int currentOctave = 0.f;
	int currentScaleIndex = 0.f;
	bool lastWasRemain = false;

	float lastResult = 0;
	bool isDirty = true;

	RameligCoreData data;

	void calculateDistribution();
	RameligActions determineAction(ChanceGenerator* chanceGenerator);
};

struct ChanceGenerator {
	virtual ~ChanceGenerator();
	virtual float generateJumpChance(float lower, float upper) = 0;
	virtual float generateActionChance(float lower, float upper) = 0;
};

struct RameligCore {
	RameligCore();
	RameligCore(std::shared_ptr<ChanceGenerator> chanceGenerator);

	void process(RameligCoreData& data, float upperLimit, float lowerLimit);

	private:
		std::shared_ptr<ChanceGenerator> m_chanceGenerator;
		RameligCoreState m_state;
};
