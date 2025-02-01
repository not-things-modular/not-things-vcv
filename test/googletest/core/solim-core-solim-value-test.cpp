#include <gtest/gtest.h>
#include "core/solim-core.hpp"

/**
 * SolimValue unit tests
 */
SolimValue getSolimValue() {
	SolimValue solimValue;
	
	solimValue.value = 4.20f;
	solimValue.addOctave = SolimValue::AddOctave::LOWER;
	solimValue.sortRelative = SolimValue::SortRelative::BEFORE;
	solimValue.replaceOriginal = true;

	return solimValue;
}

TEST(SolimValueTest, WithSamePropertiesIsEqual) {
	SolimValue value1 = getSolimValue();
	SolimValue value2 = getSolimValue();
	
	EXPECT_EQ(value1, value2);
	EXPECT_TRUE(value1 == value2);
	EXPECT_FALSE(value1 != value2);
}

TEST(SolimValueTest, WithDifferentValueIsDifferent) {
	SolimValue value1 = getSolimValue();
	SolimValue value2 = getSolimValue();

	value2.value++;
	
	EXPECT_NE(value1, value2);
	EXPECT_FALSE(value1 == value2);
	EXPECT_TRUE(value1 != value2);
}

TEST(SolimValueTest, WithDifferentAddOctaveIsDifferent) {
	SolimValue value1 = getSolimValue();
	SolimValue value2 = getSolimValue();

	value2.addOctave = SolimValue::AddOctave::NONE;
	
	EXPECT_NE(value1, value2);
	EXPECT_FALSE(value1 == value2);
	EXPECT_TRUE(value1 != value2);
}

TEST(SolimValueTest, WithDifferentSortRelativeIsDifferent) {
	SolimValue value1 = getSolimValue();
	SolimValue value2 = getSolimValue();

	value2.sortRelative = SolimValue::SortRelative::AFTER;
	
	EXPECT_NE(value1, value2);
	EXPECT_FALSE(value1 == value2);
	EXPECT_TRUE(value1 != value2);
}

TEST(SolimValueTest, WithDifferentReplaceOriginalIsDifferent) {
	SolimValue value1 = getSolimValue();
	SolimValue value2 = getSolimValue();

	value2.replaceOriginal = false;
	
	EXPECT_NE(value1, value2);
	EXPECT_FALSE(value1 == value2);
	EXPECT_TRUE(value1 != value2);
}

TEST(SolimValueTest, WithOnlyIndexSameIsDifferent) {
	SolimValue value1 = getSolimValue();
	SolimValue value2 = getSolimValue();

	value2.value++;
	value2.addOctave = SolimValue::AddOctave::NONE;
	value2.sortRelative = SolimValue::SortRelative::AFTER;
	value2.replaceOriginal = false;
	
	EXPECT_NE(value1, value2);
	EXPECT_FALSE(value1 == value2);
	EXPECT_TRUE(value1 != value2);
}

/**
 * SolimValueSet unit tests
 */
SolimValueSet getSolimValueSet() {
	SolimValueSet solimValueSet;

	std::fill(solimValueSet.inputValues.begin(), solimValueSet.inputValues.end(), getSolimValue());
	std::fill(solimValueSet.outputOctaves.begin(), solimValueSet.outputOctaves.end(), SolimValue::AddOctave::NONE);
	std::fill(solimValueSet.outputValues.begin(), solimValueSet.outputValues.end(), getSolimValue());

	return solimValueSet;
}

void fillSolimValueSet(SolimValueSet& solimValueSet) {
	solimValueSet.inputValueCount = 4;
	for (int i = 0; i < 4; i++) {
		solimValueSet.inputValues[i].value += i;
	}

	solimValueSet.lowerLimit = 1.69f;
	solimValueSet.upperLimit = 4.20f;
	solimValueSet.sort = 1.f;

	solimValueSet.outputValueCount = 5;
	for (int i = 0; i < 5; i++) {
		solimValueSet.outputValues[i].value += i;
	}
	solimValueSet.resortMode = SolimValueSet::ResortMode::RESORT_NONE;

	solimValueSet.resultValueCount = 5;
	for (int i = 0; i < 5; i++) {
		solimValueSet.resultValues[i] += i;
	}
}

TEST(SolimValueSetTest, TwoEmptySetsInputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	// Both sets have their input and output values count set to 0, so they should not compare any other input parameters
	valueSet2.inputValues[0].value++;
	valueSet2.outputValues[0].value++;
	valueSet2.upperLimit++;
	valueSet2.lowerLimit++;
	valueSet2.sort++;

	EXPECT_TRUE(valueSet1.inputParametersMatch(valueSet2));
}

