#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core/solim-core.hpp"


struct MockSolimCoreProcessor : SolimCoreProcessor {
	MOCK_METHOD(void, processValues, (SolimValueSet&), (override));
	MOCK_METHOD(void, processResults, (SolimValueSet&), (override));
};

using RandomTiggerArray = std::array<RandomTrigger, 8>;
using SolimValueSetArray = std::array<SolimValueSet, 8>;
using IntArray = std::array<int, 16>;
struct MockSolimCoreRandomizer : SolimCoreRandomizer {
	MOCK_METHOD(void, process, (int, RandomTiggerArray*, SolimValueSetArray&, SolimValueSetArray&), (override));
};

TEST(SolimCoreTest, ShouldConstructWorkingNonMockedInstance) {
	std::array<SolimValueSet, 8> valueSet;
	SolimCore solimCore;

	solimCore.processAndActivateInactiveValues(8, nullptr);
}

TEST(SolimCoreTest, ProcessShouldNotCallProcessorOnEqualInputAndResultValuesOnAllColumns) {
	std::array<SolimValueSet, 8> valueSet;
	MockSolimCoreProcessor* processor = new MockSolimCoreProcessor();
	MockSolimCoreRandomizer* randomizer = new MockSolimCoreRandomizer();

	SolimCore solimCore(processor, randomizer);
	std::array<RandomTrigger, 8> randomTriggers;

	for (int column = 0; column < 8; column++) {
		// Set both valueSets to 0 input length so they are considered equal
		// Set the outputValueCount and resultValueCount to verify that they do get updated
		// Assign outputValues and resultValues to verify they are updated
		solimCore.getActiveValues(column).inputValueCount = 0;
		solimCore.getActiveValues(column).outputValueCount = 8;
		solimCore.getActiveValues(column).resultValueCount = 4;
		solimCore.getInactiveValues(column).inputValueCount = 0;
		solimCore.getInactiveValues(column).outputValueCount = 1;
		solimCore.getInactiveValues(column).resultValueCount = 2;

		for (int i = 0; i < 16; i++) {
			solimCore.getActiveValues(column).outputValues[i].value = i + column;
			solimCore.getInactiveValues(column).outputValues[i].value = -i - column;
		}

		for (int i = 0; i < 8; i++) {
			solimCore.getActiveValues(column).resultValues[i] = i + column;
			solimCore.getInactiveValues(column).resultValues[i] = -i - column;
		}
	}

	// The processor should not have been called at all
	EXPECT_CALL(*processor, processValues).Times(0);
	EXPECT_CALL(*processor, processResults).Times(0);
	// The randomzier should be called
	EXPECT_CALL(*randomizer, process).Times(1);

	// Execute the functionality
	solimCore.processAndActivateInactiveValues(8, &randomTriggers);

	for (int column = 0; column < 8; column++) {
		// Both activeValues and inactiveValues should now have the same outputValueCount, outputValues resultValueCount and resultValues
		// (the ones from the old active values)
		EXPECT_EQ(solimCore.getActiveValues(column).outputValueCount, 8);
		EXPECT_EQ(solimCore.getInactiveValues(column).outputValueCount, 8);
		EXPECT_EQ(solimCore.getActiveValues(column).resultValueCount, 4);
		EXPECT_EQ(solimCore.getInactiveValues(column).resultValueCount, 4);

		// The old active values had 8 outputValues, so only the first 8 values should have been copied over
		for (int i = 0; i < 8; i++) {
			EXPECT_NEAR(solimCore.getActiveValues(column).outputValues[i].value, i + column, 0.0001f);
			EXPECT_NEAR(solimCore.getInactiveValues(column).outputValues[i].value, i + column, 0.0001f);
		}
		// The next 8 outputValues should remain untouched
		for (int i = 8; i < 16; i++) {
			EXPECT_NEAR(solimCore.getActiveValues(column).outputValues[i].value, -i - column, 0.0001f);
			EXPECT_NEAR(solimCore.getInactiveValues(column).outputValues[i].value, i + column, 0.0001f);
		}

		// The old active values had 4 resultValues, so only the first 4 values should have been copied over
		for (int i = 0; i < 4; i++) {
			EXPECT_NEAR(solimCore.getActiveValues(column).resultValues[i], i + column, 0.0001f);
			EXPECT_NEAR(solimCore.getInactiveValues(column).resultValues[i], i + column, 0.0001f);
		}
		// The next 8 outputValues should remain untouched
		for (int i = 4; i < 8; i++) {
			EXPECT_NEAR(solimCore.getActiveValues(column).resultValues[i], -i - column, 0.0001f);
			EXPECT_NEAR(solimCore.getInactiveValues(column).resultValues[i], i + column, 0.0001f);
		}
	}
}

