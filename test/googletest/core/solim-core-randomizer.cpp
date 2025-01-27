#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core/solim-core.hpp"


const std::array<int, 16> unrandomizedIndices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

// Allow mocking of the randomization
class MockMt19937 {
    public:
        MockMt19937(std::vector<uint_fast32_t> values) : m_values(values) {
        }

        uint_fast32_t operator()() {
            uint_fast32_t result = m_values[m_idx];
            m_idx = (m_idx + 1) % m_values.size();
            return result;
        }

    private:
        std::vector<uint_fast32_t> m_values;
        int m_idx = 0;
};

std::array<RandomTrigger, 8> getRandomTriggerArray(RandomTrigger randomTrigger) {
    std::array<RandomTrigger, 8> randomTriggers = {
        randomTrigger, randomTrigger, randomTrigger, randomTrigger, randomTrigger, randomTrigger, randomTrigger, randomTrigger
    };
    return randomTriggers;
}

std::array<SolimValueSet, 8> getValueSetArray(int outputValueCount) {
    std::array<SolimValueSet, 8> valueSet;
    for (int i = 0; i < 8; i++) {
        valueSet[i].outputValueCount = outputValueCount;
    }
    return valueSet;
}

TEST(SolimCoreRandomizerTest, FirstRandomProcessShouldDoNothing) {
    SolimCoreRandomizer randomizer;

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ALL);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, unrandomizedIndices);
    }
}

TEST(SolimCoreRandomizerTest, SecondRandomProcessWithoutTirggerShoulDoNothing) {
    SolimCoreRandomizer randomizer;

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ALL);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    randomTriggers = getRandomTriggerArray(RandomTrigger::NONE);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, unrandomizedIndices);
    }
}

TEST(SolimCoreRandomizerTest, SecondRandomProcessWithTriggerShoulRandomizeAll) {
    SolimCoreRandomizer randomizer;

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ALL);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    for (int i = 0; i < 8; i++) {
        // They should be randomized, so not equal
        EXPECT_NE(valueSet[i].indices, unrandomizedIndices);
        // But contain the same elements
        EXPECT_THAT(valueSet[i].indices, testing::UnorderedElementsAreArray(unrandomizedIndices));
    }
}

TEST(SolimCoreRandomizerTest, SecondRandomProcessWithTriggerShouldRandomizeOnlyActiveColumns) {
    SolimCoreRandomizer randomizer;

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ALL);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    randomizer.process(4, &randomTriggers, valueSet, valueSet);

    // The first four columns should have been randomized
    for (int i = 0; i < 4; i++) {
        // They should be randomized, so not equal
        EXPECT_NE(valueSet[i].indices, unrandomizedIndices);
        // But contain the same elements
        EXPECT_THAT(valueSet[i].indices, testing::UnorderedElementsAreArray(unrandomizedIndices));
    }

    // The rest should remain as-is
    for (int i = 4; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, unrandomizedIndices);
    }
}

TEST(SolimCoreRandomizerTest, SecondRandomProcessShouldOnlyRandomizeColumsWithTriggers) {
    SolimCoreRandomizer randomizer;

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::NONE);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    randomTriggers[2] = randomTriggers[4] = randomTriggers[6] = RandomTrigger::ALL;
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    for (int i = 0; i < 8; i++) {
        if (i == 2 || i == 4 || i == 6) {
            // Columns 2, 4 and 6 should have been randomized
            EXPECT_NE(valueSet[i].indices, unrandomizedIndices);
            EXPECT_THAT(valueSet[i].indices, testing::UnorderedElementsAreArray(unrandomizedIndices));
        } else {
            // The rest should remain as-is
            EXPECT_EQ(valueSet[i].indices, unrandomizedIndices);
        }
    }
}

TEST(SolimCoreRandomizerTest, NoneActionAfterAllRandomizeShouldDoNothing) {
    SolimCoreRandomizer randomizer;

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ALL);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    std::array<std::array<int, 16>, 8> randomizedValueSets;
    for (int i = 0; i < 8; i++) {
        randomizedValueSets[i] = valueSet[i].indices;
    }

    randomTriggers = getRandomTriggerArray(RandomTrigger::NONE);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, randomizedValueSets[i]);
    }
}

TEST(SolimCoreRandomizerTest, AllRandomizeActionAfterAllRandomizeShouldRandomizeAgain) {
    SolimCoreRandomizer randomizer;

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ALL);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    std::array<std::array<int, 16>, 8> randomizedValueSets;
    for (int i = 0; i < 8; i++) {
        randomizedValueSets[i] = valueSet[i].indices;
    }

    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    for (int i = 0; i < 8; i++) {
        EXPECT_NE(valueSet[i].indices, randomizedValueSets[i]);
            EXPECT_THAT(valueSet[i].indices, testing::UnorderedElementsAreArray(unrandomizedIndices));
    }
}

