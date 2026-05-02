#include <gmock/gmock.h>
#include "core/ramelig-core.hpp"

struct MockRameligChanceGenerator : RameligChanceGenerator {
	MOCK_METHOD(float, generateJumpChance, (float, float), (override));
	MOCK_METHOD(float, generateActionChance, (float, float), (override));
};

struct MockRameligActionListener : RameligActionListener {
	MOCK_METHOD(void, rameligActionPerformed, (int, RameligActions), (override));
};

RameligDistributionData populateDistributionData(float randomJumpChance, float randomShiftChance, float moveUpChance, float stayChance, float moveDownChance, float moveTwoFactor) {
	RameligDistributionData data;

	data.randomJumpChance = randomJumpChance;
	data.randomShiftChance = randomShiftChance;
	data.moveUpChance = moveUpChance;
	data.stayChance = stayChance;
	data.moveDownChance = moveDownChance;
	data.moveTwoFactor = moveTwoFactor;
	data.stayRepeatFactor = 0.5f;

	return data;
}

TEST(RameligCore, calculateDistributionShouldConstructBalancedDistributon) {
	std::array<float, 7> distribution;
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, 0.5f);

	RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

	EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 1.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 2.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_TWO], 3.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_ONE], 4.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 5.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 6.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::STAY], 7.f, 0.00001f);
}

TEST(RameligCore, calculateDistributionShouldSetJumpDistribution) {
	std::array<float, 7> distribution;
	RameligDistributionData data = populateDistributionData(1.234f, 1.f, 2.f, 1.f, 2.f, 0.5f);

	RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

	EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 1.234f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 2.234f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_TWO], 3.234f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_ONE], 4.234f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 5.234f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 6.234f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::STAY], 7.234f, 0.00001f);
}

TEST(RameligCore, calculateDistributionShouldSetShiftDistribution) {
	std::array<float, 7> distribution;
	RameligDistributionData data = populateDistributionData(1.f, 1.234f, 2.f, 1.f, 2.f, 0.5f);

	RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

	EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 1.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 2.234f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_TWO], 3.234f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_ONE], 4.234f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 5.234f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 6.234f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::STAY], 7.234f, 0.00001f);
}

TEST(RameligCore, calculateDistributionShouldSetUpDistribution) {
	std::array<float, 7> distribution;
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 3.f, 1.f, 2.f, 0.5f);

	RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

	EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 1.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 2.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_TWO], 3.5f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_ONE], 5.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 6.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 7.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::STAY], 8.f, 0.00001f);
}

TEST(RameligCore, calculateDistributionShouldSetDownDistribution) {
	std::array<float, 7> distribution;
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 3.f, 0.5f);

	RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

	EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 1.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 2.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_TWO], 3.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_ONE], 4.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 5.5f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 7.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::STAY], 8.f, 0.00001f);
}

TEST(RameligCore, calculateDistributionShouldSetStayDistribution) {
	std::array<float, 7> distribution;
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.234f, 2.f, 0.5f);

	RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

	EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 1.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 2.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_TWO], 3.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_ONE], 4.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 5.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 6.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::STAY], 7.234f, 0.00001f);
}

TEST(RameligCore, calculateDistributionWithZeroMoveTwoFactorShouldMoveOne) {
	std::array<float, 7> distribution;
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, 0.f);

	RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

	EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 1.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 2.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_TWO], 2.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_ONE], 4.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 6.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 6.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::STAY], 7.f, 0.00001f);
}

TEST(RameligCore, calculateDistributionWithOneMoveTwoFactorShouldMoveTwo) {
	std::array<float, 7> distribution;
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, 1.f);

	RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

	EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 1.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 2.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_TWO], 4.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_ONE], 4.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 4.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 6.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::STAY], 7.f, 0.00001f);
}

TEST(RameligCore, calculateDistributionWithPartialMoveTwoFactorShouldMoveOneAndTwo) {
	std::array<float, 7> distribution;
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, 0.75f);

	RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

	EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 1.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 2.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_TWO], 3.5f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_ONE], 4.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 4.5f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 6.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::STAY], 7.f, 0.00001f);
}

TEST(RameligCore, calculateDistributionWithPartialMoveTwoFactorShouldMoveOneAndTwoProportionally) {
	std::array<float, 7> distribution;
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 4.5f, 1.f, 3.5f, 0.75f);

	RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

	EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 1.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 2.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_TWO], 5.375f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_ONE], 6.5f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 7.375f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 10.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::STAY], 11.f, 0.00001f);
}

