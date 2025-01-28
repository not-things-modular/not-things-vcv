#include <gtest/gtest.h>
#include <cmath>
#include <random>
#include <list>


/* Test the two limitValue methods */
extern float limitValueLoop(float value, float lowerLimit, float upperLimit);
extern float limitValueIf(float value, float lowerLimit, float upperLimit);

#define RNG_FLOAT_PRECISION 10000


/** Utility method for getting the factional part of a value, allowing for float rounding errors */
float fractionalPart(float value) {
    float frac = value - std::floor(value);
    return (frac < 0.000001f || frac > 0.999999f) ? 0.0f : frac;
}

class LimitValueParameterizedTest : public testing::TestWithParam<std::function<float(float, float, float)>> {
    std::function<float(float, float, float)> m_limitValueFunction;

    void SetUp() override {
        m_limitValueFunction = GetParam();
    }

    public:
        std::function<float(float, float, float)> getLimitValueFunction() {
            return m_limitValueFunction;
        }
};

TEST_P(LimitValueParameterizedTest, ValueWithinLimitsDoesNothing) {
    EXPECT_FLOAT_EQ(getLimitValueFunction()(2.34f, 2.f, 4.1f), 2.34f);
    EXPECT_FLOAT_EQ(getLimitValueFunction()(2.34f, 2.f, 4.0f), 2.34f);
    EXPECT_FLOAT_EQ(getLimitValueFunction()(2.34f, 2.f, 3.1f), 2.34f);
    EXPECT_FLOAT_EQ(getLimitValueFunction()(2.34f, 2.f, 3.f), 2.34f);
    EXPECT_FLOAT_EQ(getLimitValueFunction()(2.34f, 2.f, 2.9f), 2.34f);
    EXPECT_FLOAT_EQ(getLimitValueFunction()(2.34f, 2.f, 2.5f), 2.34f);
}

TEST_P(LimitValueParameterizedTest, ValueOnLimitsDoesNothing) {
    EXPECT_FLOAT_EQ(getLimitValueFunction()(1.23f, 1.23f, 3.1f), 1.23f);
    EXPECT_FLOAT_EQ(getLimitValueFunction()(1.23f, 2.f, 2.23f), 2.23f);
}

TEST_P(LimitValueParameterizedTest, ValueBelowLimitsButWillNotGoOverUpperLimitLimitsUpward) {
    // There is more then one volt between lower and upper limit, so moving the value upwards never goes above upper limit
    EXPECT_FLOAT_EQ(getLimitValueFunction()(1.23f, 3.f, 5.1f), 3.23f);
    // There is one volt between lower and upper limit, so moving the value upwards never goes above upper limit
    EXPECT_FLOAT_EQ(getLimitValueFunction()(1.23f, 3.f, 4.f), 3.23f);
    // There is less than one volt between the limits, but moving the value upwards will stay below upper limit
    EXPECT_FLOAT_EQ(getLimitValueFunction()(1.23f, 3.f, 3.24f), 3.23f);
    // There is less than one volt between the limits, but moving the value upwards ends up exactly at the upper limit
    EXPECT_FLOAT_EQ(getLimitValueFunction()(1.23f, 3.f, 3.23f), 3.23f);
    // The fractal part of the original value is lower then the fractal part of the lower limit, so just adding the difference
    // between the non-fractal parts of them to the original value will still end up (just) below the lower limit, and an
    // additional octave will have to be added (i.e. this test is specific for the limitValueIf version)
    EXPECT_FLOAT_EQ(getLimitValueFunction()(1.22f, 3.23f, 4.23f), 4.22f);
}

TEST_P(LimitValueParameterizedTest, ValueBelowLimitsAndGoesAboveUpperLimitLimitsDownward) {
    EXPECT_FLOAT_EQ(getLimitValueFunction()(0.23f, 2.f, 2.22f), 1.23f);
    EXPECT_FLOAT_EQ(getLimitValueFunction()(1.23f, 2.f, 2.22f), 1.23f);
}

TEST_P(LimitValueParameterizedTest, ValueAboveLimitsButWillNotGoUnderLowerLimitLimitsDownward) {
    // There is more then one volt between lower and upper limit, so moving the value downward never goes below lower limit
    EXPECT_FLOAT_EQ(getLimitValueFunction()(5.23f, 2.f, 3.22f), 2.23f);
    // There is one volt between lower and upper limit, so moving the value downward never goes below lower limit
    EXPECT_FLOAT_EQ(getLimitValueFunction()(5.23f, 2.f, 3.f), 2.23f);
    // There is less than one volt between the limits, but moving the value downward will stay above lower limit
    EXPECT_FLOAT_EQ(getLimitValueFunction()(5.23f, 2.f, 3.f), 2.23f);
    // There is less than one volt between the limits, but moving the value downwards ends up exactly at the lower limit
    EXPECT_FLOAT_EQ(getLimitValueFunction()(5.23f, 2.23f, 3.f), 2.23f);
}

