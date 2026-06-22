#include <gmock/gmock.h>
#include "core/ratrilig-core.hpp"
#include <tuple>

struct MockRatriligChanceGenerator : RatriligChanceGenerator {
	MOCK_METHOD(float, generateSkipChance, (), (override));
	MOCK_METHOD(float, generateDensityModifier, (), (override));
	MOCK_METHOD(float, generateTrigger, (), (override));
};

struct MockRatriligCoreListener : RatriligCoreListener {
	MOCK_METHOD(void, clusterStateChanged, (int, bool, float, float), (override));
	MOCK_METHOD(void, phraseStateChanged, (int, bool, float, float), (override));
	MOCK_METHOD(void, cycleStateChanged, (int, bool, float), (override));
	MOCK_METHOD(void, valueChanged, (int, int, int, int, float, float, bool), (override));
	MOCK_METHOD(void, clusterStarted, (int), (override));
	MOCK_METHOD(void, phraseStarted, (int), (override));
	MOCK_METHOD(void, cycleStarted, (int), (override));
};

RatriligData populateRatriligData(float density, int clusterSize, float clusterSkipChance, float clusterDensityModifier, int phraseSize, float phraseSkipChance, float phraseDensityModifier, int cycleSize, float cycleSkipChance, float cycleDensityModifier, float clusterBiasAmount, float clusterBiasPosition, float phraseBiasAmount, float phraseBiasPosition) {
	RatriligData data;

	data.density = density;

	data.clusterData.size = clusterSize;
	data.clusterData.skipChance = clusterSkipChance;
	data.clusterData.densityModifier = clusterDensityModifier;

	data.phraseData.size = phraseSize;
	data.phraseData.skipChance = phraseSkipChance;
	data.phraseData.densityModifier = phraseDensityModifier;

	data.cycleData.size = cycleSize;
	data.cycleData.skipChance = cycleSkipChance;
	data.cycleData.densityModifier = cycleDensityModifier;

	data.clusterData.biasAmount = clusterBiasAmount;
	data.clusterData.biasPosition = clusterBiasPosition;
	data.phraseData.biasAmount = phraseBiasAmount;
	data.phraseData.biasPosition = phraseBiasPosition;

	return data;
}

RatriligData populateDefaultRatriligData(float density, int clusterSize, int phraseSize, int cycleSize) {
	return populateRatriligData(density, clusterSize, 0.f, 0.f, phraseSize, 0.f, 0.f, cycleSize, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
}

RatriligCoreState populateRatriligState(int clusterIndex, int phraseIndex, int cycleIndex, bool clusterEnabled, bool phraseEnabled, bool cycleEnabled, float clusterDensityModifier, float phraseDensityModifier, float cycleDensityModifier, float clusterBiasAmount, float phraseBiasAmount) {
	RatriligCoreState state;

	state.clusterState.index = clusterIndex;
	state.phraseState.index = phraseIndex;
	state.cycleState.index = cycleIndex;

	state.clusterState.enabled = clusterEnabled;
	state.phraseState.enabled = phraseEnabled;
	state.cycleState.enabled = cycleEnabled;

	state.clusterState.densityModifier = clusterDensityModifier;
	state.phraseState.densityModifier = phraseDensityModifier;
	state.cycleState.densityModifier = cycleDensityModifier;

	state.clusterState.biasAmount = clusterBiasAmount;
	state.phraseState.biasAmount = phraseBiasAmount;

	return state;
}

RatriligCoreState populateDefaultRatriligState() {
	return populateRatriligState(0, 0, 0, false, false, false, 0.f, 0.f, 0.f, 0.f, 0.f);
}

void expectChanceGeneratorInvocations(MockRatriligChanceGenerator &mockRatriligChanceGenerator, int generateSkipChanceCount, int generateDensityModifierCount, int generateTriggerCount) {
	EXPECT_CALL(mockRatriligChanceGenerator, generateSkipChance).Times(generateSkipChanceCount);
	EXPECT_CALL(mockRatriligChanceGenerator, generateDensityModifier).Times(generateDensityModifierCount);
	EXPECT_CALL(mockRatriligChanceGenerator, generateTrigger).Times(generateTriggerCount);
}

enum RatriligCoreTestTargets {
	TEST_FOR_CLUSTER,
	TEST_FOR_PHRASE,
	TEST_FOR_CYCLE
};

class RatriligCoreProcessorAdvanceTest : public testing::TestWithParam<RatriligCoreTestTargets> {
	protected:
		void callAdvance(RatriligCoreProcessor& processor, RatriligData& data, RatriligProcessorProgress& progress) {
			if (GetParam() == TEST_FOR_CLUSTER) {
				processor.advanceCluster(data, progress);
 			} else if (GetParam() == TEST_FOR_PHRASE) {
				processor.advancePhrase(data, progress);
 			} else if (GetParam() == TEST_FOR_CYCLE) {
				processor.advanceCycle(data, progress);
			}
		}
};

TEST_P(RatriligCoreProcessorAdvanceTest, advanceShouldNotStartNewWhileBelowThreshold) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);

	int sizes[] = { 5, 8, 2 };

	for (int i = 0; i < 3; i++) {
		RatriligProcessorProgress progress;
		RatriligData data = populateDefaultRatriligData(0.f,
			GetParam() == TEST_FOR_CLUSTER ? sizes[i] : 5,
			GetParam() == TEST_FOR_PHRASE ? sizes[i] : 3,
			GetParam() == TEST_FOR_CYCLE ? sizes[i] : 2);
		RatriligCoreProcessor processor(mockRatriligChanceGenerator);

		std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
		RatriligCoreState referenceState = populateDefaultRatriligState();
		*state = populateDefaultRatriligState();
		processor.setState(state);

		RatriligCoreLayerState& layerState =
			GetParam() == TEST_FOR_CLUSTER ? state->clusterState :
			GetParam() == TEST_FOR_PHRASE ? state->phraseState : state->cycleState;
		RatriligCoreLayerState& referenceLayerState =
			GetParam() == TEST_FOR_CLUSTER ? referenceState.clusterState :
			GetParam() == TEST_FOR_PHRASE ? referenceState.phraseState :
			referenceState.cycleState;

		for (int j = 0; j < sizes[i] - 1; j++) {
			// Set the progress properties to true before the test so that we can explicitly verify that the correct one gets updated
			progress.clusterStarted = true;
			progress.phraseStarted = true;
			progress.cycleStarted = true;
			callAdvance(processor, data, progress);

			EXPECT_EQ(progress.clusterStarted, GetParam() == TEST_FOR_CLUSTER ? false : true);
			EXPECT_EQ(progress.phraseStarted, GetParam() == TEST_FOR_PHRASE ? false : true);
			EXPECT_EQ(progress.cycleStarted, GetParam() == TEST_FOR_CYCLE ? false : true);

			// The only state change should be the layer index increased by one (i.e. one above the innner loop index)
			EXPECT_EQ(layerState.index, referenceLayerState.index + 1);
			EXPECT_EQ(layerState.index, j + 1);
			referenceLayerState.index++;
			EXPECT_EQ(*state, referenceState);
		}
	}
}

