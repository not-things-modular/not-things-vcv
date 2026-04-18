#include <gmock/gmock.h>
#include "core/ramelig-core.hpp"

RameligDistributionData getDistributionData() {
    RameligDistributionData rdd;

    rdd = {
        randomJumpChance: 6.9,
        randomShiftChance: 4.20,

        moveUpChance: 3.14,
        stayChance: 9.6,
        moveDownChance: 10.2,

        moveTwoFactor: 1.23,
        stayRepeatFactor: 3.21
    };

    return rdd;
}

TEST(RameligCoreDistributionData, ShouldCompareCorrectlyForIdenticalInstances) {
    RameligDistributionData rdd1 = getDistributionData();
    RameligDistributionData rdd2 = getDistributionData();

    EXPECT_EQ(rdd1, rdd2);
    EXPECT_TRUE(rdd1 == rdd2);
    EXPECT_FALSE(rdd1 != rdd2);
}

TEST(RameligCoreDistributionData, ShouldCompareCorrectlyForDifferentInstances) {
    RameligDistributionData rdd1 = getDistributionData();
    RameligDistributionData rdd2 = getDistributionData();

    rdd1.randomJumpChance = rdd1.randomJumpChance + 1;
    EXPECT_NE(rdd1, rdd2);
    EXPECT_FALSE(rdd1 == rdd2);
    EXPECT_TRUE(rdd1 != rdd2);
    rdd1.randomJumpChance = rdd2.randomJumpChance;

    rdd1.randomShiftChance = rdd1.randomShiftChance + 1;
    EXPECT_NE(rdd1, rdd2);
    EXPECT_FALSE(rdd1 == rdd2);
    EXPECT_TRUE(rdd1 != rdd2);
    rdd1.randomShiftChance = rdd2.randomShiftChance;

    rdd1.moveUpChance = rdd1.moveUpChance + 1;
    EXPECT_NE(rdd1, rdd2);
    EXPECT_FALSE(rdd1 == rdd2);
    EXPECT_TRUE(rdd1 != rdd2);
    rdd1.moveUpChance = rdd2.moveUpChance;

    rdd1.stayChance = rdd1.stayChance + 1;
    EXPECT_NE(rdd1, rdd2);
    EXPECT_FALSE(rdd1 == rdd2);
    EXPECT_TRUE(rdd1 != rdd2);
    rdd1.stayChance = rdd2.stayChance;

    rdd1.moveDownChance = rdd1.moveDownChance + 1;
    EXPECT_NE(rdd1, rdd2);
    EXPECT_FALSE(rdd1 == rdd2);
    EXPECT_TRUE(rdd1 != rdd2);
    rdd1.moveDownChance = rdd2.moveDownChance;

    rdd1.moveTwoFactor = rdd1.moveTwoFactor + 1;
    EXPECT_NE(rdd1, rdd2);
    EXPECT_FALSE(rdd1 == rdd2);
    EXPECT_TRUE(rdd1 != rdd2);
    rdd1.moveTwoFactor = rdd2.moveTwoFactor;

    rdd1.stayRepeatFactor = rdd1.stayRepeatFactor + 1;
    EXPECT_NE(rdd1, rdd2);
    EXPECT_FALSE(rdd1 == rdd2);
    EXPECT_TRUE(rdd1 != rdd2);
    rdd1.stayRepeatFactor = rdd2.stayRepeatFactor;
}
