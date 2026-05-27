#include <gmock/gmock.h>
#include "core/ratrilig-core.hpp"

struct MockRatriligCoreListener : RatriligCoreListener {
	MOCK_METHOD(void, clusterStateChanged, (int, bool, float, float), (override));
	MOCK_METHOD(void, phraseStateChanged, (int, bool, float, float), (override));
	MOCK_METHOD(void, cycleStateChanged, (int, bool, float), (override));

	MOCK_METHOD(void, valueChanged, (int, int, int, int, float, float, bool), (override));
	MOCK_METHOD(void, clusterStarted, (int), (override));
	MOCK_METHOD(void, phraseStarted, (int), (override));
	MOCK_METHOD(void, cycleStarted, (int), (override));
};

struct MockRatriligCoreProcessor : RatriligCoreProcessor {
	MockRatriligCoreProcessor() : RatriligCoreProcessor({}) {};

	MOCK_METHOD(void, setState, (std::shared_ptr<RatriligCoreState>), (override));

	MOCK_METHOD(void, advanceCluster, (RatriligData&, RatriligProcessorProgress&), (override));
	MOCK_METHOD(void, advancePhrase, (RatriligData&, RatriligProcessorProgress&), (override));
	MOCK_METHOD(void, advanceCycle, (RatriligData&, RatriligProcessorProgress&), (override));

	MOCK_METHOD(void, determineClusterDensity, (RatriligData&, RatriligProcessorProgress&), (override));
	MOCK_METHOD(void, determinePhraseDensity, (RatriligData&, RatriligProcessorProgress&), (override));
	MOCK_METHOD(void, determineCycleDensity, (RatriligData&, RatriligProcessorProgress&), (override));

	MOCK_METHOD(void, determineDensity, (RatriligData&, RatriligProcessorProgress&), (override));
	MOCK_METHOD(void, determineHigh, (RatriligProcessorProgress&), (override));
};

TEST(RatriligCore, processShouldCallProcessorInOrder) {
	MockRatriligCoreListener mockListener;
	std::shared_ptr<MockRatriligCoreProcessor> mockProcessor = std::make_shared<MockRatriligCoreProcessor>();

	RatriligCore ratriligCore(69, &mockListener, mockProcessor);
	RatriligData data;

	// Value updates should be passed on
	EXPECT_CALL(mockListener, valueChanged).Times(1);
	EXPECT_CALL(mockListener, clusterStateChanged).Times(1);
	EXPECT_CALL(mockListener, phraseStateChanged).Times(1);
	EXPECT_CALL(mockListener, cycleStateChanged).Times(1);
	// Nothing should be notified as "started"
	EXPECT_CALL(mockListener, clusterStarted).Times(0);
	EXPECT_CALL(mockListener, phraseStarted).Times(0);
	EXPECT_CALL(mockListener, cycleStarted).Times(0);

	{
		testing::InSequence inSequence;

		RatriligProcessorProgress* firstProgress = nullptr;
		EXPECT_CALL(*mockProcessor, advanceCluster(testing::Ref(data), testing::_)).Times(1).WillOnce(testing::WithArg<1>(testing::Invoke([&](RatriligProcessorProgress& arg) {
			firstProgress = &arg;
		})));
		EXPECT_CALL(*mockProcessor, advancePhrase(testing::Ref(data), testing::Truly([&](RatriligProcessorProgress& arg) {
			return &arg == firstProgress;
		}))).Times(1);
		EXPECT_CALL(*mockProcessor, advanceCycle(testing::Ref(data), testing::Truly([&](RatriligProcessorProgress& arg) {
			return &arg == firstProgress;
		}))).Times(1);
		EXPECT_CALL(*mockProcessor, determineClusterDensity(testing::Ref(data), testing::Truly([&](RatriligProcessorProgress& arg) {
			return &arg == firstProgress;
		}))).Times(1);
		EXPECT_CALL(*mockProcessor, determinePhraseDensity(testing::Ref(data), testing::Truly([&](RatriligProcessorProgress& arg) {
			return &arg == firstProgress;
		}))).Times(1);
		EXPECT_CALL(*mockProcessor, determineCycleDensity(testing::Ref(data), testing::Truly([&](RatriligProcessorProgress& arg) {
			return &arg == firstProgress;
		}))).Times(1);
		EXPECT_CALL(*mockProcessor, determineDensity(testing::Ref(data), testing::Truly([&](RatriligProcessorProgress& arg) {
			return &arg == firstProgress;
		}))).Times(1);
		EXPECT_CALL(*mockProcessor, determineHigh(testing::Truly([&](RatriligProcessorProgress& arg) {
			return &arg == firstProgress;
		}))).Times(1);
	};

	ratriligCore.process(data);
}