TEST_P(RatriligCoreProcessorAdvanceTest, advanceShouldStartNewWhenOnThreshold) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);

	int sizes[] = { 5, 8, 2 };

	for (int i = 0; i < 3; i++) {
		RatriligProcessorProgress progress;
		RatriligData data = populateDefaultRatriligData(0.f,
			GetParam() == TEST_FOR_CLUSTER ? sizes[i] : 5,
			GetParam() == TEST_FOR_PHRASE ? sizes[i] : 3,
			GetParam() == TEST_FOR_CYCLE ? sizes[i] : 2);
		RatriligCoreProcessor processor(mockRatriligChanceGenerator);

		std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
		RatriligCoreState referenceState = populateDefaultRatriligState();
		*state = populateDefaultRatriligState();
		processor.setState(state);

		RatriligCoreLayerState& layerState =
			GetParam() == TEST_FOR_CLUSTER ? state->clusterState :
			GetParam() == TEST_FOR_PHRASE ? state->phraseState : state->cycleState;
		RatriligCoreLayerState& referenceLayerState =
			GetParam() == TEST_FOR_CLUSTER ? referenceState.clusterState :
			GetParam() == TEST_FOR_PHRASE ? referenceState.phraseState :
			referenceState.cycleState;

		layerState.index = sizes[i] - 1;

		progress.clusterStarted = GetParam() != TEST_FOR_CLUSTER;
		progress.phraseStarted = GetParam() == TEST_FOR_CYCLE;
		progress.cycleStarted = false;

		callAdvance(processor, data, progress);

		EXPECT_TRUE(progress.clusterStarted);
		EXPECT_EQ(progress.phraseStarted, GetParam() == TEST_FOR_PHRASE || GetParam() == TEST_FOR_CYCLE);
		EXPECT_EQ(progress.cycleStarted, GetParam() == TEST_FOR_CYCLE);

		EXPECT_EQ(layerState.index, 0);
		referenceLayerState.index = 0;
		EXPECT_EQ(*state, referenceState);
	}
}

TEST_P(RatriligCoreProcessorAdvanceTest, advanceShouldDoNothingLowerLayerNotStarted) {
	if (GetParam() == TEST_FOR_CLUSTER) {
		GTEST_SKIP() << "This test is not applicable to clusters since there is no lower layer";
	}
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);

	int sizes[] = { 5, 8, 2 };

	for (int i = 0; i < 3; i++) {
		RatriligProcessorProgress progress;
		RatriligData data = populateDefaultRatriligData(0.f,
			GetParam() == TEST_FOR_CLUSTER ? sizes[i] : 5,
			GetParam() == TEST_FOR_PHRASE ? sizes[i] : 3,
			GetParam() == TEST_FOR_CYCLE ? sizes[i] : 2);
		RatriligCoreProcessor processor(mockRatriligChanceGenerator);

		std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
		RatriligCoreState referenceState = populateDefaultRatriligState();
		*state = populateDefaultRatriligState();
		processor.setState(state);

		// These indices should not be touched by the phrase advancement
		referenceState.clusterState.index = state->clusterState.index = 1;
		referenceState.phraseState.index = state->phraseState.index = 1;
		referenceState.cycleState.index = state->cycleState.index = 1;

		for (int j = 0; j < sizes[i]; j++) {
			// Set all the "progress started" property for the layer under test so that we can verify that it gets updated
			progress.phraseStarted = GetParam() == TEST_FOR_PHRASE;
			progress.cycleStarted = GetParam() == TEST_FOR_CYCLE;
			
			callAdvance(processor, data, progress);

			EXPECT_FALSE(progress.clusterStarted);
			EXPECT_FALSE(progress.cycleStarted);
			EXPECT_FALSE(progress.phraseStarted);

			// Nothing should have changed
			EXPECT_EQ(*state, referenceState);
		}
	}
}

