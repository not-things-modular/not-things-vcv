#include <gtest/gtest.h>

#include "core/solim-core.hpp"


// The processor doesn't maintain a state, so we can use a single instance for all tests
SolimCoreProcessor solimCoreProcessor;

/** Unit tests for the processValues method */

SolimValueSet createValueSet() {
	SolimValueSet valueSet;

	valueSet.inputValueCount = 0;
	for (int i = 0; i < 8; i++) {
		valueSet.inputValues[i].value = 0.f;
		valueSet.inputValues[i].addOctave = SolimValue::AddOctave::NONE;
		valueSet.inputValues[i].sortRelative = SolimValue::SortRelative::BEFORE;
		valueSet.inputValues[i].replaceOriginal = false;
		valueSet.outputValues[i].value = 0.f;
	}

	valueSet.lowerLimit = -10.f;
	valueSet.upperLimit = 10.f;
	valueSet.sort = 0;
	valueSet.outputOctaves = { SolimValue::AddOctave::NONE };
	valueSet.outputReplaceOriginal = { false };

	return valueSet;
}

void setUnsortedInputValues(SolimValueSet& valueSet) {
	valueSet.inputValues[0].value = 5.33f;
	valueSet.inputValues[1].value = 1.23f;
	valueSet.inputValues[2].value = 4.43f;
	valueSet.inputValues[3].value = 10.0f;
	valueSet.inputValues[4].value = -1.f;
	valueSet.inputValues[5].value = -2.f;
	valueSet.inputValues[6].value = 0.f;
	valueSet.inputValues[7].value = 8.5f;
}

void verifyEmptyResultValues(SolimValueSet& valueSet) {
	EXPECT_EQ(valueSet.resultValueCount, 0);
	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(valueSet.resultValues[i], 0.f, 0.00001f);
	}
}

TEST(SolimCoreProcessorTestForInputValues, WithNoInputValuesShouldHaveNoOutputValues) {
	SolimValueSet valueSet = createValueSet();
	valueSet.outputValueCount = 420;

	solimCoreProcessor.processValues(valueSet);

	EXPECT_EQ(valueSet.outputValueCount, 0);

	verifyEmptyResultValues(valueSet);
}

TEST(SolimCoreProcessorTestForInputValues, WithNoModificationsShouldDoNothingAndUpdateOutputValueCount) {
	SolimValueSet valueSet = createValueSet();

	setUnsortedInputValues(valueSet);
	valueSet.inputValueCount = 8;
	valueSet.outputValueCount = 69;

	solimCoreProcessor.processValues(valueSet);

	EXPECT_EQ(valueSet.outputValueCount, 8);

	// Compare the output values against a new valueSet: if the processor did end up sorting,
	// just checking the inputValue against the ouputValue wouldn't notice that
	SolimValueSet referenceValueSet = createValueSet();
	setUnsortedInputValues(referenceValueSet);
	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(valueSet.outputValues[i].value, referenceValueSet.inputValues[i].value, 0.0001f);
	}

	verifyEmptyResultValues(valueSet);
}

TEST(SolimCoreProcessorTestForInputValues, ShouldLimitValues) {
	SolimValueSet valueSet = createValueSet();

	setUnsortedInputValues(valueSet);
	valueSet.inputValueCount = 8;
	valueSet.outputValueCount = 42;
	valueSet.lowerLimit = -1.f;
	valueSet.upperLimit = 3.f;

	solimCoreProcessor.processValues(valueSet);

	EXPECT_EQ(valueSet.outputValueCount, 8);

	EXPECT_NEAR(valueSet.outputValues[0].value, 2.33f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[1].value, 1.23f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[2].value, 2.43f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[3].value, 3.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[4].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[5].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[6].value, 0.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[7].value, 2.5f, 0.0001f);

	verifyEmptyResultValues(valueSet);
}