TEST(SolimCoreTest, ProcessShouldNotCallProcessorOnEqualInputAndResultValuesOnSingleColumn) {
	std::array<SolimValueSet, 8> valueSet;
	MockSolimCoreProcessor* processor = new MockSolimCoreProcessor();
	MockSolimCoreRandomizer* randomizer = new MockSolimCoreRandomizer();

	SolimCore solimCore(processor, randomizer);
	std::array<RandomTrigger, 8> randomTriggers;

	for (int column = 0; column < 8; column++) {
		// Set both valueSets to 0 input length so they are considered equal
		// Set the outputValueCount and resultValueCount to verify that they do get updated
		// Assign outputValues and resultValues to verify they are updated
		solimCore.getActiveValues(column).inputValueCount = 0;
		solimCore.getActiveValues(column).outputValueCount = 8;
		solimCore.getActiveValues(column).resultValueCount = 4;
		solimCore.getInactiveValues(column).inputValueCount = 0;
		solimCore.getInactiveValues(column).outputValueCount = 1;
		solimCore.getInactiveValues(column).resultValueCount = 2;

		for (int i = 0; i < 16; i++) {
			solimCore.getActiveValues(column).outputValues[i].value = i + column;
			solimCore.getInactiveValues(column).outputValues[i].value = -i - column;
		}

		for (int i = 0; i < 8; i++) {
			solimCore.getActiveValues(column).resultValues[i] = i + column;
			solimCore.getInactiveValues(column).resultValues[i] = -i - column;
		}
	}

	// The processor should not have been called at all
	EXPECT_CALL(*processor, processValues).Times(0);
	EXPECT_CALL(*processor, processResults).Times(0);
	// The randomzier should be called
	EXPECT_CALL(*randomizer, process).Times(1);

	// Execute the functionality
	solimCore.processAndActivateInactiveValues(1, &randomTriggers);

	 /* Only the first column should have been processed */
	EXPECT_EQ(solimCore.getActiveValues(0).outputValueCount, 8);
	EXPECT_EQ(solimCore.getInactiveValues(0).outputValueCount, 8);
	EXPECT_EQ(solimCore.getActiveValues(0).resultValueCount, 4);
	EXPECT_EQ(solimCore.getInactiveValues(0).resultValueCount, 4);

	// The old active values had 8 outputValues, so only the first 8 values should have been copied over
	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(solimCore.getActiveValues(0).outputValues[i].value, i, 0.0001f);
		EXPECT_NEAR(solimCore.getInactiveValues(0).outputValues[i].value, i, 0.0001f);
	}
	// The next 8 outputValues should remain untouched
	for (int i = 8; i < 16; i++) {
		EXPECT_NEAR(solimCore.getActiveValues(0).outputValues[i].value, -i, 0.0001f);
		EXPECT_NEAR(solimCore.getInactiveValues(0).outputValues[i].value, i, 0.0001f);
	}

	// The old active values had 4 resultValues, so only the first 4 values should have been copied over
	for (int i = 0; i < 4; i++) {
		EXPECT_NEAR(solimCore.getActiveValues(0).resultValues[i], i, 0.0001f);
		EXPECT_NEAR(solimCore.getInactiveValues(0).resultValues[i], i, 0.0001f);
	}
	// The next 8 outputValues should remain untouched
	for (int i = 4; i < 8; i++) {
		EXPECT_NEAR(solimCore.getActiveValues(0).resultValues[i], -i, 0.0001f);
		EXPECT_NEAR(solimCore.getInactiveValues(0).resultValues[i], i, 0.0001f);
	}

	/* The other columns should have remained as-is */
	for (int column = 1; column < 8; column++) {
		// Both activeValues and inactiveValues should now have the same outputValueCount, outputValues resultValueCount and resultValues
		// (the ones from the old active values)
		EXPECT_EQ(solimCore.getActiveValues(column).outputValueCount, 1);
		EXPECT_EQ(solimCore.getInactiveValues(column).outputValueCount, 8);
		EXPECT_EQ(solimCore.getActiveValues(column).resultValueCount, 2);
		EXPECT_EQ(solimCore.getInactiveValues(column).resultValueCount, 4);

		for (int i = 0; i < 16; i++) {
			EXPECT_NEAR(solimCore.getActiveValues(column).outputValues[i].value, -i - column, 0.0001f);
			EXPECT_NEAR(solimCore.getInactiveValues(column).outputValues[i].value, i + column, 0.0001f);
		}
		for (int i = 0; i < 8; i++) {
			EXPECT_NEAR(solimCore.getActiveValues(column).resultValues[i], -i - column, 0.0001f);
			EXPECT_NEAR(solimCore.getInactiveValues(column).resultValues[i], i + column, 0.0001f);
		}
	}
}