INSTANTIATE_TEST_SUITE_P(RatriligCoreTestSuite, RatriligCoreProcessorAdvanceTest, testing::Values(TEST_FOR_CLUSTER, TEST_FOR_PHRASE, TEST_FOR_CYCLE));


struct RatriligDensityTestParams {
	RatriligDensityTestParams() {
		biasPosition = 0.f;
		biasAmount = 0.f;
		skipChance = 0.f;
		densityModifier = 0.f;
		parentLayerIndex = 0;
		parentLayerSize = 4;
		generatedSkipChance = 0.f;
		generatedDensityModifier = -1.f;
		expectedLayerEnabled = false;
		expectedLayerModifier = 0.f;
		expectedLayerBiasAmount = 0.f;
	}

	float biasPosition;
	float biasAmount;
	float skipChance;
	float densityModifier;
	int parentLayerIndex;
	int parentLayerSize;
	float generatedSkipChance;
	float generatedDensityModifier;
	bool expectedLayerEnabled;
	float expectedLayerModifier;
	float expectedLayerBiasAmount;
};

class RatriligCoreProcessorLayerDensityTest : public testing::TestWithParam<RatriligCoreTestTargets> {
	protected:
		void testDetermineDensity(RatriligDensityTestParams testParams) {
			// Set up the mocked chance generator
			std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();

			if (testParams.generatedSkipChance >= 0.f) {
				EXPECT_CALL(*mockRatriligChanceGenerator, generateSkipChance).Times(1).WillOnce(testing::Return(testParams.generatedSkipChance));
			} else {
				EXPECT_CALL(*mockRatriligChanceGenerator, generateSkipChance).Times(0);
			}
			if (testParams.generatedDensityModifier >= 0.f) {
				EXPECT_CALL(*mockRatriligChanceGenerator, generateDensityModifier).Times(1).WillOnce(testing::Return(testParams.generatedDensityModifier));
			} else {
				EXPECT_CALL(*mockRatriligChanceGenerator, generateDensityModifier).Times(0);
			}
			EXPECT_CALL(*mockRatriligChanceGenerator, generateTrigger).Times(0);

			// Prepeare the test data objects and main processor
			RatriligProcessorProgress progress;
			RatriligData data = populateDefaultRatriligData(0.f,
				3,
				GetParam() == TEST_FOR_CLUSTER ? testParams.parentLayerSize : 4,
				GetParam() == TEST_FOR_PHRASE ? testParams.parentLayerSize : 2);
			RatriligCoreProcessor processor(mockRatriligChanceGenerator);

			RatriligLayerData& layerData =
				GetParam() == TEST_FOR_CLUSTER ? data.clusterData :
				GetParam() == TEST_FOR_PHRASE ? data.phraseData : data.cycleData;

			layerData.skipChance = testParams.skipChance;
			layerData.densityModifier = testParams.densityModifier;

			progress.clusterStarted = GetParam() == TEST_FOR_CLUSTER;
			progress.phraseStarted = GetParam() == TEST_FOR_PHRASE;
			progress.cycleStarted = GetParam() == TEST_FOR_CYCLE;

			// Prepare the state before triggering processing, setting the cluster properties to other values then the expected ones to verify their updating.
			std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
			*state = populateDefaultRatriligState();
			RatriligCoreLayerState& layerState =
				GetParam() == TEST_FOR_CLUSTER ? state->clusterState :
				GetParam() == TEST_FOR_PHRASE ? state->phraseState :
				state->cycleState;

			layerState.enabled = !testParams.expectedLayerEnabled;
			layerState.densityModifier = 1024.f;
			processor.setState(state);
			
			// The expected state after the processing
			RatriligCoreState referenceState = *state;
			RatriligCoreLayerState& referenceLayerState =
				GetParam() == TEST_FOR_CLUSTER ? referenceState.clusterState :
				GetParam() == TEST_FOR_PHRASE ? referenceState.phraseState :
				referenceState.cycleState;

			referenceLayerState.enabled = testParams.expectedLayerEnabled;

			// Clusters and phrases have additional bias settings
			if (GetParam() == TEST_FOR_CLUSTER) {
				data.clusterData.biasPosition = testParams.biasPosition;
				data.clusterData.biasAmount = testParams.biasAmount;
				state->phraseState.index = testParams.parentLayerIndex;
				referenceState.clusterState.biasAmount = testParams.expectedLayerBiasAmount;
				referenceState.phraseState.index = testParams.parentLayerIndex;
			}
			else if (GetParam() == TEST_FOR_PHRASE) {
				data.phraseData.biasPosition = testParams.biasPosition;
				data.phraseData.biasAmount = testParams.biasAmount;
				state->cycleState.index = testParams.parentLayerIndex;
				referenceState.phraseState.biasAmount = testParams.expectedLayerBiasAmount;
				referenceState.cycleState.index = testParams.parentLayerIndex;
			}

			// Trigger processing
			callDetermineDensity(processor, data, progress);

			// Verify the result
			referenceLayerState.densityModifier = layerState.densityModifier; // The DensityModifier is based on a calculation, so to avoid rounding errors, we'll verify that afterwards with a 'near' check.
			EXPECT_EQ(*state, referenceState);
			EXPECT_NEAR(layerState.densityModifier, testParams.expectedLayerModifier, 0.0001f);
		}