TEST(RatriligCore, processShouldNotifyClusterStart) {
	testing::NiceMock<MockRatriligCoreListener> mockListener;
	std::shared_ptr<testing::NiceMock<MockRatriligCoreProcessor>> mockProcessor = std::make_shared<testing::NiceMock<MockRatriligCoreProcessor>>();

	RatriligCore ratriligCore(69, &mockListener, mockProcessor);
	RatriligData data;

	EXPECT_CALL(mockListener, clusterStarted).Times(1);
	EXPECT_CALL(mockListener, phraseStarted).Times(0);
	EXPECT_CALL(mockListener, cycleStarted).Times(0);

	EXPECT_CALL(*mockProcessor, advanceCluster(testing::Ref(data), testing::_)).Times(1).WillOnce(testing::WithArg<1>(testing::Invoke([&](RatriligProcessorProgress& arg) {
		arg.clusterStarted = true;
	})));

	ratriligCore.process(data);
}

TEST(RatriligCore, processShouldNotifyPhraseStart) {
	testing::NiceMock<MockRatriligCoreListener> mockListener;
	std::shared_ptr<testing::NiceMock<MockRatriligCoreProcessor>> mockProcessor = std::make_shared<testing::NiceMock<MockRatriligCoreProcessor>>();

	RatriligCore ratriligCore(69, &mockListener, mockProcessor);
	RatriligData data;

	EXPECT_CALL(mockListener, clusterStarted).Times(0);
	EXPECT_CALL(mockListener, phraseStarted).Times(1);
	EXPECT_CALL(mockListener, cycleStarted).Times(0);

	EXPECT_CALL(*mockProcessor, advanceCluster(testing::Ref(data), testing::_)).Times(1).WillOnce(testing::WithArg<1>(testing::Invoke([&](RatriligProcessorProgress& arg) {
		arg.phraseStarted = true;
	})));

	ratriligCore.process(data);
}

TEST(RatriligCore, processShouldNotifyCycleStart) {
	testing::NiceMock<MockRatriligCoreListener> mockListener;
	std::shared_ptr<testing::NiceMock<MockRatriligCoreProcessor>> mockProcessor = std::make_shared<testing::NiceMock<MockRatriligCoreProcessor>>();

	RatriligCore ratriligCore(69, &mockListener, mockProcessor);
	RatriligData data;

	EXPECT_CALL(mockListener, clusterStarted).Times(0);
	EXPECT_CALL(mockListener, phraseStarted).Times(0);
	EXPECT_CALL(mockListener, cycleStarted).Times(1);

	EXPECT_CALL(*mockProcessor, advanceCluster(testing::Ref(data), testing::_)).Times(1).WillOnce(testing::WithArg<1>(testing::Invoke([&](RatriligProcessorProgress& arg) {
		arg.cycleStarted = true;
	})));

	ratriligCore.process(data);
}

TEST(RatriligCore, processShouldNotifyValueChangesSetOne) {
	testing::NiceMock<MockRatriligCoreListener> mockListener;
	std::shared_ptr<testing::NiceMock<MockRatriligCoreProcessor>> mockProcessor = std::make_shared<testing::NiceMock<MockRatriligCoreProcessor>>();

	RatriligCoreState* state = nullptr;
	EXPECT_CALL(*mockProcessor, setState(testing::_)).Times(1).WillOnce(testing::WithArg<0>(testing::Invoke([&](std::shared_ptr<RatriligCoreState> arg) {
		state = arg.get();
	})));

	RatriligCore ratriligCore(69, &mockListener, mockProcessor);
	RatriligData data;

	EXPECT_CALL(*mockProcessor, advanceCluster(testing::Ref(data), testing::_)).Times(1).WillOnce(testing::WithArg<1>(testing::Invoke([&](RatriligProcessorProgress& arg) {
		state->cycleState.index = 1.f;
		state->phraseState.index = 2.f;
		state->clusterState.index = 3.f;
		arg.density = .4f;
		arg.chance = 5.f;
		state->high = true;

		state->clusterState.enabled = true;
		state->clusterState.densityModifier = 6.f;
		state->clusterState.biasAmount = 7.f;

		state->phraseState.enabled = false;
		state->phraseState.densityModifier = 8.f;
		state->phraseState.biasAmount = 9.f;

		state->cycleState.enabled = true;
		state->cycleState.densityModifier = 10.f;
	})));

	EXPECT_CALL(mockListener, valueChanged(69, 1.f, 2.f, 3.f, .4f, 5.f, true)).Times(1);
	EXPECT_CALL(mockListener, clusterStateChanged(69, true, 6.f, 7.f)).Times(1);
	EXPECT_CALL(mockListener, phraseStateChanged(69, false, 8.f, 9.f)).Times(1);
	EXPECT_CALL(mockListener, cycleStateChanged(69, true, 10.f)).Times(1);

	ratriligCore.process(data);

	EXPECT_TRUE(ratriligCore.isHigh());
}

