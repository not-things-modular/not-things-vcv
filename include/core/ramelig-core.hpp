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

struct RameligDistributionData {
	float randomJumpChance;
	float randomMoveChance;

	float moveUpChance;
	float remainChance;
	float moveDownChance;

	float moveTwoFactor;
	float remainRepeatFactor;

	bool operator==(const RameligDistributionData& other) const;
	bool operator!=(const RameligDistributionData& other) const;
};

struct RameligCoreState {
	RameligDistributionData distributionData;

	int currentOctave = 0.f;
	int currentScaleIndex = 0.f;
	bool lastWasRemain = false;

	float lastResult = 0;
	bool isDirty = true;

	std::array<float, 7> actionDistribution;
};

struct ChanceGenerator {
	virtual ~ChanceGenerator() {};
	virtual float generateJumpChance(float lower, float upper) = 0;
	virtual float generateActionChance(float lower, float upper) = 0;
};

struct RameligActionListener {
	virtual ~RameligActionListener() {};
	virtual void rameligActionPerformed(int channel, RameligActions action) = 0;
};

struct RameligCore {
	RameligCore(RameligActionListener *actionListener);
	RameligCore(RameligActionListener *actionListener, std::shared_ptr<ChanceGenerator> chanceGenerator);

	void setScale(std::vector<int>& scale);
	float process(int channel, RameligDistributionData& data, bool forceMove, bool forceJump, float lowerLimit, float upperLimit);

	private:
		std::shared_ptr<ChanceGenerator> m_chanceGenerator;
		RameligCoreState m_state[16];
		std::array<float, 12> m_notes;

		std::vector<int> m_scale;
		std::vector<float> m_quantizationValues;

		RameligActionListener *m_actionListener;

		void calculateDistribution(int channel);
		void calculateQuantization();

		RameligActions determineAction(int channel);
		std::pair<int, int> quantize(int channel, float value, float lowerLimit, float upperLimit);
		std::pair<int, int> move(int channel, std::pair<int, int>& current, int movement);
};