		void callDetermineDensity(RatriligCoreProcessor& processor, RatriligData& data, RatriligProcessorProgress& progress) {
			if (GetParam() == TEST_FOR_CLUSTER) {
				processor.determineClusterDensity(data, progress);
 			} else if (GetParam() == TEST_FOR_PHRASE) {
				processor.determinePhraseDensity(data, progress);
 			} else if (GetParam() == TEST_FOR_CYCLE) {
				processor.determineCycleDensity(data, progress);
			}
		}
};

TEST_P(RatriligCoreProcessorLayerDensityTest, determineLayerDensityShouldDoNothingIfLayerNotStarted) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);

	RatriligProcessorProgress progress;
	RatriligData data = populateDefaultRatriligData(0.f, 3, 4, 2);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	RatriligCoreState referenceState = populateDefaultRatriligState();
	*state = populateDefaultRatriligState();
	processor.setState(state);

	// The starting of the other layers should have no impact
	progress.clusterStarted = GetParam() != TEST_FOR_CLUSTER;
	progress.phraseStarted = GetParam() != TEST_FOR_PHRASE;
	progress.cycleStarted = GetParam() != TEST_FOR_CYCLE;

	callDetermineDensity(processor, data, progress);
	EXPECT_EQ(*state, referenceState);
}

TEST_P(RatriligCoreProcessorLayerDensityTest, determineLayerDensityWithNoBiasLayerAndSkipChanceBelowThresholdShouldDisableLayer) {
	RatriligDensityTestParams testParams;

	// As long as the generated skip chance is below the cluster skip chance, the cluster should remain disabled
	testParams.skipChance = .75f;
	testParams.generatedSkipChance = .5f;
	testDetermineDensity(testParams);

	testParams.skipChance = .1001f;
	testParams.generatedSkipChance = .1f;
	testDetermineDensity(testParams);

	testParams.skipChance = .0001f;
	testParams.generatedSkipChance = 0.f;
	testDetermineDensity(testParams);

	testParams.skipChance = 1.f;
	testParams.generatedSkipChance = 0.f;
	testDetermineDensity(testParams);

	testParams.skipChance = 1.f;
	testParams.generatedSkipChance = .9999f;
	testDetermineDensity(testParams);
}

TEST_P(RatriligCoreProcessorLayerDensityTest, determineLayerDensityWithNoBiasAndSkipChanceEqualtoOrAboveThresholdShouldEnableLayer) {
	RatriligDensityTestParams testParams;

	testParams.expectedLayerEnabled = true;

	// Equal skip chance enables the cluster, but the cluster density modifier reduces the output modifier to 0
	testParams.skipChance = .1f;
	testParams.generatedSkipChance = .1f;
	testParams.generatedDensityModifier = .1f;
	testParams.densityModifier = 0.f;
	testParams.expectedLayerModifier = 0.f;
	testDetermineDensity(testParams);

	// Same as the previous test, but this time the generated density modifier reduces the output modifier to 0
	testParams.generatedDensityModifier = 0.f;
	testParams.densityModifier = .1f;
	testDetermineDensity(testParams);

	// Now both generated and cluster density modifiers are non-null, resulting in a non-null output modifier
	testParams.generatedDensityModifier = .4f;
	testParams.expectedLayerModifier = .4f * .1f;
	testDetermineDensity(testParams);

	// With a different set of generated and density modifiers
	testParams.generatedDensityModifier = .33f;
	testParams.densityModifier = .998f;
	testParams.expectedLayerModifier = .33f * .998f;
	testDetermineDensity(testParams);

	// Instead of an equal skip chance, the generated skip chance is above the cluster skip chance. Keep the (expected) density modifiers from the previous test
	testParams.skipChance = .5f;
	testParams.generatedSkipChance = .501f;
	testDetermineDensity(testParams);

	testParams.skipChance = .0f;
	testParams.generatedSkipChance = .501f;
	testDetermineDensity(testParams);

	testParams.skipChance = .999f;
	testParams.generatedSkipChance = 1.f;
	testDetermineDensity(testParams);
}

