#include <gmock/gmock.h>
#include "core/ramelig-core.hpp"

RameligDistributionData populateDistributionData(float randomJumpChance, float randomShiftChance, float moveUpChance, float stayChance, float moveDownChance, float moveTwoFactor) {
    RameligDistributionData data;

    data.randomJumpChance = randomJumpChance;
    data.randomShiftChance = randomShiftChance;
    data.moveUpChance = moveUpChance;
    data.stayChance = stayChance;
    data.moveDownChance = moveDownChance;
    data.moveTwoFactor = moveTwoFactor;
    data.stayRepeatFactor = 1.23f;

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