TEST(SolimCoreTest, WhenProcessingAllColumnsShouldCallProcessorOnMismatchingInputValueColumns) {
	std::array<SolimValueSet, 8> valueSet;
	MockSolimCoreProcessor* processor = new MockSolimCoreProcessor();
	MockSolimCoreRandomizer* randomizer = new MockSolimCoreRandomizer();

	SolimCore solimCore(processor, randomizer);
	std::array<RandomTrigger, 8> randomTriggers;

	// First set all 8 columns to not contain any differences
	for (int column = 0; column < 8; column++) {
		// Set both valueSets to 0 input length so they are considered equal
		// Set the outputValueCount to verify that it does get updated
		// Assign outputValues to verify they are 
		solimCore.getActiveValues(column).inputValueCount = 0;
		solimCore.getActiveValues(column).outputValueCount = 8;
		solimCore.getInactiveValues(column).inputValueCount = 0;
		solimCore.getInactiveValues(column).outputValueCount = 1;

		for (int i = 0; i < 8; i++) {
			solimCore.getActiveValues(column).outputValues[i].value = i + column;
			solimCore.getInactiveValues(column).outputValues[i].value = i + column;
		}
		for (int i = 8; i < 16; i++) {
			solimCore.getActiveValues(column).outputValues[i].value = i + column;
			solimCore.getInactiveValues(column).outputValues[i].value = -i - column;
		}
	}
	// Now make the second column and the fifth column have different values and output values
	solimCore.getInactiveValues(1).inputValueCount = 1;
	solimCore.getInactiveValues(4).inputValueCount = 2;
	for (int i = 0; i < 16; i++) {
		solimCore.getInactiveValues(1).outputValues[i].value = -i - 1;
		solimCore.getInactiveValues(4).outputValues[i].value = -i - 4;
	}

	// The processor should be called for the two mismatching columns
	EXPECT_CALL(*processor, processValues(testing::Address(&solimCore.getInactiveValues(1))))
		.Times(1)
		.WillOnce(testing::Invoke([] (SolimValueSet& solimValueSet) {
			solimValueSet.outputValueCount = 8;
			for (int i = 0; i < 8; i++) {
				solimValueSet.outputValues[i].value = i + 1;
			}
		}));
	EXPECT_CALL(*processor, processValues(testing::Address(&solimCore.getInactiveValues(4))))
		.Times(1)
		.WillOnce(testing::Invoke([] (SolimValueSet& solimValueSet) {
			solimValueSet.outputValueCount = 8;
			for (int i = 0; i < 8; i++) {
				solimValueSet.outputValues[i].value = i + 4;
			}
		}));
	// The randomzier should be called
	EXPECT_CALL(*randomizer, process).Times(1);
	// The processor should not be called for the result values
	EXPECT_CALL(*processor, processResults).Times(0);


	// Execute the functionality
	solimCore.processAndActivateInactiveValues(8, &randomTriggers);

	for (int column = 0; column < 8; column++) {
		// Both activeValues and inactiveValues should now have the same outputValueCount and outputValues (the ones from the old active values)
		EXPECT_EQ(solimCore.getActiveValues(column).outputValueCount, 8);
		EXPECT_EQ(solimCore.getInactiveValues(column).outputValueCount, 8);

		// The old active values had 8 outputValues, so only the first 8 values should have been copied over
		for (int i = 0; i < 8; i++) {
			EXPECT_NEAR(solimCore.getActiveValues(column).outputValues[i].value, i + column, 0.0001f);
			EXPECT_NEAR(solimCore.getInactiveValues(column).outputValues[i].value, i + column, 0.0001f);
		}
		// The next 8 outputValues should remain untouched
		for (int i = 8; i < 16; i++) {
			EXPECT_NEAR(solimCore.getActiveValues(column).outputValues[i].value, -i - column, 0.0001f);
			EXPECT_NEAR(solimCore.getInactiveValues(column).outputValues[i].value, i + column, 0.0001f);
		}
	}
}