TEST_P(RatriligCoreProcessorLayerDensityTest, determineLayerDensityWithNonBiasedLayerAndSkipChanceBelowThresholdShouldDisableCluster) {
	if (GetParam() == TEST_FOR_CYCLE) {
		GTEST_SKIP() << "This test is not applicable to cycles since they have no bias";
	}

	RatriligDensityTestParams testParams;

	// The bias parameters: parent layer index, parent layer size and layer bias position
	std::tuple<int, int, float> bias[] = {
		// First four tuples with bias at the end (1.f), and the parent layer index and size not at the end
		std::make_tuple(0, 5, 1.f),
		std::make_tuple(1, 5, 1.f),
		std::make_tuple(2, 5, 1.f),
		std::make_tuple(3, 5, 1.f),
		// Then four tuples with bias at the start (0.f), and the parent layer index and size not at the start
		std::make_tuple(1, 5, 0.f),
		std::make_tuple(2, 5, 0.f),
		std::make_tuple(3, 5, 0.f),
		std::make_tuple(4, 5, 0.f),
		// And finally some diverse bias settings
		std::make_tuple(1, 4, 0.f),
		std::make_tuple(2, 6, 0.5f),
		std::make_tuple(0, 3, 0.34f),
		std::make_tuple(3, 4, 0.25f),
		std::make_tuple(1, 2, 0.45f),
		std::make_tuple(2, 9, 0.1f),
		std::make_tuple(3, 6, 0.75f),
		std::make_tuple(2, 3, 0.2f),
	};

	// Give the layer some bias (amount doesn't matter since the layer will not be biased)
	testParams.biasAmount = 0.5f;

	for (int i = 0; i < 16; i++) {
		testParams.parentLayerIndex = std::get<0>(bias[i]);
		testParams.parentLayerSize = std::get<1>(bias[i]);
		testParams.biasPosition = std::get<2>(bias[i]);

		// As long as the generated skip chance is below the layer skip chance, the layer should remain disabled
		testParams.skipChance = .75f;
		testParams.generatedSkipChance = .5f;
		testDetermineDensity(testParams);

		testParams.skipChance = .1001f;
		testParams.generatedSkipChance = .1f;
		testDetermineDensity(testParams);

		testParams.skipChance = .0001f;
		testParams.generatedSkipChance = 0.f;
		testDetermineDensity(testParams);

		testParams.skipChance = 1.f;
		testParams.generatedSkipChance = 0.f;
		testDetermineDensity(testParams);

		testParams.skipChance = 1.f;
		testParams.generatedSkipChance = .9999f;
		testDetermineDensity(testParams);
	}
}

TEST_P(RatriligCoreProcessorLayerDensityTest, determineLayerDensityWithNonBiasedLayerAndSkipChanceEqualtoOrAboveThresholdShouldEnableLayer) {
	if (GetParam() == TEST_FOR_CYCLE) {
		GTEST_SKIP() << "This test is not applicable to cycles since they have no bias";
	}

	RatriligDensityTestParams testParams;

	testParams.expectedLayerEnabled = true;

	// The bias parameters: parent layer index, parent layer size and layer bias position
	std::tuple<int, int, float> bias[] = {
		// First four tuples with bias at the end (1.f), and the parent index and size not at the end
		std::make_tuple(0, 5, 1.f),
		std::make_tuple(1, 5, 1.f),
		std::make_tuple(2, 5, 1.f),
		std::make_tuple(3, 5, 1.f),
		// Then four tuples with bias at the start (0.f), and the parent index and size not at the start
		std::make_tuple(1, 5, 0.f),
		std::make_tuple(2, 5, 0.f),
		std::make_tuple(3, 5, 0.f),
		std::make_tuple(4, 5, 0.f),
		// And finally some diverse bias settings
		std::make_tuple(1, 4, 0.f),
		std::make_tuple(2, 6, 0.5f),
		std::make_tuple(0, 3, 0.34f),
		std::make_tuple(3, 4, 0.25f),
		std::make_tuple(1, 2, 0.45f),
		std::make_tuple(2, 9, 0.1f),
		std::make_tuple(3, 6, 0.75f),
		std::make_tuple(2, 3, 0.2f),
	};

	// Give the cluster some bias (amount doesn't matter since the cluster will not be biased)
	testParams.biasAmount = 0.5f;

	for (int i = 0; i < 16; i++) {
		testParams.parentLayerIndex = std::get<0>(bias[i]);
		testParams.parentLayerSize = std::get<1>(bias[i]);
		testParams.biasPosition = std::get<2>(bias[i]);

		// Equal skip chance enables the cluster, but the cluster density modifier reduces the output modifier to 0
		testParams.skipChance = .1f;
		testParams.generatedSkipChance = .1f;
		testParams.generatedDensityModifier = .1f;
		testParams.densityModifier = 0.f;
		testParams.expectedLayerModifier = 0.f;
		testDetermineDensity(testParams);

		// Same as the previous test, but this time the generated density modifier reduces the output modifier to 0
		testParams.generatedDensityModifier = 0.f;
		testParams.densityModifier = .1f;
		testDetermineDensity(testParams);

		// Now both generated and cluster density modifiers are non-null, resulting in a non-null output modifier
		testParams.generatedDensityModifier = .4f;
		testParams.expectedLayerModifier = .4f * .1f;
		testDetermineDensity(testParams);

		// With a different set of generated and density modifiers
		testParams.generatedDensityModifier = .33f;
		testParams.densityModifier = .998f;
		testParams.expectedLayerModifier = .33f * .998f;
		testDetermineDensity(testParams);

		// Instead of an equal skip chance, the generated skip chance is above the cluster skip chance. Keep the (expected) density modifiers from the previous test
		testParams.skipChance = .5f;
		testParams.generatedSkipChance = .501f;
		testDetermineDensity(testParams);

		testParams.skipChance = .0f;
		testParams.generatedSkipChance = .501f;
		testDetermineDensity(testParams);

		testParams.skipChance = .999f;
		testParams.generatedSkipChance = 1.f;
		testDetermineDensity(testParams);
	}
}