TEST(SolimValueSetTest, EmptyValueSetIsDifferentFromNonEmptyForInputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	valueSet2.inputValueCount = 1;

	EXPECT_FALSE(valueSet1.inputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithSamePropertiesAreEqualForInputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);

	EXPECT_TRUE(valueSet1.inputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyDifferentInputCountsAreDifferentForInputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.inputValueCount++;

	EXPECT_FALSE(valueSet1.inputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyOneDifferentInputValueAreDifferentForInputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.inputValues[2].value = -2.5f;

	EXPECT_FALSE(valueSet1.inputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyOneDifferentInputValueOutsideOfScopeAreEqualForInputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.inputValues[valueSet2.inputValueCount].value = -2.5f;

	EXPECT_TRUE(valueSet1.inputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyDifferentUpperLimitAreDifferentForInputMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.upperLimit++;

	EXPECT_FALSE(valueSet1.inputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyDifferentLowerLimitAreDifferentForInputMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.lowerLimit++;

	EXPECT_FALSE(valueSet1.inputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyDifferentSortAreDifferentForInputMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.sort++;

	EXPECT_FALSE(valueSet1.inputParametersMatch(valueSet2));
}

TEST(SolimValueTest, InputMatchShouldIgnoreNonInputProperties) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);

	valueSet2.outputValueCount++;
	valueSet2.outputValues[1].value++;
	valueSet2.indices[1]++;
	valueSet2.outputOctaves[1] = SolimValue::AddOctave::HIGHER;
	valueSet2.outputReplaceOriginal[1] = !valueSet2.outputReplaceOriginal[1];
	valueSet2.resultValueCount++;
	valueSet2.resultValues[1]++;

	EXPECT_TRUE(valueSet1.inputParametersMatch(valueSet2));
}

TEST(SolimValueSetTest, TwoEmptySetsOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	// Both sets have their input and output values count set to 0, so they should not compare any other output parameters
	valueSet2.outputValues[0].value++;
	valueSet2.outputReplaceOriginal[0] = !valueSet2.outputReplaceOriginal[0];
	valueSet2.resortMode = SolimValueSet::ResortMode::RESORT_CONNECTED;

	EXPECT_TRUE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueSetTest, EmptyValueSetIsDifferentFromNonEmptyForOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	valueSet2.outputValueCount = 1;

	EXPECT_FALSE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithSamePropertiesAreEqualForOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);

	EXPECT_TRUE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyDifferentOutputCountsAreDifferentForOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.outputValueCount++;

	EXPECT_FALSE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyOneDifferentOutputValueAreDifferentForOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.outputValues[2].value = -2.5f;

	EXPECT_FALSE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyOneDifferentOutputValueOutsideOfScopeAreEqualForOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.outputValues[valueSet2.outputValueCount].value = -2.5f;

	EXPECT_TRUE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyOneDifferentOutputOctavesAreDifferentForOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.outputOctaves[1] = SolimValue::AddOctave::LOWER;

	EXPECT_FALSE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyOneDifferentOutputReplaceOriginalAreDifferentForOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.outputReplaceOriginal[1] = !valueSet2.outputReplaceOriginal[1];

	EXPECT_FALSE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyDifferentResortAreDifferentForOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.resortMode = SolimValueSet::ResortMode::RESORT_ALL;

	EXPECT_FALSE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithConnectedResortModeAndDifferentConnectedOutputsAreDifferentForOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();
	std::array<bool, 8> connected1 = { true };
	std::array<bool, 8> connected2 = { true };
	connected2[4] = false;

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet1.resortMode = SolimValueSet::ResortMode::RESORT_CONNECTED;
	valueSet1.outputConnected = &connected1;
	valueSet2.resortMode = SolimValueSet::ResortMode::RESORT_CONNECTED;
	valueSet2.outputConnected = &connected2;

	EXPECT_FALSE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithResortModeAllOrResortModeNoneShouldNotBeInfluencedByOutputConnectedForOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();
	std::array<bool, 8> connected1 = { true };
	std::array<bool, 8> connected2 = { true };
	connected2[4] = false;

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet1.resortMode = SolimValueSet::ResortMode::RESORT_ALL;
	valueSet1.outputConnected = &connected1;
	valueSet2.resortMode = SolimValueSet::ResortMode::RESORT_ALL;
	valueSet2.outputConnected = &connected2;
	EXPECT_TRUE(valueSet1.outputParametersMatch(valueSet2));

	valueSet1.resortMode = SolimValueSet::ResortMode::RESORT_NONE;
	valueSet2.resortMode = SolimValueSet::ResortMode::RESORT_NONE;
	EXPECT_TRUE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueTest, NonEmptyValueSetsWithOnlyOneDifferentIndicesAreDifferentForOutputsMatch) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);
	valueSet2.indices[1]++;

	EXPECT_FALSE(valueSet1.outputParametersMatch(valueSet2));
}

TEST(SolimValueTest, OutputMatchShouldIgnoreNonOutputProperties) {
	SolimValueSet valueSet1 = getSolimValueSet();
	SolimValueSet valueSet2 = getSolimValueSet();

	fillSolimValueSet(valueSet1);
	fillSolimValueSet(valueSet2);

	valueSet2.inputValueCount++;
	valueSet2.inputValues[1].value++;
	valueSet2.upperLimit++;
	valueSet2.lowerLimit++;
	valueSet2.sort++;
	valueSet2.resultValueCount++;
	valueSet2.resultValues[1]++;

	EXPECT_TRUE(valueSet1.outputParametersMatch(valueSet2));
}