TEST(RameligCore, calculateDistributionWithAllZerosShouldWork) {
	std::array<float, 7> distribution;
	RameligDistributionData data = populateDistributionData(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);

	RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

	EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 0.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 0.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_TWO], 0.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::UP_ONE], 0.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 0.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 0.f, 0.00001f);
	EXPECT_NEAR(distribution[RameligActions::STAY], 0.f, 0.00001f);
}

TEST(RameligCore, processShouldForceJump) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 1.f, 1.f, 1.f, 1.f);
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::RANDOM_JUMP));
	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance).Times(0);
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance(-5.f, 5.f)).Times(1);

	RameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator).process(data, true, false, false, -5.f, 5.f);
}

TEST(RameligCore, processShouldForceShift) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 1.f, 1.f, 1.f, 1.f);
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::RANDOM_SHIFT));
	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance).Times(0);
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance(-5.f, 5.f)).Times(1).WillOnce(testing::Return(-2.5f));

	RameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator).process(data, false, true, false, -5.f, 5.f);
}

TEST(RameligCore, processShouldForceStay) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 1.f, 1.f, 1.f, 1.f);
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::STAY));
	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance).Times(0);
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance).Times(0);

	RameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator).process(data, false, false, true, -5.f, 5.f);
}

TEST(RameligCore, processShouldPreferForceJumpOverForceShiftAndForceStay) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 1.f, 1.f, 1.f, 1.f);
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::RANDOM_JUMP));
	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance).Times(0);
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance(-5.f, 5.f)).Times(1);

	RameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator).process(data, true, true, true, -5.f, 5.f);
}

TEST(RameligCore, processShouldPreferForceShiftOverForceStay) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 1.f, 1.f, 1.f, 1.f);
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::RANDOM_SHIFT));
	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance).Times(0);
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance(-5.f, 5.f)).Times(1);

	RameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator).process(data, false, true, true, -5.f, 5.f);
}

TEST(RameligCore, processShouldPerformRandomJumpActionWhenGenerated) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(3)
		.WillOnce(testing::Return(0.f))
		.WillOnce(testing::Return(0.49f))
		.WillOnce(testing::Return(1.f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(3)
		.WillOnce(testing::Return(2.5f))
		.WillOnce(testing::Return(-2.5f))
		.WillOnce(testing::Return(3.5f));
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::RANDOM_JUMP))
		.Times(3);

	float result = RameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator).process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(2.5f, -10.f, 10.f)), 0.0001f);

	result = RameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator).process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(-2.5f, -10.f, 10.f)), 0.0001f);

	result = RameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator).process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(3.5f, -10.f, 10.f)), 0.0001f);
}

TEST(RameligCore, processShouldPerformRandomShiftActionWhenGenerated) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(3)
		.WillOnce(testing::Return(1.0001f))
		.WillOnce(testing::Return(1.49f))
		.WillOnce(testing::Return(2.f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(3)
		.WillOnce(testing::Return(2.5f))
		.WillOnce(testing::Return(-2.5f))
		.WillOnce(testing::Return(3.5f));
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::RANDOM_SHIFT))
		.Times(3);

	float result = RameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator).process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(2.5f, -10.f, 10.f)), 0.0001f);

	result = RameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator).process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(-2.5f, -10.f, 10.f)), 0.0001f);

	result = RameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator).process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(3.5f, -10.f, 10.f)), 0.0001f);
}

TEST(RameligCore, processShouldPerformUpTwoActionWhenGenerated) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(3)
		.WillOnce(testing::Return(2.0001f))
		.WillOnce(testing::Return(2.49f))
		.WillOnce(testing::Return(3.f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::UP_TWO))
		.Times(3);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(0, 2)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(0, 4)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(0, 6)), 0.0001f);
}

TEST(RameligCore, processShouldPerformUpOneActionWhenGenerated) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(3)
		.WillOnce(testing::Return(3.0001f))
		.WillOnce(testing::Return(3.49f))
		.WillOnce(testing::Return(4.f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::UP_ONE))
		.Times(3);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(0, 1)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(0, 2)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(0, 3)), 0.0001f);
}

TEST(RameligCore, processShouldPerformDownOneActionWhenGenerated) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(3)
		.WillOnce(testing::Return(4.0001f))
		.WillOnce(testing::Return(4.49f))
		.WillOnce(testing::Return(5.f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::DOWN_ONE))
		.Times(3);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(-1, 6)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(-1, 5)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(-1, 4)), 0.0001f);
}