TEST_P(RatriligCoreProcessorLayerDensityTest, determineLayerDensityWithBiasedLayerShouldAlwaysEnableLayer) {
	if (GetParam() == TEST_FOR_CYCLE) {
		GTEST_SKIP() << "This test is not applicable to cycles since they have no bias";
	}

	RatriligDensityTestParams testParams;

	testParams.expectedLayerEnabled = true;

	// The bias parameters: phrase index, phrase size, cluster bias position and cluster bias amount
	std::tuple<int, int, float, float> bias[] = {
		// First four tuples with bias at the end (1.f), with different bias amountss
		std::make_tuple(4, 5, 1.f, .1f),
		std::make_tuple(4, 5, 1.f, .33f),
		std::make_tuple(4, 5, 1.f, .67f),
		std::make_tuple(4, 5, 1.f, 1.f),
		// Then four tuples with bias at the start (0.f), with different bias amountss
		std::make_tuple(0, 5, 0.f, .15f),
		std::make_tuple(0, 5, 0.f, .25f),
		std::make_tuple(0, 5, 0.f, .75f),
		std::make_tuple(0, 5, 0.f, 1.f),
		// // And finally some diverse bias settings
		std::make_tuple(0, 4, 0.f, .1f),
		std::make_tuple(3, 6, 0.5f, .2f),
		std::make_tuple(1, 3, 0.34f, .3f),
		std::make_tuple(1, 4, 0.25f, .4f),
		std::make_tuple(0, 2, 0.45f, .5f),
		std::make_tuple(0, 9, 0.1f, .6f),
		std::make_tuple(4, 6, 0.75f, .7f),
		std::make_tuple(0, 3, 0.2f, .8f),
	};

	// For biased clusters, skip chance should not be determined.
	testParams.generatedSkipChance = -1.f;

	for (int i = 0; i < 16; i++) {
		testParams.parentLayerIndex = std::get<0>(bias[i]);
		testParams.parentLayerSize = std::get<1>(bias[i]);
		testParams.biasPosition = std::get<2>(bias[i]);
		testParams.biasAmount = std::get<3>(bias[i]);
		testParams.expectedLayerBiasAmount = std::get<3>(bias[i]);

		testParams.skipChance = .1f;
		testParams.generatedDensityModifier = .1f;
		testParams.densityModifier = 0.f;
		testParams.expectedLayerModifier = 0.f;
		testDetermineDensity(testParams);

		testParams.generatedDensityModifier = 0.f;
		testParams.densityModifier = .1f;
		testDetermineDensity(testParams);

		testParams.generatedDensityModifier = .4f;
		testParams.expectedLayerModifier = .4f * .1f;
		testDetermineDensity(testParams);

		testParams.generatedDensityModifier = .33f;
		testParams.densityModifier = .998f;
		testParams.expectedLayerModifier = .33f * .998f;
		testDetermineDensity(testParams);

		testParams.skipChance = .5f;
		testDetermineDensity(testParams);

		testParams.skipChance = .0f;
		testDetermineDensity(testParams);

		testParams.skipChance = .999f;
		testDetermineDensity(testParams);
	}
}

INSTANTIATE_TEST_SUITE_P(RatriligCoreTestSuite, RatriligCoreProcessorLayerDensityTest, testing::Values(TEST_FOR_CLUSTER, TEST_FOR_PHRASE, TEST_FOR_CYCLE));



TEST(RatriligCoreProcessor, determineDensityShouldUseBaseDensityModifier) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	processor.setState(state);

	RatriligProcessorProgress progress;
	RatriligData data = populateDefaultRatriligData(0.f, 0, 0, 0);
	
	float densities[] = { 0.f, .1f, .33f, .42f, .5f, .67f, .75f, .99f, 1.f };
	for (int i = 0; i < 9; i++) {
		data.density = densities[i];
		processor.determineDensity(data, progress);

		EXPECT_EQ(progress.density, densities[i]);
	}
}

