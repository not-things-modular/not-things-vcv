#include <gmock/gmock.h>
#include "core/ramelig-core.hpp"
#include <algorithm>
#include <cmath>

#define NOTE_C     (0.f)
#define NOTE_C_S   (1.f / 12)
#define NOTE_D     (2.f / 12)
#define NOTE_D_S   (3.f / 12)
#define NOTE_E     (4.f / 12)
#define NOTE_F     (5.f / 12)
#define NOTE_F_S   (6.f / 12)
#define NOTE_G     (7.f / 12)
#define NOTE_G_S   (8.f / 12)
#define NOTE_A     (9.f / 12)
#define NOTE_A_S   (10.f / 12)
#define NOTE_B     (11.f / 12)

std::array<float, 12> NOTES = {  NOTE_C, NOTE_C_S, NOTE_D, NOTE_D_S, NOTE_E, NOTE_F, NOTE_F_S, NOTE_G, NOTE_G_S, NOTE_A, NOTE_A_S, NOTE_B };

TEST(RameligCoreScale, quantizeToVoltageShouldUseAssignedScaleAndProvidedOctave) {
	RameligScale scale;

	scale.setScale({ 0, 3, 6, 9, 11 });

	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(0, 0)), NOTE_C + 0.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(2, 0)), NOTE_C + 2.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(-2, 0)), NOTE_C - 2.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(0, 1)), NOTE_D_S + 0.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(4, 1)), NOTE_D_S + 4.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(-4, 1)), NOTE_D_S - 4.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(0, 2)), NOTE_F_S + 0.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(0, 3)), NOTE_A + 0.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(0, 4)), NOTE_B + 0.f, 0.0001f);

	scale.setScale({ 1, 4, 5, 8, 10 });

	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(0, 0)), NOTE_C_S + 0.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(2, 0)), NOTE_C_S + 2.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(-2, 0)), NOTE_C_S - 2.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(0, 1)), NOTE_E + 0.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(4, 1)), NOTE_E + 4.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(-4, 1)), NOTE_E - 4.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(0, 2)), NOTE_F + 0.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(0, 3)), NOTE_G_S + 0.f, 0.0001f);
	EXPECT_NEAR(scale.quantizedToVoltage(std::make_pair(0, 4)), NOTE_A_S + 0.f, 0.0001f);
}

TEST(RameligCoreScale, quantizeShouldQuantizeToNotesInZeroToOneVoltRange) {
	RameligScale scale;

	for (int i = 0; i < 12; i++) {
		scale.setScale({ i });

		for (float j = -10.f; j <= 10.f; j += 0.1f) {
			std::pair<int, int> quantizedIndex = scale.quantize(j, 0.f, 0.999999f);
			float quantizedVoltage = scale.quantizedToVoltage(quantizedIndex);
			EXPECT_EQ(quantizedIndex, std::make_pair(0, 0));
			EXPECT_NEAR(quantizedVoltage, (float) i / 12, 0.0001f);
		}
	}
}

TEST(RameligCoreScale, quantizeWithDefaultScaleShouldQuantizeToC) {
	RameligScale scale;

	for (float j = -10.f; j <= 10.f; j += 0.1f) {
		std::pair<int, int> quantizedIndex = scale.quantize(j, 0.f, 0.999999f);
		float quantizedVoltage = scale.quantizedToVoltage(quantizedIndex);
		EXPECT_EQ(quantizedIndex, std::make_pair(0, 0));
		EXPECT_NEAR(quantizedVoltage, 0.f, 0.0001f);
	}
}

TEST(RameligCoreScale, quantizeWithEmptyScaleShouldQuantizeToC) {
	RameligScale scale;

	scale.setScale({});

	for (float j = -10.f; j <= 10.f; j += 0.1f) {
		std::pair<int, int> quantizedIndex = scale.quantize(j, 0.f, 0.999999f);
		float quantizedVoltage = scale.quantizedToVoltage(quantizedIndex);
		EXPECT_EQ(quantizedIndex, std::make_pair(0, 0));
		EXPECT_NEAR(quantizedVoltage, 0.f, 0.0001f);
	}
}

TEST(RameligCoreScale, quantizeShouldQuantizeToNotesInMinusOneToZeroRange) {
	RameligScale scale;

	for (int i = 0; i < 12; i++) {
		scale.setScale({ i });

		for (float j = -10.f; j <= 10.f; j += 0.1f) {
			std::pair<int, int> quantizedIndex = scale.quantize(j, -1.f, -0.000001f);
			float quantizedVoltage = scale.quantizedToVoltage(quantizedIndex);
			EXPECT_EQ(quantizedIndex, std::make_pair(-1, 0));
			EXPECT_NEAR(quantizedVoltage, (float) i / 12 - 1, 0.0001f);
		}
	}
}