TEST(SolimCoreTest, WhenProcessingAllColumnsShouldCallProcessorOnMismatchingInputValueColumnsWithinSubset) {
	std::array<SolimValueSet, 8> valueSet;
	MockSolimCoreProcessor* processor = new MockSolimCoreProcessor();
	MockSolimCoreRandomizer* randomizer = new MockSolimCoreRandomizer();

	SolimCore solimCore(processor, randomizer);
	std::array<RandomTrigger, 8> randomTriggers;

	// First set all 8 columns to not contain any differences
	for (int column = 0; column < 8; column++) {
		// Set both valueSets to 0 input length so they are considered equal
		// Set the outputValueCount to verify that it does get updated
		// Assign outputValues to verify they are 
		solimCore.getActiveValues(column).inputValueCount = 0;
		solimCore.getActiveValues(column).outputValueCount = 8;
		solimCore.getInactiveValues(column).inputValueCount = 0;
		solimCore.getInactiveValues(column).outputValueCount = 1;

		for (int i = 0; i < 8; i++) {
			solimCore.getActiveValues(column).outputValues[i].value = i + column;
			solimCore.getInactiveValues(column).outputValues[i].value = column < 4 ? i + column : -i - column;
		}
		for (int i = 8; i < 16; i++) {
			solimCore.getActiveValues(column).outputValues[i].value = i + column;
			solimCore.getInactiveValues(column).outputValues[i].value = -i - column;
		}
	}
	// Now make the second column and the fifth column have different values and output values
	solimCore.getInactiveValues(1).inputValueCount = 1;
	solimCore.getInactiveValues(4).inputValueCount = 2;
	for (int i = 0; i < 16; i++) {
		solimCore.getInactiveValues(1).outputValues[i].value = -i - 1;
	}

	// The processor should be called for the two mismatching columns
	EXPECT_CALL(*processor, processValues(testing::Address(&solimCore.getInactiveValues(1))))
		.Times(1)
		.WillOnce(testing::Invoke([] (SolimValueSet& solimValueSet) {
			solimValueSet.outputValueCount = 8;
			for (int i = 0; i < 8; i++) {
				solimValueSet.outputValues[i].value = i + 1;
			}
		}));
	// The randomzier should be called
	EXPECT_CALL(*randomizer, process).Times(1);
	// The processor should not be called for the result values
	EXPECT_CALL(*processor, processResults).Times(0);


	// Execute the functionality
	solimCore.processAndActivateInactiveValues(4, &randomTriggers);

	for (int column = 0; column < 8; column++) {
		if (column < 4) {
			// Both activeValues and inactiveValues should now have the same outputValueCount and outputValues (the ones from the old active values)
			EXPECT_EQ(solimCore.getActiveValues(column).outputValueCount, 8);
			EXPECT_EQ(solimCore.getInactiveValues(column).outputValueCount, 8);

			// The old active values had 8 outputValues, so only the first 8 values should have been copied over
			for (int i = 0; i < 8; i++) {
				EXPECT_NEAR(solimCore.getActiveValues(column).outputValues[i].value, i + column, 0.0001f);
				EXPECT_NEAR(solimCore.getInactiveValues(column).outputValues[i].value, i + column, 0.0001f);
			}
			// The next 8 outputValues should remain untouched
			for (int i = 8; i < 16; i++) {
				EXPECT_NEAR(solimCore.getActiveValues(column).outputValues[i].value, -i - column, 0.0001f);
				EXPECT_NEAR(solimCore.getInactiveValues(column).outputValues[i].value, i + column, 0.0001f);
			}
		} else {
			// The other columns are outside of the processing range, so should not have been touched, only switched over.
			EXPECT_EQ(solimCore.getActiveValues(column).outputValueCount, 1);
			EXPECT_EQ(solimCore.getInactiveValues(column).outputValueCount, 8);

			for (int i = 0; i < 8; i++) {
				EXPECT_NEAR(solimCore.getActiveValues(column).outputValues[i].value, -i - column, 0.0001f);
				EXPECT_NEAR(solimCore.getInactiveValues(column).outputValues[i].value, i + column, 0.0001f);
			}
		}
	}
}