TEST(RameligCore, processShouldPerformDownTwoActionWhenGenerated) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(3)
		.WillOnce(testing::Return(5.0001f))
		.WillOnce(testing::Return(5.49f))
		.WillOnce(testing::Return(6.f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::DOWN_TWO))
		.Times(3);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(-1, 5)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(-1, 3)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(-1, 1)), 0.0001f);
}

TEST(RameligCore, processShouldPerformStayActionWhenGeneratedWithRepeatReduction) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(1)
		.WillOnce(testing::Return(6.0001f));
	// Since it's a repeat stay, the second and third call will hae a reduced stay chance.
	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 6.5f))
		.Times(2)
		.WillOnce(testing::Return(6.25f))
		.WillOnce(testing::Return(6.5f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::STAY))
		.Times(3);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	// Guide the core once so we move away from the initial 0
	rameligCore.guideLast(-3.25f);

	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(-3.25, -10.f, 10.f)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(-3.25, -10.f, 10.f)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(-3.25, -10.f, 10.f)), 0.0001f);
}

TEST(RameligCore, processShouldPerformStayActionWhenGeneratedWithoutRepeatReduction) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale
	data.stayRepeatFactor = 1.f; // Set the repeat factor to max, so it shouldn't reduce the stay repeat chance

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(3)
		.WillOnce(testing::Return(6.0001f))
		.WillOnce(testing::Return(6.49f))
		.WillOnce(testing::Return(7.f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::STAY))
		.Times(3);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	// Guide the core once so we move away from the initial 0
	rameligCore.guideLast(-3.25f);

	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(-3.25, -10.f, 10.f)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(-3.25, -10.f, 10.f)), 0.0001f);

	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(-3.25, -10.f, 10.f)), 0.0001f);
}

TEST(RameligCore, processShouldKeepUpOneActionWithinBounds) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(1)
		.WillOnce(testing::Return(3.5f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::DOWN_ONE))
		.Times(1);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	// Guide the core once so we're at the upper edge
	rameligCore.guideLast(5.f);

	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(4, 6)), 0.0001f);
}

TEST(RameligCore, processShouldKeepUpTwoActionWithinBounds) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(1)
		.WillOnce(testing::Return(2.5f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::DOWN_TWO))
		.Times(1);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	// Guide the core once so we're at the upper edge
	rameligCore.guideLast(5.f);

	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(4, 5)), 0.0001f);
}

TEST(RameligCore, processShouldKeepDownOneActionWithinBounds) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(1)
		.WillOnce(testing::Return(4.5f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::UP_ONE))
		.Times(1);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	// Guide the core once so we're at the upper edge
	rameligCore.guideLast(-5.f);

	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(-5, 1)), 0.0001f);
}

TEST(RameligCore, processShouldKeepDownTwoActionWithinBounds) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 7.f))
		.Times(1)
		.WillOnce(testing::Return(5.5f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::UP_TWO))
		.Times(1);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	// Guide the core once so we're at the upper edge
	rameligCore.guideLast(-5.f);

	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(-5, 2)), 0.0001f);
}

TEST(RameligCore, processAfterRandomJumpShouldReturnToOriginalLocation) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance)
		.Times(0);
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(1)
		.WillOnce(testing::Return(2.5f));
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::RANDOM_JUMP))
		.Times(1);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::STAY))
		.Times(1);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	float result1 = rameligCore.process(data, true, false, false, -5.f, 5.f);
	EXPECT_NEAR(result1, rameligScale->quantizedToVoltage(rameligScale->quantize(2.5f, -10.f, 10.f)), 0.0001f);

	float result2 = rameligCore.process(data, false, false, true, -5.f, 5.f);
	EXPECT_EQ(0.f, result2);
}

TEST(RameligCore, processAfterRandomShiftShouldStayInNewLocation) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance)
		.Times(0);
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(1)
		.WillOnce(testing::Return(2.5f));
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::RANDOM_SHIFT))
		.Times(1);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::STAY))
		.Times(1);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	float result1 = rameligCore.process(data, false, true, false, -5.f, 5.f);
	EXPECT_NEAR(result1, rameligScale->quantizedToVoltage(rameligScale->quantize(2.5f, -10.f, 10.f)), 0.0001f);

	float result2 = rameligCore.process(data, false, false, true, -5.f, 5.f);
	EXPECT_EQ(result1, result2);
}

TEST(RameligCore, guideShouldInfluenceNextProcessForStay) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance)
		.Times(0);
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::STAY))
		.Times(1);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	rameligCore.guideLast(2.25f);

	float result = rameligCore.process(data, false, false, true, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(2.25f, -10.f, 10.f)), 0.0001f);
}