TEST_P(LimitValueParameterizedTest, ValueAboveLimitsAndGoesBelowLowerLimitLimitsDownward) {
    EXPECT_FLOAT_EQ(getLimitValueFunction()(5.23f, 2.f, 2.22f), 1.23f);
    EXPECT_FLOAT_EQ(getLimitValueFunction()(2.23f, 2.f, 2.22f), 1.23f);
}

INSTANTIATE_TEST_SUITE_P(
    LimitValueTest,
    LimitValueParameterizedTest,
    testing::Values(limitValueLoop, limitValueIf)
);

TEST(LimitValueTestBurst, BurstRandomValuesAndLimits) {
    int burstCount = 1000000;
    float precision = 0.00002f;
    std::mt19937 rng = std::mt19937(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> dist(-5.f, 5.f);
    std::list<float> lowerLimits;
    std::list<float> upperLimits;
    std::list<float> values;

    for (int i = 0; i < burstCount; i++) {
        float lower = round(dist(rng) / RNG_FLOAT_PRECISION) * RNG_FLOAT_PRECISION;
        float upper = round(dist(rng) / RNG_FLOAT_PRECISION) * RNG_FLOAT_PRECISION;
        float value = round(dist(rng) / RNG_FLOAT_PRECISION) * RNG_FLOAT_PRECISION;

        lowerLimits.push_back(lower);
        upperLimits.push_back(upper);
        values.push_back(value);

        // Every ten items, insert an entry where the value equals the lowerLimit, and one where the value equals the upper limit
        if (burstCount % 10 == 0) {
            lowerLimits.push_back(lower);
            upperLimits.push_back(upper);
            values.push_back(upper);

            lowerLimits.push_back(lower);
            upperLimits.push_back(upper);
            values.push_back(lower);
        }
    }

    while (values.size() > 0) {
        float lowerLimit = lowerLimits.back();
        float upperLimit = upperLimits.back();
        float value = values.back();

        float loopResult = limitValueLoop(value, lowerLimit, upperLimit);
        float ifResult = limitValueIf(value, lowerLimit, upperLimit);
        EXPECT_NEAR(loopResult, ifResult, precision) << "loopResult and ifResult differ: value = " << value << "; lowerLimit = " << lowerLimit << "; upperLimit = " << upperLimit << "; loopResult = " << loopResult << "; ifResult = " << ifResult;

        // Removing the octave information (i.e. everything non-decimal), the value should still be on the same note.
        // Do take into account that it may have gone between negative vs positive, so add 10.0f to make it all-positive.
        EXPECT_NEAR(fractionalPart(value + 10.f), fractionalPart(loopResult + 10.f), precision);

        // Apply the float deviation allowance that's also applied in solim-core.cpp
        lowerLimit = lowerLimit - 0.00001f;
        upperLimit = upperLimit + 0.00001f;

        // Lower limit is below upper limit
        if (lowerLimit < upperLimit) {
            if (lowerLimit + 1 <= upperLimit) {
                // There is one volt difference between lower and upper limit, so the limited value ends up between them
                EXPECT_GE(loopResult, lowerLimit);
                EXPECT_LE(loopResult, upperLimit);
            } else if ((::fmodf(loopResult, 1.f) > ::fmodf(lowerLimit, 1.f)) && (::fmodf(loopResult, 1.f) < ::fmodf(upperLimit, 1.f))) {
                // The upward limiting moves the value above the lower limit, but below the upper limit
                EXPECT_GT(loopResult, lowerLimit);
                EXPECT_LT(loopResult, upperLimit);
            } else {
                // There is less than one volt between the two limits, and moving the value upwards moves it beyond the upper limit.
                // The upper limit is applied last, so the limit should be just below it.
                EXPECT_GT(loopResult, lowerLimit - 1.f);
                EXPECT_LE(loopResult, upperLimit);
                EXPECT_GE(loopResult, upperLimit - 1.f);
            }
        }
        // Lower limit is above upper limit
        else {
            // The result should always be below upperLimit, but no further than 1.f below it.
            if (upperLimit > lowerLimit) {
                EXPECT_LT(loopResult, lowerLimit);
            } else {
                EXPECT_LE(loopResult, lowerLimit);
            }
            EXPECT_LE(loopResult, upperLimit);
            EXPECT_GE(loopResult, upperLimit - 1.f);
        }

        lowerLimits.pop_back();
        upperLimits.pop_back();
        values.pop_back();
    }
}