TEST(SolimCoreTest, WhenProcessingAllColumnsShouldCallProcessorOnMismatchingOutputValueColumns) {
	std::array<SolimValueSet, 8> valueSet;
	MockSolimCoreProcessor* processor = new MockSolimCoreProcessor();
	MockSolimCoreRandomizer* randomizer = new MockSolimCoreRandomizer();

	SolimCore solimCore(processor, randomizer);
	std::array<RandomTrigger, 8> randomTriggers;

	// First set all 8 columns to not contain any differences
	for (int column = 0; column < 8; column++) {
		// Set both valueSets to 0 input length so they are considered equal
		// Assign result values to verify they are copied over 
		solimCore.getActiveValues(column).inputValueCount = 0;
		solimCore.getActiveValues(column).outputValueCount = 8;
		solimCore.getActiveValues(column).resultValueCount = 4;
		solimCore.getInactiveValues(column).inputValueCount = 0;
		solimCore.getInactiveValues(column).outputValueCount = 8;
		solimCore.getInactiveValues(column).resultValueCount = 1;

		for (int i = 0; i < 8; i++) {
			solimCore.getActiveValues(column).resultValues[i] = i + column;
			solimCore.getInactiveValues(column).resultValues[i] = -i - column;
		}
	}
	// Now make the second column and the fifth column have different resort values and result values
	solimCore.getInactiveValues(1).resortMode = SolimValueSet::ResortMode::RESORT_ALL;
	solimCore.getInactiveValues(4).resortMode = SolimValueSet::ResortMode::RESORT_ALL;
	for (int i = 0; i < 4; i++) {
		solimCore.getInactiveValues(1).resultValues[i] = -1000;
		solimCore.getInactiveValues(4).resultValues[i] = -1000;
	}

	// The processor should not be called for the input values
	EXPECT_CALL(*processor, processValues).Times(0);
	// The processor should be called for the two mismatching columns on the output
	EXPECT_CALL(*processor, processResults(testing::Address(&solimCore.getInactiveValues(1))))
		.Times(1)
		.WillOnce(testing::Invoke([] (SolimValueSet& solimValueSet) {
			solimValueSet.resultValueCount = 4;
			for (int i = 0; i < 4; i++) {
				solimValueSet.resultValues[i] = i + 1;
			}
		}));
	EXPECT_CALL(*processor, processResults(testing::Address(&solimCore.getInactiveValues(4))))
		.Times(1)
		.WillOnce(testing::Invoke([] (SolimValueSet& solimValueSet) {
			solimValueSet.resultValueCount = 4;
			for (int i = 0; i < 4; i++) {
				solimValueSet.resultValues[i] = i + 4;
			}
		}));
	// The randomzier should be called
	EXPECT_CALL(*randomizer, process).Times(1);

	// Execute the functionality
	solimCore.processAndActivateInactiveValues(8, &randomTriggers);

	for (int column = 0; column < 8; column++) {
		// Both activeValues and inactiveValues should now have the same outputValueCount and outputValues (the ones from the old active values)
		EXPECT_EQ(solimCore.getActiveValues(column).resultValueCount, 4);
		EXPECT_EQ(solimCore.getInactiveValues(column).resultValueCount, 4);

		// The old active values had 8 outputValues, so only the first 4 values should have been copied over
		for (int i = 0; i < 4; i++) {
			EXPECT_NEAR(solimCore.getActiveValues(column).resultValues[i], i + column, 0.0001f);
			EXPECT_NEAR(solimCore.getInactiveValues(column).resultValues[i], i + column, 0.0001f);
		}
		// The next 4 resultValues should remain untouched
		for (int i = 4; i < 8; i++) {
			EXPECT_NEAR(solimCore.getActiveValues(column).resultValues[i], -i - column, 0.0001f);
			EXPECT_NEAR(solimCore.getInactiveValues(column).resultValues[i], i + column, 0.0001f);
		}
	}
}