TEST(RameligCore, guideShouldInfluenceNextProcessForMovement) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance)
		.Times(4)
		.WillOnce(testing::Return(2.5))
		.WillOnce(testing::Return(3.5))
		.WillOnce(testing::Return(4.5))
		.WillOnce(testing::Return(5.5));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::UP_TWO))
		.Times(1);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::UP_ONE))
		.Times(1);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::DOWN_ONE))
		.Times(1);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::DOWN_TWO))
		.Times(1);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);

	rameligCore.guideLast(2.f);
	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(2, 2)), 0.0001f);

	rameligCore.guideLast(3.f);
	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(3, 1)), 0.0001f);

	rameligCore.guideLast(-2.f);
	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(-3, 6)), 0.0001f);

	rameligCore.guideLast(-3.f);
	result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(std::pair<int, int>(-4, 5)), 0.0001f);
}

TEST(RameligCore, guideShouldNotInfluenceProcessWithShiftAction) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance)
		.Times(0);
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(1)
		.WillOnce(testing::Return(4.25f));
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::RANDOM_SHIFT))
		.Times(1);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::STAY))
		.Times(1);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);

	rameligCore.guideLast(2.f);
	float result = rameligCore.process(data, false, true, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(4.25f, -5.f, 5.f)), 0.0001f);

	result = rameligCore.process(data, false, false, true, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(4.25f, -5.f, 5.f)), 0.0001f);
}

TEST(RameligCore, guideShouldNotInfluenceJumpActionInProcessButDoInfluenceReturnValueAfterJump) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f); // A distribution that results in an equal chacne (1.f) for each of the possible actions
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0, 2, 4, 5, 7, 9, 11 }); // A C major scale

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance)
		.Times(0);
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(1)
		.WillOnce(testing::Return(4.25f));
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::RANDOM_JUMP))
		.Times(1);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::STAY))
		.Times(1);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);

	rameligCore.guideLast(2.f);
	float result = rameligCore.process(data, true, false, false, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(4.25f, -5.f, 5.f)), 0.0001f);

	result = rameligCore.process(data, false, false, true, -5.f, 5.f);
	EXPECT_NEAR(result, rameligScale->quantizedToVoltage(rameligScale->quantize(2.f, -5.f, 5.f)), 0.0001f);
}

TEST(RameligCore, processShouldRequantizeAfterDistributionChange) {
	RameligDistributionData data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .5f);
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0 });

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance)
		.Times(0);
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(0);
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::STAY))
		.Times(3);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	rameligCore.guideLast(2.25f);
	float result1 = rameligCore.process(data, false, false, true, -5.f, 5.f);
	EXPECT_NEAR(result1, rameligScale->quantizedToVoltage(rameligScale->quantize(2.25f, -10.f, 10.f)), 0.0001f);

	rameligScale->setScale({ 5 });
	data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .6f);
	float result2 = rameligCore.process(data, false, false, true, -5.f, 5.f);
	EXPECT_NEAR(result2, rameligScale->quantizedToVoltage(rameligScale->quantize(result1, -10.f, 10.f)), 0.0001f);

	rameligScale->setScale({ 9 });
	data = populateDistributionData(1.f, 1.f, 2.f, 1.f, 2.f, .7f);
	float result3 = rameligCore.process(data, false, false, true, -5.f, 5.f);
	EXPECT_NEAR(result3, rameligScale->quantizedToVoltage(rameligScale->quantize(result2, -10.f, 10.f)), 0.0001f);
}

TEST(RameligCore, processShouldWorkWithZeroDistribution) {
	RameligDistributionData data = populateDistributionData(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	std::shared_ptr<RameligScale> rameligScale = std::make_shared<RameligScale>();
	std::shared_ptr<MockRameligChanceGenerator> mockRameligChanceGenerator = std::make_shared<MockRameligChanceGenerator>();
	MockRameligActionListener mockRameligActionListener;

	rameligScale->setScale({ 0 });

	EXPECT_CALL(*mockRameligChanceGenerator, generateActionChance(0.f, 0.f))
		.Times(1)
		.WillOnce(testing::Return(0.f));
	EXPECT_CALL(*mockRameligChanceGenerator, generateJumpChance)
		.Times(1)
		.WillOnce(testing::Return(3.f));
	EXPECT_CALL(mockRameligActionListener, rameligActionPerformed(42, RameligActions::RANDOM_JUMP))
		.Times(1);

	RameligCore rameligCore(42, rameligScale, &mockRameligActionListener, mockRameligChanceGenerator);
	rameligCore.guideLast(2.f);
	float result = rameligCore.process(data, false, false, false, -5.f, 5.f);
	EXPECT_EQ(result, 3.f);
}