TEST(SolimCoreRandomizerTest, NoRandomToRandomAndBackShouldResetAndRandomAgain) {
    SolimCoreRandomizer randomizer;

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ALL);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    // Everything should become randomized
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    for (int i = 0; i < 8; i++) {
        EXPECT_NE(valueSet[i].indices, unrandomizedIndices);
        EXPECT_THAT(valueSet[i].indices, testing::UnorderedElementsAreArray(unrandomizedIndices));
    }

    // Remove randomization
    randomizer.process(8, nullptr, valueSet, valueSet);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, unrandomizedIndices);
    }

    // First run of re-randomization (i.e. no action yet)
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, unrandomizedIndices);
    }

    // Second run of re-randomization (i.e. randomize)
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    for (int i = 0; i < 8; i++) {
        EXPECT_NE(valueSet[i].indices, unrandomizedIndices);
        EXPECT_THAT(valueSet[i].indices, testing::UnorderedElementsAreArray(unrandomizedIndices));
    }
}

TEST(SolimCoreRandomizerTest, ResetTriggerShouldResetIndices) {
    SolimCoreRandomizer randomizer;

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ALL);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    randomTriggers = getRandomTriggerArray(RandomTrigger::NONE);
    randomTriggers[2] = randomTriggers[4] = randomTriggers[6] = RandomTrigger::RESET;
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    for (int i = 0; i < 8; i++) {
        if (i == 2 || i == 4 || i == 6) {
            // Columns 2, 4 and 6 should have been reset
            EXPECT_EQ(valueSet[i].indices, unrandomizedIndices);
        } else {
            // The rest should remain as-is
            // They should be randomized, so not equal
            EXPECT_NE(valueSet[i].indices, unrandomizedIndices);
            // But contain the same elements
            EXPECT_THAT(valueSet[i].indices, testing::UnorderedElementsAreArray(unrandomizedIndices));
        }
    }
}

TEST(SolimCoreRandomizerTest, ReducingOutputValueCountShouldRestoreLastIndices) {
    SolimCoreRandomizer randomizer;

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ALL);
    std::array<SolimValueSet, 8> oldValueSet = getValueSetArray(16);
    std::array<SolimValueSet, 8> newValueSet = getValueSetArray(16);
    for (int i = 0; i < 8; i++) {
        oldValueSet[i].outputValueCount = 8;
        newValueSet[i].outputValueCount = 8;
    }
    // Make sure both the oldValueSet and newValueSet are fully randomized
    randomizer.process(8, &randomTriggers, oldValueSet, newValueSet);
    randomizer.process(8, &randomTriggers, oldValueSet, newValueSet);
    randomizer.process(8, &randomTriggers, newValueSet, oldValueSet);
    randomizer.process(8, &randomTriggers, newValueSet, oldValueSet);

    for (int i = 0; i < 8; i++) {
        newValueSet[i].outputValueCount = i + 4;
    }
    randomTriggers = getRandomTriggerArray(RandomTrigger::NONE);
    randomizer.process(8, &randomTriggers, oldValueSet, newValueSet);

    for (int i = 0; i < 8; i++) {
        for (int j = i + 4; j < 16; j++) {
            EXPECT_EQ(newValueSet[i].indices[j], j);
        }
    }
}

TEST(SolimCoreRandomizerTest, RemoveAndAddColumnShouldResetNewColumn) {
    SolimCoreRandomizer randomizer;

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ALL);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    randomTriggers = getRandomTriggerArray(RandomTrigger::NONE);
    randomizer.process(6, &randomTriggers, valueSet, valueSet);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    // The first 6 columns should still be randomized
    for (int i = 0; i < 6; i++) {
        EXPECT_NE(valueSet[i].indices, unrandomizedIndices);
        EXPECT_THAT(valueSet[i].indices, testing::UnorderedElementsAreArray(unrandomizedIndices));
    }

    // The last two columns were re-added, so they should have been reset
    for (int i = 6; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, unrandomizedIndices);
    }
}

TEST(SolimCoreRandomizerTest, RandomizeOneShouldMoveOneToRandomPlace) {
    MockMt19937 mockMt19937({ 5, 8, 4, 7 }); // alternatingly will switch 5 with 8 and 4 with 7
    SolimCoreRandomizer randomizer(0, 100, mockMt19937);

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ONE);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    std::array<int, 16> fiveAndEight = { 0, 1, 2, 3, 4, 8, 6, 7, 5, 9, 10, 11, 12, 13, 14, 15 };
    std::array<int, 16> fourAndSeven = { 0, 1, 2, 3, 7, 5, 6, 4, 8, 9, 10, 11, 12, 13, 14, 15 };
    for (int i = 0; i < 8; i += 2) {
        EXPECT_EQ(valueSet[i].indices, fiveAndEight);
        EXPECT_EQ(valueSet[i + 1].indices, fourAndSeven);
    }
}

