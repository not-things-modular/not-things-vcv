#pragma once

#include <array>
#include <vector>
#include <random>
#include <memory>


enum RameligActions {
	RANDOM_JUMP = 0,
	RANDOM_SHIFT = 1,
	UP_TWO = 2,
	UP_ONE = 3,
	DOWN_ONE = 4,
	DOWN_TWO = 5,
	STAY = 6
};

struct RameligDistributionData {
	float randomJumpChance;
	float randomShiftChance;

	float moveUpChance;
	float stayChance;
	float moveDownChance;

	float moveTwoFactor;
	float stayRepeatFactor;

	bool operator==(const RameligDistributionData& other) const;
	bool operator!=(const RameligDistributionData& other) const;
};

struct RameligCoreState {
	RameligDistributionData distributionData;

	int currentOctave = 0;
	int currentScaleIndex = 0;
	bool lastWasStay = false;

	float lastResult = 0;
	bool isDirty = true;

	std::array<float, 7> actionDistribution;
};

struct RameligChanceGenerator {
	virtual ~RameligChanceGenerator() {};
	virtual float generateJumpChance(float lower, float upper) = 0;
	virtual float generateActionChance(float lower, float upper) = 0;
};

struct RameligActionListener {
	virtual ~RameligActionListener() {};
	virtual void rameligActionPerformed(int coreId, RameligActions action) = 0;
};

struct RameligScale {
	RameligScale();

	void setScale(const std::vector<int>& scale);

	std::pair<int, int> quantize(float value, float lowerLimit, float upperLimit);
	std::pair<int, int> move(std::pair<int, int>& current, int movement);

	float quantizedToVoltage(const std::pair<int, int>& quantized);

	private:
		std::array<float, 12> m_notes;
		std::vector<int> m_scale;

		std::vector<float> m_quantizationValues;

		void calculateQuantization();
};

struct RameligCore {
	RameligCore(int id, std::shared_ptr<RameligScale> rameligScale, RameligActionListener *actionListener);
	RameligCore(int id, std::shared_ptr<RameligScale> rameligScale, RameligActionListener *actionListener, std::shared_ptr<RameligChanceGenerator> chanceGenerator);

	void guideLast(float value);
	float process(RameligDistributionData& data, bool forceJump, bool forceShift, bool forceStay, float lowerLimit, float upperLimit);

	void calculateDistribution(RameligDistributionData& data, std::array<float, 7>& distribution);

	private:
		int m_id;

		std::shared_ptr<RameligChanceGenerator> m_chanceGenerator;
		std::shared_ptr<RameligScale> m_scale;

		RameligCoreState m_state;

		RameligActionListener *m_actionListener;

		void calculateDistribution();

		RameligActions determineAction();
};