TEST(RameligCoreScale, quantizeShouldQuantizeToNotesInABroaderRange) {
	RameligScale scale;

	for (int i = 0; i < 12; i++) {
		scale.setScale({ i });

		// All values below the lower limit should:
		// - Quantize to the same note as when quantizing in the 0-1V range (but the octave will be different)
		// - Quantize to the very first scale note that fits within the limit range, i.e. quantize the same as the lower limit value would.
		for (float j = -10.f; j < -4.5f; j += 0.1f) {
			std::pair<int, int> quantizedReference = scale.quantize(j, 0.f, 0.999999f);
			std::pair<int, int> quantizedWide = scale.quantize(j, -4.5f, 4.5f);
			std::pair<int, int> quantizedLowerLimit = scale.quantize(-4.5f, -4.5f, 4.5f);
			EXPECT_EQ(quantizedWide.second, quantizedReference.second);
			EXPECT_EQ(quantizedWide, quantizedLowerLimit);
		}

		// All values inside the range should:
		// - Quantize to the same note as when quantizing in the 0-1V range (but the octave will be different)
		// - The quantization result should be within 1V of the original value, and be within the specified ranges
		for (float j = -4.5f; j < 4.5f; j += 0.1f) {
			std::pair<int, int> quantizedReference = scale.quantize(j, 0.f, 0.999999f);
			std::pair<int, int> quantizedWide = scale.quantize(j, -4.5f, 4.5f);
			float quantizedVoltage = scale.quantizedToVoltage(quantizedWide);
			EXPECT_EQ(quantizedWide.second, quantizedReference.second);
			if (quantizedVoltage < j) {
				EXPECT_LE(j - quantizedVoltage, 1.f);
			} else {
				EXPECT_LE(quantizedVoltage - j, 1.f);
			}
		}

		// All values above the upper limit should:
		// - Quantize to the same note as when quantizing in the 0-1V range (but the octave will be different)
		// - Quantize to the very last scale note that fits within the limit range, i.e. quantize the same as the upper limit value would.
		for (float j = 4.5f; j < 10.f; j += 0.1f) {
			std::pair<int, int> quantizedReference = scale.quantize(j, 0.f, 0.999999f);
			std::pair<int, int> quantizedWide = scale.quantize(j, -4.5f, 4.5f);
			std::pair<int, int> quantizedUpperLimit = scale.quantize(4.5f, -4.5f, 4.5f);
			EXPECT_EQ(quantizedWide.second, quantizedReference.second);
			EXPECT_EQ(quantizedWide, quantizedUpperLimit);
		}
	}
}

TEST(RameligCoreScale, quantizeShouldQuantizeToTheNearest) {
	RameligScale scale1;
	RameligScale scale2;

	scale1.setScale({ 2, 6, 10 });
	scale2.setScale({ 3, 7, 11 });

	for (int i = -5; i <= 5; i++) {
		// Skew all notes we check with upwards with just-under-1V, so that we're sure quantization should
		// always happen downward (instead of being unsure due to the float rounding errors)
		EXPECT_EQ(scale1.quantize(-0.00001f + i, -10.f, 10.f), std::make_pair(-1 + i, 2));
		EXPECT_EQ(scale2.quantize(-0.00001f + i, -10.f, 10.f), std::make_pair(-1 + i, 2));
		EXPECT_EQ(scale1.quantize(NOTE_C + 0.999999f + i, -10.f, 10.f), std::make_pair(0 + i, 2));
		EXPECT_EQ(scale2.quantize(NOTE_C + 0.999999f + i, -10.f, 10.f), std::make_pair(0 + i, 2));
		EXPECT_EQ(scale1.quantize(NOTE_C_S + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 0));
		EXPECT_EQ(scale2.quantize(NOTE_C_S + 0.999999f + i, -10.f, 10.f), std::make_pair(0 + i, 2));
		EXPECT_EQ(scale1.quantize(NOTE_D + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 0));
		EXPECT_EQ(scale2.quantize(NOTE_D + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 0));
		EXPECT_EQ(scale1.quantize(NOTE_D_S + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 0));
		EXPECT_EQ(scale2.quantize(NOTE_D_S + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 0));
		EXPECT_EQ(scale1.quantize(NOTE_E + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 0));
		EXPECT_EQ(scale2.quantize(NOTE_E + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 0));
		EXPECT_EQ(scale1.quantize(NOTE_F + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 1));
		EXPECT_EQ(scale2.quantize(NOTE_F + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 0));
		EXPECT_EQ(scale1.quantize(NOTE_F_S + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 1));
		EXPECT_EQ(scale2.quantize(NOTE_F_S + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 1));
		EXPECT_EQ(scale1.quantize(NOTE_G + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 1));
		EXPECT_EQ(scale2.quantize(NOTE_G + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 1));
		EXPECT_EQ(scale1.quantize(NOTE_G_S + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 1));
		EXPECT_EQ(scale2.quantize(NOTE_G_S + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 1));
		EXPECT_EQ(scale1.quantize(NOTE_A + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 2));
		EXPECT_EQ(scale2.quantize(NOTE_A + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 1));
		EXPECT_EQ(scale1.quantize(NOTE_A_S + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 2));
		EXPECT_EQ(scale2.quantize(NOTE_A_S + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 2));
		EXPECT_EQ(scale1.quantize(NOTE_B + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 2));
		EXPECT_EQ(scale2.quantize(NOTE_B + 0.999999f + i, -10.f, 10.f), std::make_pair(1 + i, 2));
	}
}

