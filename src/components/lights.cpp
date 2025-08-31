#include "components/lights.hpp"

void reduceLightWithThreshold(Light& light, float deltaTime, float lambda) {
	if (light.getBrightness() > 0.f) {
		if (light.getBrightness() > 0.0001f) {
			light.setBrightnessSmooth(0.f, deltaTime, lambda);
		} else {
			light.setBrightness(0.f);
		}
	}
}