TEST(RatriligCoreProcessor, determineDensityShouldUseClusterDensityModifier) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	processor.setState(state);

	RatriligProcessorProgress progress;
	RatriligData data = populateDefaultRatriligData(0.f, 0, 0, 0);
	
	float densities[] = { 0.f, .1f, .33f, .42f, .5f, .67f, .75f, .99f, 1.f };
	for (int i = 0; i < 9; i++) {
		state->clusterState.densityModifier = densities[i];
		processor.determineDensity(data, progress);

		EXPECT_EQ(progress.density, densities[i]);
	}
}

TEST(RatriligCoreProcessor, determineDensityShouldUsePhraseDensityModifier) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	processor.setState(state);

	RatriligProcessorProgress progress;
	RatriligData data = populateDefaultRatriligData(0.f, 0, 0, 0);
	
	float densities[] = { 0.f, .1f, .33f, .42f, .5f, .67f, .75f, .99f, 1.f };
	for (int i = 0; i < 9; i++) {
		state->phraseState.densityModifier = densities[i];
		processor.determineDensity(data, progress);

		EXPECT_EQ(progress.density, densities[i]);
	}
}

TEST(RatriligCoreProcessor, determineDensityShouldUseCycleDensityModifier) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	processor.setState(state);

	RatriligProcessorProgress progress;
	RatriligData data = populateDefaultRatriligData(0.f, 0, 0, 0);
	
	float densities[] = { 0.f, .1f, .33f, .42f, .5f, .67f, .75f, .99f, 1.f };
	for (int i = 0; i < 9; i++) {
		state->cycleState.densityModifier = densities[i];
		processor.determineDensity(data, progress);

		EXPECT_EQ(progress.density, densities[i]);
	}
}

TEST(RatriligCoreProcessor, determineDensityShouldUseClusterBiasAmount) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	processor.setState(state);

	RatriligProcessorProgress progress;
	RatriligData data = populateDefaultRatriligData(0.f, 0, 0, 0);
	
	float bias[] = { 0.f, .1f, .33f, .42f, .5f, .67f, .75f, .99f, 1.f };
	for (int i = 0; i < 9; i++) {
		state->clusterState.biasAmount = bias[i];
		processor.determineDensity(data, progress);

		EXPECT_EQ(progress.density, bias[i]);
	}
}

TEST(RatriligCoreProcessor, determineDensityShouldUsePhraseBiasAmount) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	processor.setState(state);

	RatriligProcessorProgress progress;
	RatriligData data = populateDefaultRatriligData(0.f, 0, 0, 0);
	
	float bias[] = { 0.f, .1f, .33f, .42f, .5f, .67f, .75f, .99f, 1.f };
	for (int i = 0; i < 9; i++) {
		state->phraseState.biasAmount = bias[i];
		processor.determineDensity(data, progress);

		EXPECT_EQ(progress.density, bias[i]);
	}
}

TEST(RatriligCoreProcessor, determineDensityShouldCombineBiasAmounts) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	processor.setState(state);

	RatriligProcessorProgress progress;
	RatriligData data = populateDefaultRatriligData(0.f, 0, 0, 0);
	
	// A list of bias amounts:
	// - the cluster bias
	// - the phrase bias
	// - the expected end bias (the higher of the two biases + half of the lower bias)
	float bias[][3] = {
		{ .1f, .2f, .2f + (.1f / 2) },
		{ .3f, .9f, .9f + (.3f / 2) },
		{ .5f, .2f, .5f + (.2f / 2) },
		{ .6f, .8f, .8f + (.6f / 2) },
		{ .05f, .06f, .6f + (.5f / 2) },
		{ .06f, .05f, .6f + (.5f / 2) },
	};
	for (int i = 0; i < 1; i++) {
		state->clusterState.biasAmount = bias[i][0];
		state->phraseState.biasAmount = bias[i][1];
		processor.determineDensity(data, progress);

		EXPECT_NEAR(progress.density, bias[i][2], 0.0001f);
	}
}

TEST(RatriligCoreProcessor, determineDensityShouldCombineAllFactors) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	processor.setState(state);

	RatriligProcessorProgress progress;
	RatriligData data = populateDefaultRatriligData(0.f, 0, 0, 0);
	
	// A list of bias amounts:
	// - the cluster bias
	// - the phrase bias
	// - the expected end bias (the higher of the two biases + half of the lower bias)
	float bias[][3] = {
		{ .1f, .2f, .2f + (.1f / 2) },
		{ .3f, .9f, .9f + (.3f / 2) },
		{ .5f, .2f, .5f + (.2f / 2) },
		{ .6f, .8f, .8f + (.6f / 2) },
		{ .05f, .06f, .6f + (.5f / 2) },
		{ .06f, .05f, .6f + (.5f / 2) },
	};
	float densities[][4] = {
		{ 1.f, .1f, .3f, .5f },
		{ 0.f, .2f, .25f, .4f },
		{ .6f, .0f, .35f, .6f },
		{ .01f, .05f, .0f, .01f },
		{ .02f, .6f, .33f, .0f },
		{ .5f, 1.f, .2f, .02f },
	};
	for (int i = 0; i < 1; i++) {
		data.density = densities[i][0];
		state->clusterState.densityModifier = densities[i][1];
		state->phraseState.densityModifier = densities[i][2];
		state->cycleState.densityModifier = densities[i][3];

		state->clusterState.biasAmount = bias[i][0];
		state->phraseState.biasAmount = bias[i][1];

		processor.determineDensity(data, progress);

		EXPECT_NEAR(progress.density, densities[i][0] + densities[i][1] + densities[i][2] + densities[i][3] + bias[i][2], 0.0001f);
	}
}

