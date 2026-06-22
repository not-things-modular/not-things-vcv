#include "util/scale.hpp"
#include <cmath>

float powScale(float value, float pow) {
	return value >= 0.f ? std::pow(value, pow) : -std::pow(-value, pow);
}

float reversePowScale(float value, float pow) {
	return 1.f - powScale(value, pow);
}