TEST(SolimCoreProcessorTestForInputValues, WithReplaceOriginalAndNoInputOctavingShouldRemoveValues) {
	SolimValueSet valueSet = createValueSet();

	setUnsortedInputValues(valueSet);
	valueSet.inputValueCount = 8;
	valueSet.outputValueCount = 69;

	valueSet.inputValues[3].replaceOriginal = true;
	valueSet.inputValues[6].replaceOriginal = true;

	solimCoreProcessor.processValues(valueSet);

	EXPECT_EQ(valueSet.outputValueCount, 6);

	// Compare the output values against a new valueSet: if the processor did end up sorting,
	// just checking the inputValue against the ouputValue wouldn't notice that
	SolimValueSet referenceValueSet = createValueSet();
	setUnsortedInputValues(referenceValueSet);
	EXPECT_NEAR(valueSet.outputValues[0].value, referenceValueSet.inputValues[0].value, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[1].value, referenceValueSet.inputValues[1].value, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[2].value, referenceValueSet.inputValues[2].value, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[3].value, referenceValueSet.inputValues[4].value, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[4].value, referenceValueSet.inputValues[5].value, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[5].value, referenceValueSet.inputValues[7].value, 0.0001f);

	verifyEmptyResultValues(valueSet);
}

TEST(SolimCoreProcessorTestForInputValues, WithReplaceOriginalAndSomeInputOctavingPreSortShouldRemoveSomeValuesAndAddOctaves) {
	SolimValueSet valueSet = createValueSet();

	setUnsortedInputValues(valueSet);
	valueSet.inputValueCount = 7;
	valueSet.outputValueCount = 69;
	valueSet.sort = 1;

	valueSet.inputValues[0].addOctave = SolimValue::AddOctave::LOWER;
	valueSet.inputValues[5].replaceOriginal = true;
	valueSet.inputValues[5].addOctave = SolimValue::AddOctave::HIGHER;
	valueSet.inputValues[6].replaceOriginal = true;
	valueSet.inputValues[6].addOctave = SolimValue::AddOctave::LOWER;

	solimCoreProcessor.processValues(valueSet);

	EXPECT_EQ(valueSet.outputValueCount, 8);

	EXPECT_NEAR(valueSet.outputValues[0].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[1].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[2].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[3].value, 1.23f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[4].value, 4.33f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[5].value, 4.43f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[6].value, 5.33f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[7].value, 10.f, 0.0001f);

	verifyEmptyResultValues(valueSet);
}

TEST(SolimCoreProcessorTestForInputValues, WithReplaceOriginalAndSomeInputOctavingPostSortShouldRemoveSomeValuesAndAddOctavesForAscendingSort) {
	SolimValueSet valueSet = createValueSet();

	setUnsortedInputValues(valueSet);
	valueSet.inputValueCount = 7;
	valueSet.outputValueCount = 69;
	valueSet.sort = 1;

	valueSet.inputValues[0].addOctave = SolimValue::AddOctave::LOWER;
	valueSet.inputValues[0].sortRelative = SolimValue::SortRelative::AFTER;
	valueSet.inputValues[5].replaceOriginal = true;
	valueSet.inputValues[5].addOctave = SolimValue::AddOctave::HIGHER;
	valueSet.inputValues[5].sortRelative = SolimValue::SortRelative::AFTER;
	valueSet.inputValues[6].replaceOriginal = true;
	valueSet.inputValues[6].addOctave = SolimValue::AddOctave::LOWER;
	valueSet.inputValues[6].sortRelative = SolimValue::SortRelative::AFTER;

	solimCoreProcessor.processValues(valueSet);

	EXPECT_EQ(valueSet.outputValueCount, 8);

	EXPECT_NEAR(valueSet.outputValues[0].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[1].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[2].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[3].value, 1.23f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[4].value, 4.43f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[6].value, 4.33f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[5].value, 5.33f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[7].value, 10.f, 0.0001f);
	
	verifyEmptyResultValues(valueSet);
}