TEST(RatriligCoreProcessor, determineHighShouldNotSetHighIfClusterDisabled) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	state->clusterState.enabled = false;
	state->phraseState.enabled = true;
	state->cycleState.enabled = true;
	processor.setState(state);

	RatriligProcessorProgress progress;

	processor.determineHigh(progress);

	EXPECT_FALSE(state->high);
	EXPECT_EQ(progress.chance, 0.f);
}

TEST(RatriligCoreProcessor, determineHighShouldNotSetHighIfPhraseDisabled) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	state->clusterState.enabled = true;
	state->phraseState.enabled = false;
	state->cycleState.enabled = true;
	processor.setState(state);

	RatriligProcessorProgress progress;

	processor.determineHigh(progress);

	EXPECT_FALSE(state->high);
	EXPECT_EQ(progress.chance, 0.f);
}

TEST(RatriligCoreProcessor, determineHighShouldNotSetHighIfCycleDisabled) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	expectChanceGeneratorInvocations(*mockRatriligChanceGenerator, 0, 0, 0);
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	state->clusterState.enabled = true;
	state->phraseState.enabled = true;
	state->cycleState.enabled = false;
	processor.setState(state);

	RatriligProcessorProgress progress;

	processor.determineHigh(progress);

	EXPECT_FALSE(state->high);
	EXPECT_EQ(progress.chance, 0.f);
}

TEST(RatriligCoreProcessor, determineHighShouldNotSetHighIfChanceAbove) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	state->clusterState.enabled = true;
	state->phraseState.enabled = true;
	state->cycleState.enabled = true;
	processor.setState(state);

	RatriligProcessorProgress progress;

	// A list of expected and generated skip chances
	float chances[][2] = {
		{ .5f, .75f },
		{ .1f, .1001f },
		{ 0.f, .0001f },
		{ 0.f, 1.f },
		{ .9999f, 1.f }
	};

	EXPECT_CALL(*mockRatriligChanceGenerator, generateSkipChance).Times(0);
	EXPECT_CALL(*mockRatriligChanceGenerator, generateDensityModifier).Times(0);
	EXPECT_CALL(*mockRatriligChanceGenerator, generateTrigger).Times(5)
		.WillOnce(testing::Return(chances[0][1]))
		.WillOnce(testing::Return(chances[1][1]))
		.WillOnce(testing::Return(chances[2][1]))
		.WillOnce(testing::Return(chances[3][1]))
		.WillOnce(testing::Return(chances[4][1]));

	for (int i = 0; i < 5; i++) {
		progress.density = chances[i][0];
		processor.determineHigh(progress);

		EXPECT_FALSE(state->high);
		EXPECT_EQ(progress.chance, chances[i][1]);
	}
}

TEST(RatriligCoreProcessor, determineHighShouldSetHighIfChanceEqualOrBelowDensity) {
	std::shared_ptr<MockRatriligChanceGenerator> mockRatriligChanceGenerator = std::make_shared<MockRatriligChanceGenerator>();
	RatriligCoreProcessor processor(mockRatriligChanceGenerator);

	std::shared_ptr<RatriligCoreState> state = std::make_shared<RatriligCoreState>();
	*state = populateDefaultRatriligState();
	state->clusterState.enabled = true;
	state->phraseState.enabled = true;
	state->cycleState.enabled = true;
	processor.setState(state);

	RatriligProcessorProgress progress;

	// A list of expected and generated skip chances
	float chances[][2] = {
		{ .75f, .5f },
		{ .1001f, .1f },
		{ .0001f, 0.f },
		{ 1.f, 0.f },
		{ .25f, .25f },
		{ .33f, .33f },
		{ .69f, .69f },
		{ 1.f, 1.f },
	};

	EXPECT_CALL(*mockRatriligChanceGenerator, generateSkipChance).Times(0);
	EXPECT_CALL(*mockRatriligChanceGenerator, generateDensityModifier).Times(0);
	EXPECT_CALL(*mockRatriligChanceGenerator, generateTrigger).Times(8)
		.WillOnce(testing::Return(chances[0][1]))
		.WillOnce(testing::Return(chances[1][1]))
		.WillOnce(testing::Return(chances[2][1]))
		.WillOnce(testing::Return(chances[3][1]))
		.WillOnce(testing::Return(chances[4][1]))
		.WillOnce(testing::Return(chances[5][1]))
		.WillOnce(testing::Return(chances[6][1]))
		.WillOnce(testing::Return(chances[7][1]));

	for (int i = 0; i < 8; i++) {
		progress.density = chances[i][0];
		processor.determineHigh(progress);

		EXPECT_TRUE(state->high);
		EXPECT_EQ(progress.chance, chances[i][1]);
	}
}