TEST(SolimCoreTest, WhenProcessingAllColumnsShouldCallProcessorOnMismatchingOutputValueColumnsWithinSubset) {
	std::array<SolimValueSet, 8> valueSet;
	MockSolimCoreProcessor* processor = new MockSolimCoreProcessor();
	MockSolimCoreRandomizer* randomizer = new MockSolimCoreRandomizer();

	SolimCore solimCore(processor, randomizer);
	std::array<RandomTrigger, 8> randomTriggers;

	// First set all 8 columns to not contain any differences
	for (int column = 0; column < 8; column++) {
		// Set both valueSets to 0 input length so they are considered equal
		// Assign result values to verify they are copied over 
		solimCore.getActiveValues(column).inputValueCount = 0;
		solimCore.getActiveValues(column).outputValueCount = 8;
		solimCore.getActiveValues(column).resultValueCount = 4;
		solimCore.getInactiveValues(column).inputValueCount = 0;
		solimCore.getInactiveValues(column).outputValueCount = 8;
		solimCore.getInactiveValues(column).resultValueCount = 1;

		for (int i = 0; i < 8; i++) {
			solimCore.getActiveValues(column).resultValues[i] = i + column;
			solimCore.getInactiveValues(column).resultValues[i] = -i - column;
		}
	}
	// Now make the second column and the fifth column have different resort values and result values
	solimCore.getInactiveValues(1).resortMode = SolimValueSet::ResortMode::RESORT_ALL;
	solimCore.getInactiveValues(4).resortMode = SolimValueSet::ResortMode::RESORT_ALL;
	for (int i = 0; i < 4; i++) {
		solimCore.getInactiveValues(1).resultValues[i] = -1000;
	}

	// The processor should not be called for the input values
	EXPECT_CALL(*processor, processValues).Times(0);
	// The processor should be called for the two mismatching columns on the output
	EXPECT_CALL(*processor, processResults(testing::Address(&solimCore.getInactiveValues(1))))
		.Times(1)
		.WillOnce(testing::Invoke([] (SolimValueSet& solimValueSet) {
			solimValueSet.resultValueCount = 4;
			for (int i = 0; i < 4; i++) {
				solimValueSet.resultValues[i] = i + 1;
			}
		}));
	// The randomzier should be called
	EXPECT_CALL(*randomizer, process).Times(1);

	// Execute the functionality
	solimCore.processAndActivateInactiveValues(4, &randomTriggers);

	for (int column = 0; column < 8; column++) {
		if (column < 4) {
			// Both activeValues and inactiveValues should now have the same outputValueCount and outputValues (the ones from the old active values)
			EXPECT_EQ(solimCore.getActiveValues(column).resultValueCount, 4);
			EXPECT_EQ(solimCore.getInactiveValues(column).resultValueCount, 4);

			// The old active values had 8 outputValues, so only the first 4 values should have been copied over
			for (int i = 0; i < 4; i++) {
				EXPECT_NEAR(solimCore.getActiveValues(column).resultValues[i], i + column, 0.0001f);
				EXPECT_NEAR(solimCore.getInactiveValues(column).resultValues[i], i + column, 0.0001f);
			}
			// The next 4 resultValues should remain untouched
			for (int i = 4; i < 8; i++) {
				EXPECT_NEAR(solimCore.getActiveValues(column).resultValues[i], -i - column, 0.0001f);
				EXPECT_NEAR(solimCore.getInactiveValues(column).resultValues[i], i + column, 0.0001f);
			}
		} else {
			// The other columns are outside of the processing range, so should not have been touched, only switched over.
			EXPECT_EQ(solimCore.getActiveValues(column).resultValueCount, 1);
			EXPECT_EQ(solimCore.getInactiveValues(column).resultValueCount, 4);

			for (int i = 0; i < 8; i++) {
				EXPECT_NEAR(solimCore.getActiveValues(column).resultValues[i], -i - column, 0.0001f);
				EXPECT_NEAR(solimCore.getInactiveValues(column).resultValues[i], i + column, 0.0001f);
			}
		}
	}
}