TEST(SolimCoreProcessorTestForInputValues, WithReplaceOriginalAndSomeInputOctavingPostSortShouldRemoveSomeValuesAndAddOctavesForDescendingSort) {
	SolimValueSet valueSet = createValueSet();

	setUnsortedInputValues(valueSet);
	valueSet.inputValueCount = 7;
	valueSet.outputValueCount = 69;
	valueSet.sort = -1;

	valueSet.inputValues[0].addOctave = SolimValue::AddOctave::LOWER;
	valueSet.inputValues[0].sortRelative = SolimValue::SortRelative::AFTER;
	valueSet.inputValues[5].replaceOriginal = true;
	valueSet.inputValues[5].addOctave = SolimValue::AddOctave::HIGHER;
	valueSet.inputValues[5].sortRelative = SolimValue::SortRelative::AFTER;
	valueSet.inputValues[6].replaceOriginal = true;
	valueSet.inputValues[6].addOctave = SolimValue::AddOctave::LOWER;
	valueSet.inputValues[6].sortRelative = SolimValue::SortRelative::AFTER;

	solimCoreProcessor.processValues(valueSet);

	EXPECT_EQ(valueSet.outputValueCount, 8);

	EXPECT_NEAR(valueSet.outputValues[7].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[6].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[5].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[4].value, 1.23f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[3].value, 4.43f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[2].value, 4.33f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[1].value, 5.33f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[0].value, 10.f, 0.0001f);
	
	verifyEmptyResultValues(valueSet);
}

TEST(SolimCoreProcessorTestForInputValues, ShouldSortValuesAscending) {
	SolimValueSet valueSet = createValueSet();

	setUnsortedInputValues(valueSet);
	valueSet.inputValueCount = 8;
	valueSet.outputValueCount = 69;
	valueSet.sort = 1;

	solimCoreProcessor.processValues(valueSet);

	EXPECT_EQ(valueSet.outputValueCount, 8);

	EXPECT_NEAR(valueSet.outputValues[0].value, -2.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[1].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[2].value, 0.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[3].value, 1.23f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[4].value, 4.43f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[5].value, 5.33f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[6].value, 8.5f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[7].value, 10.0f, 0.0001f);

	verifyEmptyResultValues(valueSet);
}

TEST(SolimCoreProcessorTestForInputValues, ShouldSortValuesDescending) {
	SolimValueSet valueSet = createValueSet();

	setUnsortedInputValues(valueSet);
	valueSet.inputValueCount = 8;
	valueSet.outputValueCount = 69;
	valueSet.sort = -1;

	solimCoreProcessor.processValues(valueSet);

	EXPECT_EQ(valueSet.outputValueCount, 8);

	EXPECT_NEAR(valueSet.outputValues[0].value, 10.0f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[1].value, 8.5f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[2].value, 5.33f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[3].value, 4.43f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[4].value, 1.23f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[5].value, 0.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[6].value, -1.f, 0.0001f);
	EXPECT_NEAR(valueSet.outputValues[7].value, -2.f, 0.0001f);

	verifyEmptyResultValues(valueSet);
}



/** Unit tests for the processResults method */


SolimValueSet createOutputSet() {
	SolimValueSet valueSet;

	valueSet.sort = 0;
	valueSet.outputValueCount = 0;
	for (int i = 0; i < 8; i++) {
		valueSet.outputValues[i].value = 0.f;
		valueSet.resultValues[i] = 0.f;
	}

	valueSet.outputOctaves = { SolimValue::AddOctave::NONE };
	valueSet.outputReplaceOriginal = { false };

	return valueSet;
}

void setUnsortedOutputValues(SolimValueSet& valueSet) {
	valueSet.outputValues[0].value = 5.43f;
	valueSet.outputValues[1].value = 1.23f;
	valueSet.outputValues[2].value = 4.33f;
	valueSet.outputValues[3].value = 10.0f;
	valueSet.outputValues[4].value = -1.f;
	valueSet.outputValues[5].value = -2.f;
	valueSet.outputValues[6].value = 0.f;
	valueSet.outputValues[7].value = 8.5f;
	valueSet.outputValues[8].value = -2.12f;
	valueSet.outputValues[9].value = 0.31f;
	valueSet.outputValues[10].value = 8.6f;
}