TEST(SolimCoreRandomizerTest, RandomizeOneWithTwoElementsShouldSwap) {
    SolimCoreRandomizer randomizer;
    std::array<int, 16> swappedIndices = { 1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ONE);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(2);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    // A first move should swap the first two indices
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, swappedIndices);
    }

    // A second move should swap them back
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, unrandomizedIndices);
    }

    // A thrid move should swap them again
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, swappedIndices);
    }
}

TEST(SolimCoreRandomizerTest, RandomizeOneShouldNotAllowMovingToSamePlace) {
    MockMt19937 mockMt19937({ 5, 5, 8, 4, 4, 7 }); // alternatingly will switch 5 with 8 and 4 with 7
    SolimCoreRandomizer randomizer(0, 100, mockMt19937);

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::ONE);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    std::array<int, 16> fiveAndEight = { 0, 1, 2, 3, 4, 8, 6, 7, 5, 9, 10, 11, 12, 13, 14, 15 };
    std::array<int, 16> fourAndSeven = { 0, 1, 2, 3, 7, 5, 6, 4, 8, 9, 10, 11, 12, 13, 14, 15 };
    for (int i = 0; i < 8; i += 2) {
        EXPECT_EQ(valueSet[i].indices, fiveAndEight);
        EXPECT_EQ(valueSet[i + 1].indices, fourAndSeven);
    }
}

TEST(SolimCoreRandomizerTest, MoveOneShouldMoveItemsUpAndDown) {
    MockMt19937 mockMt19937({ 5, 0, 6, 2, 7, 1, 8, 3 }); // alternatingly will switch 5 with 8 and 4 with 7
    SolimCoreRandomizer randomizer(0, 100, mockMt19937);

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::MOVE);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    std::array<int, 16> fiveDown = { 0, 1, 2, 3, 5, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    std::array<int, 16> SixDown = { 0, 1, 2, 3, 4, 6, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    std::array<int, 16> SevenUp = { 0, 1, 2, 3, 4, 5, 6, 8, 7, 9, 10, 11, 12, 13, 14, 15 };
    std::array<int, 16> EightUp = { 0, 1, 2, 3, 4, 5, 6, 7, 9, 8, 10, 11, 12, 13, 14, 15 };

    EXPECT_EQ(valueSet[0].indices, fiveDown);
    EXPECT_EQ(valueSet[1].indices, SixDown);
    EXPECT_EQ(valueSet[2].indices, SevenUp);
    EXPECT_EQ(valueSet[3].indices, EightUp);
    EXPECT_EQ(valueSet[4].indices, fiveDown);
    EXPECT_EQ(valueSet[5].indices, SixDown);
    EXPECT_EQ(valueSet[6].indices, SevenUp);
    EXPECT_EQ(valueSet[7].indices, EightUp);
}

TEST(SolimCoreRandomizerTest, MoveOneShouldWrapAround) {
    MockMt19937 mockMt19937({ 0, 0, 15, 1, 0, 2, 7, 3 }); // Move 0 down, 15 up, 0 down, 7 up
    SolimCoreRandomizer randomizer(0, 100, mockMt19937);

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::MOVE);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(16);
    // Set some columns to eight ouput values to check that wrap around occurs based on output value size
    valueSet[2].outputValueCount = valueSet[3].outputValueCount = valueSet[6].outputValueCount = valueSet[7].outputValueCount = 8;
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    std::array<int, 16> wrap16 = { 15, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0 };
    std::array<int, 16> wrap8 = { 7, 1, 2, 3, 4, 5, 6, 0, 8, 9, 10, 11, 12, 13, 14, 15 };

    EXPECT_EQ(valueSet[0].indices, wrap16);
    EXPECT_EQ(valueSet[1].indices, wrap16);
    EXPECT_EQ(valueSet[2].indices, wrap8);
    EXPECT_EQ(valueSet[3].indices, wrap8);
    EXPECT_EQ(valueSet[4].indices, wrap16);
    EXPECT_EQ(valueSet[5].indices, wrap16);
    EXPECT_EQ(valueSet[6].indices, wrap8);
    EXPECT_EQ(valueSet[7].indices, wrap8);
}

TEST(SolimCoreRandomizerTest, MoveWithTwoElementsShouldSwap) {
    SolimCoreRandomizer randomizer;
    std::array<int, 16> swappedIndices = { 1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    std::array<RandomTrigger, 8> randomTriggers = getRandomTriggerArray(RandomTrigger::MOVE);
    std::array<SolimValueSet, 8> valueSet = getValueSetArray(2);
    randomizer.process(8, &randomTriggers, valueSet, valueSet);

    // A first move should swap the first two indices
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, swappedIndices);
    }

    // A second move should swap them back
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, unrandomizedIndices);
    }

    // A thrid move should swap them again
    randomizer.process(8, &randomTriggers, valueSet, valueSet);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(valueSet[i].indices, swappedIndices);
    }
}
