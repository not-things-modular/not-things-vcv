#include <gmock/gmock.h>
#include "core/ramelig-core.hpp"

TEST(RameligCore, calculateDistributionShouldConstructBalancedDistributon) {
    std::array<float, 7> distribution;
    RameligDistributionData data = {
        randomJumpChance: 1.f,
        randomShiftChance: 1.f,
        moveUpChance: 2.f,
        stayChance: 1.f,
        moveDownChance: 2.f,
        moveTwoFactor: 0.5f,
        stayRepeatFactor: 1.f,
    };

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
    RameligDistributionData data = {
        randomJumpChance: 1.234f,
        randomShiftChance: 1.f,
        moveUpChance: 2.f,
        stayChance: 1.f,
        moveDownChance: 2.f,
        moveTwoFactor: 0.5f,
        stayRepeatFactor: 1.f,
    };

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
    RameligDistributionData data = {
        randomJumpChance: 1.f,
        randomShiftChance: 1.234f,
        moveUpChance: 2.f,
        stayChance: 1.f,
        moveDownChance: 2.f,
        moveTwoFactor: 0.5f,
        stayRepeatFactor: 1.f,
    };

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
    RameligDistributionData data = {
        randomJumpChance: 1.f,
        randomShiftChance: 1.f,
        moveUpChance: 3.f,
        stayChance: 1.f,
        moveDownChance: 2.f,
        moveTwoFactor: 0.5f,
        stayRepeatFactor: 1.f,
    };

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
    RameligDistributionData data = {
        randomJumpChance: 1.f,
        randomShiftChance: 1.f,
        moveUpChance: 2.f,
        stayChance: 1.f,
        moveDownChance: 3.f,
        moveTwoFactor: 0.5f,
        stayRepeatFactor: 1.f,
    };

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
    RameligDistributionData data = {
        randomJumpChance: 1.f,
        randomShiftChance: 1.f,
        moveUpChance: 2.f,
        stayChance: 1.234f,
        moveDownChance: 2.f,
        moveTwoFactor: 0.5f,
        stayRepeatFactor: 1.f,
    };

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
    RameligDistributionData data = {
        randomJumpChance: 1.f,
        randomShiftChance: 1.f,
        moveUpChance: 2.f,
        stayChance: 1.f,
        moveDownChance: 2.f,
        moveTwoFactor: 0.f,
        stayRepeatFactor: 1.f,
    };

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
    RameligDistributionData data = {
        randomJumpChance: 1.f,
        randomShiftChance: 1.f,
        moveUpChance: 2.f,
        stayChance: 1.f,
        moveDownChance: 2.f,
        moveTwoFactor: 1.f,
        stayRepeatFactor: 1.f,
    };

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
    RameligDistributionData data = {
        randomJumpChance: 1.f,
        randomShiftChance: 1.f,
        moveUpChance: 2.f,
        stayChance: 1.f,
        moveDownChance: 2.f,
        moveTwoFactor: 0.75f,
        stayRepeatFactor: 1.f,
    };

    RameligCore(0, nullptr, nullptr).calculateDistribution(data, distribution);

    EXPECT_NEAR(distribution[RameligActions::RANDOM_JUMP], 1.f, 0.00001f);
    EXPECT_NEAR(distribution[RameligActions::RANDOM_SHIFT], 2.f, 0.00001f);
    EXPECT_NEAR(distribution[RameligActions::UP_TWO], 3.5f, 0.00001f);
    EXPECT_NEAR(distribution[RameligActions::UP_ONE], 4.f, 0.00001f);
    EXPECT_NEAR(distribution[RameligActions::DOWN_ONE], 4.5f, 0.00001f);
    EXPECT_NEAR(distribution[RameligActions::DOWN_TWO], 6.f, 0.00001f);
    EXPECT_NEAR(distribution[RameligActions::STAY], 7.f, 0.00001f);
}