TEST(SolimCoreProcessorTestForResultValues, WithNoOutputValuesShouldHaveNoResults) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.resultValueCount = 420;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 0);
}

TEST(SolimCoreProcessorTestForResultValues, WithNoOutputOctavingOrReplacingShouldUseOutputValues) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.outputValueCount = 8;
	valueSet.resultValueCount = 420;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 8);
	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(valueSet.resultValues[i], valueSet.outputValues[i].value, 0.00001f);
	}
}

TEST(SolimCoreProcessorTestForResultValues, WithNoOutputOctavingOrReplacingShouldUseSubsetOfOutputValues) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.outputValueCount = 4;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 4);
	for (int i = 0; i < 4; i++) {
		EXPECT_NEAR(valueSet.resultValues[i], valueSet.outputValues[i].value, 0.00001f);
	}
	for (int i = 4; i < 8; i++) {
		EXPECT_NEAR(valueSet.resultValues[i], 0.f, 0.00001f);
	}
}

TEST(SolimCoreProcessorTestForResultValues, WithNoOctavingShouldRemoveReplacedValues) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.outputValueCount = 8;
	valueSet.outputReplaceOriginal[2] = true;
	valueSet.outputReplaceOriginal[4] = true;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 6);
	EXPECT_NEAR(valueSet.resultValues[0], valueSet.outputValues[0].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[1], valueSet.outputValues[1].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[2], valueSet.outputValues[3].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[3], valueSet.outputValues[5].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[4], valueSet.outputValues[6].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[5], valueSet.outputValues[7].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[6], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], 0.f, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithNoMainSortAndWithOctavingShouldAddOctaves) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.outputValueCount = 4;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 6);
	EXPECT_NEAR(valueSet.resultValues[0], valueSet.outputValues[3].value - 1.f, 0.00001f); // The lowered octave of index 3
	EXPECT_NEAR(valueSet.resultValues[1], valueSet.outputValues[0].value, 0.00001f); // Followed by the 4 original values 
	EXPECT_NEAR(valueSet.resultValues[2], valueSet.outputValues[1].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[3], valueSet.outputValues[2].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[4], valueSet.outputValues[3].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[5], valueSet.outputValues[1].value + 1.f, 0.00001f); // And finally the increased octave of index 1
	EXPECT_NEAR(valueSet.resultValues[6], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], 0.f, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithNoMainSortAndWithOctavingAndReplaceOriginalShouldAddOctavesAndReplace) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.outputValueCount = 4;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputReplaceOriginal[1] = true;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.outputReplaceOriginal[3] = true;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 4);
	EXPECT_NEAR(valueSet.resultValues[0], valueSet.outputValues[3].value - 1.f, 0.00001f); // The lowered octave of index 3
	EXPECT_NEAR(valueSet.resultValues[1], valueSet.outputValues[0].value, 0.00001f); // Followed by the 2 original values that were not replaced
	EXPECT_NEAR(valueSet.resultValues[2], valueSet.outputValues[2].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[3], valueSet.outputValues[1].value + 1.f, 0.00001f); // And finally the increased octave of index 1
	EXPECT_NEAR(valueSet.resultValues[4], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[5], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[6], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], 0.f, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithNoMainSortAndWithOctavingGoingOutOfBoundsShouldOnlyUseFirstEightItems) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.outputValueCount = 8;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 8);
	EXPECT_NEAR(valueSet.resultValues[0], valueSet.outputValues[3].value - 1.f, 0.00001f); // The lowered octave of index 3
	EXPECT_NEAR(valueSet.resultValues[1], valueSet.outputValues[0].value, 0.00001f); // Followed by the remaining 7 original values that fit inside the list
	EXPECT_NEAR(valueSet.resultValues[2], valueSet.outputValues[1].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[3], valueSet.outputValues[2].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[4], valueSet.outputValues[3].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[5], valueSet.outputValues[4].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[6], valueSet.outputValues[5].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], valueSet.outputValues[6].value, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithNoMainSortAndWithOctavingGoingOutOfBoundsShouldOnlyUseFirstEightItemsAndIgnoreResort) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.outputValueCount = 8;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.resort = true;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 8);
	EXPECT_NEAR(valueSet.resultValues[0], valueSet.outputValues[3].value - 1.f, 0.00001f); // The lowered octave of index 3
	EXPECT_NEAR(valueSet.resultValues[1], valueSet.outputValues[0].value, 0.00001f); // Followed by the remaining 7 original values that fit inside the list
	EXPECT_NEAR(valueSet.resultValues[2], valueSet.outputValues[1].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[3], valueSet.outputValues[2].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[4], valueSet.outputValues[3].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[5], valueSet.outputValues[4].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[6], valueSet.outputValues[5].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], valueSet.outputValues[6].value, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithAscendingMainSortAndWithOctavingShouldAddOctaves) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.sort = 1;
	valueSet.outputValueCount = 4;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 6);
	EXPECT_NEAR(valueSet.resultValues[0], valueSet.outputValues[3].value - 1.f, 0.00001f); // The lowered octave of index 3
	EXPECT_NEAR(valueSet.resultValues[1], valueSet.outputValues[0].value, 0.00001f); // Followed by the 4 original values 
	EXPECT_NEAR(valueSet.resultValues[2], valueSet.outputValues[1].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[3], valueSet.outputValues[2].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[4], valueSet.outputValues[3].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[5], valueSet.outputValues[1].value + 1.f, 0.00001f); // And finally the increased octave of index 1
	EXPECT_NEAR(valueSet.resultValues[6], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], 0.f, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithAscendingMainSortAndWithOctavingAndReplaceOriginalShouldAddOctavesAndReplace) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.sort = 1;
	valueSet.outputValueCount = 4;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputReplaceOriginal[1] = true;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.outputReplaceOriginal[3] = true;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 4);
	EXPECT_NEAR(valueSet.resultValues[0], valueSet.outputValues[3].value - 1.f, 0.00001f); // The lowered octave of index 3
	EXPECT_NEAR(valueSet.resultValues[1], valueSet.outputValues[0].value, 0.00001f); // Followed by the 2 original values that were not replaced
	EXPECT_NEAR(valueSet.resultValues[2], valueSet.outputValues[2].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[3], valueSet.outputValues[1].value + 1.f, 0.00001f); // And finally the increased octave of index 1
	EXPECT_NEAR(valueSet.resultValues[4], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[5], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[6], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], 0.f, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithAscendingMainSortAndWithOctavingGoingOutOfBoundsShouldOnlyUseFirstEightItems) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.sort = 1;
	valueSet.outputValueCount = 8;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 8);
	EXPECT_NEAR(valueSet.resultValues[0], valueSet.outputValues[3].value - 1.f, 0.00001f); // The lowered octave of index 3
	EXPECT_NEAR(valueSet.resultValues[1], valueSet.outputValues[0].value, 0.00001f); // Followed by the remaining 7 original values that fit inside the list
	EXPECT_NEAR(valueSet.resultValues[2], valueSet.outputValues[1].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[3], valueSet.outputValues[2].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[4], valueSet.outputValues[3].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[5], valueSet.outputValues[4].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[6], valueSet.outputValues[5].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], valueSet.outputValues[6].value, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithAscendingMainSortAndWithOctavingGoingOutOfBoundsShouldOnlyUseFirstEightItemsAndIgnoreResort) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.sort = 1;
	valueSet.outputValueCount = 8;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.resort = true;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 8);
	EXPECT_NEAR(valueSet.resultValues[0], -2.f, 0.00001f); // The lowered octave of index 3
	EXPECT_NEAR(valueSet.resultValues[1], -1.f, 0.00001f); // Followed by the remaining 7 original values that fit inside the list
	EXPECT_NEAR(valueSet.resultValues[2], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[3], 1.23f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[4], 4.33f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[5], 5.43f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[6], 9.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], 10.f, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithDescendingMainSortAndWithOctavingShouldAddOctaves) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.sort = -1;
	valueSet.outputValueCount = 4;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 6);
	EXPECT_NEAR(valueSet.resultValues[0], valueSet.outputValues[1].value + 1.f, 0.00001f); // The increased octave of index 1
	EXPECT_NEAR(valueSet.resultValues[1], valueSet.outputValues[0].value, 0.00001f); // Followed by the 4 original values 
	EXPECT_NEAR(valueSet.resultValues[2], valueSet.outputValues[1].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[3], valueSet.outputValues[2].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[4], valueSet.outputValues[3].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[5], valueSet.outputValues[3].value - 1.f, 0.00001f); // And finally the lowered octave of index 3
	EXPECT_NEAR(valueSet.resultValues[6], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], 0.f, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithDescendingMainSortAndWithOctavingAndReplaceOriginalShouldAddOctavesAndReplace) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.sort = -1;
	valueSet.outputValueCount = 4;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputReplaceOriginal[1] = true;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.outputReplaceOriginal[3] = true;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 4);
	EXPECT_NEAR(valueSet.resultValues[0], valueSet.outputValues[1].value + 1.f, 0.00001f); // The increased octave of index 1
	EXPECT_NEAR(valueSet.resultValues[1], valueSet.outputValues[0].value, 0.00001f); // Followed by the 2 original values that were not replaced
	EXPECT_NEAR(valueSet.resultValues[2], valueSet.outputValues[2].value, 0.00001f); //
	EXPECT_NEAR(valueSet.resultValues[3], valueSet.outputValues[3].value - 1.f, 0.00001f); // And finally the lowered octave of index 3
	EXPECT_NEAR(valueSet.resultValues[4], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[5], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[6], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], 0.f, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithDescendingMainSortAndWithOctavingGoingOutOfBoundsShouldOnlyUseFirstEightItems) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.sort = -1;
	valueSet.outputValueCount = 8;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 8);
	EXPECT_NEAR(valueSet.resultValues[0], valueSet.outputValues[1].value + 1.f, 0.00001f); // The increased octave of index 1
	EXPECT_NEAR(valueSet.resultValues[1], valueSet.outputValues[0].value, 0.00001f); // Followed by the remaining 7 original values that fit inside the list
	EXPECT_NEAR(valueSet.resultValues[2], valueSet.outputValues[1].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[3], valueSet.outputValues[2].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[4], valueSet.outputValues[3].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[5], valueSet.outputValues[4].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[6], valueSet.outputValues[5].value, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], valueSet.outputValues[6].value, 0.00001f);
}

TEST(SolimCoreProcessorTestForResultValues, WithDescendingMainSortAndWithOctavingGoingOutOfBoundsShouldOnlyUseFirstEightItemsAndIgnoreResort) {
	SolimValueSet valueSet = createOutputSet();
	setUnsortedOutputValues(valueSet);
	valueSet.sort = -1;
	valueSet.outputValueCount = 8;
	valueSet.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet.outputOctaves[3] = SolimValue::AddOctave::LOWER;
	valueSet.resort = true;
	valueSet.resultValueCount = 69;

	solimCoreProcessor.processResults(valueSet);

	EXPECT_EQ(valueSet.resultValueCount, 8);
	EXPECT_NEAR(valueSet.resultValues[0], 10.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[1], 5.43f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[2], 4.33f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[3], 2.23f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[4], 1.23f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[5], 0.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[6], -1.f, 0.00001f);
	EXPECT_NEAR(valueSet.resultValues[7], -2.f, 0.00001f);
}