TEST(RameligCoreScale, quantizeShouldQuantizeDownwardIfCloser) {
	RameligScale scale;

	scale.setScale({ 8, 11 });
	EXPECT_EQ(scale.quantize(0.f, -2.f, 2.f), std::make_pair(-1, 1));
}

TEST(RameligCoreScale, quantizeShouldQuantizeUpwardIfCloser) {
	RameligScale scale;

	scale.setScale({ 0, 2 });
	EXPECT_EQ(scale.quantize(0.9f, -2.f, 2.f), std::make_pair(1, 0));
}

TEST(RameligCoreScale, moveShouldMoveUpOneWithinScale) {
	RameligScale scale;

	scale.setScale({ 0, 2, 4, 6 });

	for (int i = -5; i <= 5; i++) {
		EXPECT_EQ(scale.move(std::make_pair(i, 0), 1), std::make_pair(i, 1));
		EXPECT_EQ(scale.move(std::make_pair(i, 1), 1), std::make_pair(i, 2));
		EXPECT_EQ(scale.move(std::make_pair(i, 2), 1), std::make_pair(i, 3));
	}
}

TEST(RameligCoreScale, moveShouldMoveUpTwoWithinScale) {
	RameligScale scale;

	scale.setScale({ 0, 2, 4, 6 });

	for (int i = -5; i <= 5; i++) {
		EXPECT_EQ(scale.move(std::make_pair(i, 0), 2), std::make_pair(i, 2));
		EXPECT_EQ(scale.move(std::make_pair(i, 1), 2), std::make_pair(i, 3));
	}
}

TEST(RameligCoreScale, moveShouldMoveDownOneWithinScale) {
	RameligScale scale;

	scale.setScale({ 0, 2, 4, 6 });

	for (int i = -5; i <= 5; i++) {
		EXPECT_EQ(scale.move(std::make_pair(i, 1), -1), std::make_pair(i, 0));
		EXPECT_EQ(scale.move(std::make_pair(i, 2), -1), std::make_pair(i, 1));
		EXPECT_EQ(scale.move(std::make_pair(i, 3), -1), std::make_pair(i, 2));
	}
}

TEST(RameligCoreScale, moveShouldMoveDownTwoWithinScale) {
	RameligScale scale;

	scale.setScale({ 0, 2, 4, 6 });

	for (int i = -5; i <= 5; i++) {
		EXPECT_EQ(scale.move(std::make_pair(i, 2), -2), std::make_pair(i, 0));
		EXPECT_EQ(scale.move(std::make_pair(i, 3), -2), std::make_pair(i, 1));
	}
}

TEST(RameligCoreScale, moveShouldMoveUpOneOverScale) {
	RameligScale scale;

	scale.setScale({ 0, 2, 4, 6 });

	for (int i = -5; i <= 5; i++) {
		EXPECT_EQ(scale.move(std::make_pair(i, 3), 1), std::make_pair(i + 1, 0));
	}
}

TEST(RameligCoreScale, moveShouldMoveUpTwoOverScale) {
	RameligScale scale;

	scale.setScale({ 0, 2, 4, 6 });

	for (int i = -5; i <= 5; i++) {
		EXPECT_EQ(scale.move(std::make_pair(i, 3), 2), std::make_pair(i + 1, 1));
	}
}

TEST(RameligCoreScale, moveShouldMoveUpTwoOverScales) {
	RameligScale scale;

	scale.setScale({ 0 });

	for (int i = -5; i <= 5; i++) {
		EXPECT_EQ(scale.move(std::make_pair(i, 0), 2), std::make_pair(i + 2, 0)) << std::to_string(i);
	}
}

TEST(RameligCoreScale, moveShouldMoveDownOneOverScale) {
	RameligScale scale;

	scale.setScale({ 0, 2, 4, 6 });

	for (int i = -5; i <= 5; i++) {
		EXPECT_EQ(scale.move(std::make_pair(i, 0), -1), std::make_pair(i - 1, 3));
	}
}

TEST(RameligCoreScale, moveShouldMoveDownTwoOverScale) {
	RameligScale scale;

	scale.setScale({ 0, 2, 4, 6 });

	for (int i = -5; i <= 5; i++) {
		EXPECT_EQ(scale.move(std::make_pair(i, 0), -2), std::make_pair(i - 1, 2));
	}
}

TEST(RameligCoreScale, moveShouldMoveDownTwoOverScales) {
	RameligScale scale;

	scale.setScale({ 0 });

	for (int i = -5; i <= 5; i++) {
		EXPECT_EQ(scale.move(std::make_pair(i, 0), -2), std::make_pair(i - 2, 0)) << std::to_string(i);
	}
}