TEST(RatriligCore, processShouldNotifyValueChangesSetTwo) {
	testing::NiceMock<MockRatriligCoreListener> mockListener;
	std::shared_ptr<testing::NiceMock<MockRatriligCoreProcessor>> mockProcessor = std::make_shared<testing::NiceMock<MockRatriligCoreProcessor>>();

	RatriligCoreState* state = nullptr;
	EXPECT_CALL(*mockProcessor, setState(testing::_)).Times(1).WillOnce(testing::WithArg<0>(testing::Invoke([&](std::shared_ptr<RatriligCoreState> arg) {
		state = arg.get();
	})));

	RatriligCore ratriligCore(69, &mockListener, mockProcessor);
	RatriligData data;

	EXPECT_CALL(*mockProcessor, advanceCluster(testing::Ref(data), testing::_)).Times(1).WillOnce(testing::WithArg<1>(testing::Invoke([&](RatriligProcessorProgress& arg) {
		state->cycleState.index = 1.f;
		state->phraseState.index = 2.f;
		state->clusterState.index = 3.f;
		arg.density = 4.f; // Will be limited to 1.f max
		arg.chance = 5.f;
		state->high = false;

		state->clusterState.enabled = false;
		state->clusterState.densityModifier = 6.f;
		state->clusterState.biasAmount = 7.f;

		state->phraseState.enabled = true;
		state->phraseState.densityModifier = 8.f;
		state->phraseState.biasAmount = 9.f;

		state->cycleState.enabled = false;
		state->cycleState.densityModifier = 10.f;
	})));

	EXPECT_CALL(mockListener, valueChanged(69, 1.f, 2.f, 3.f, 1.f, 5.f, false)).Times(1);
	EXPECT_CALL(mockListener, clusterStateChanged(69, false, 6.f, 7.f)).Times(1);
	EXPECT_CALL(mockListener, phraseStateChanged(69, true, 8.f, 9.f)).Times(1);
	EXPECT_CALL(mockListener, cycleStateChanged(69, false, 10.f)).Times(1);

	ratriligCore.process(data);

	EXPECT_FALSE(ratriligCore.isHigh());
}

TEST(RatriligCore, resetShouldResetStateAndNotifyUpdate) {
	testing::NiceMock<MockRatriligCoreListener> mockListener;
	std::shared_ptr<testing::NiceMock<MockRatriligCoreProcessor>> mockProcessor = std::make_shared<testing::NiceMock<MockRatriligCoreProcessor>>();

	RatriligCoreState* state = nullptr;
	EXPECT_CALL(*mockProcessor, setState(testing::_)).Times(1).WillOnce(testing::WithArg<0>(testing::Invoke([&](std::shared_ptr<RatriligCoreState> arg) {
		state = arg.get();

		state->clusterState.index = 5;
		state->phraseState.index = 5;
		state->cycleState.index = 5;

		state->clusterState.enabled = false;
		state->phraseState.enabled = false;
		state->cycleState.enabled = false;

		state->high = true;
	})));

	EXPECT_CALL(mockListener, valueChanged(69,  0.f, 0.f, 0.f, 0.f, 0.f, false)).Times(1);
	EXPECT_CALL(mockListener, clusterStateChanged(69, true, 0.f, 0.f));
	EXPECT_CALL(mockListener, phraseStateChanged(69, true, 0.f, 0.f));
	EXPECT_CALL(mockListener, cycleStateChanged(69, true, 0.f));

	RatriligCore ratriligCore(69, &mockListener, mockProcessor);

	ratriligCore.reset();

	EXPECT_EQ(state->clusterState.index, RATRILIG_INDEX_WRAP_ON_ADVANCE);
	EXPECT_EQ(state->phraseState.index, RATRILIG_INDEX_WRAP_ON_ADVANCE);
	EXPECT_EQ(state->cycleState.index, RATRILIG_INDEX_WRAP_ON_ADVANCE);

	EXPECT_TRUE(state->clusterState.enabled);
	EXPECT_TRUE(state->phraseState.enabled);
	EXPECT_TRUE(state->cycleState.enabled);

	EXPECT_FALSE(state->high);
	EXPECT_FALSE(ratriligCore.isHigh());
}
