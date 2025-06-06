#pragma once
#include <rack.hpp>
using namespace rack;

template <typename TBase = GrayModuleLightWidget>
struct TBlueGreenLight : TBase {
	TBlueGreenLight() {
		this->addBaseColor(SCHEME_BLUE);
		this->addBaseColor(SCHEME_GREEN);
	}
};
using BlueGreenLight = TBlueGreenLight<>;

template <typename TBase = GrayModuleLightWidget>
struct TRedOrangeGreenLight : TBase {
	TRedOrangeGreenLight() {
		this->addBaseColor(SCHEME_RED);
		this->addBaseColor(SCHEME_ORANGE);
		this->addBaseColor(SCHEME_GREEN);
	}
};
using RedOrangeGreenLight = TRedOrangeGreenLight<>;

template <typename TBase = GrayModuleLightWidget>
struct DimmedLight : TBase {
	void step() override {
		std::vector<float> brightnesses(TBase::baseColors.size());

		if (TBase::module) {
			return TBase::step();
		}
		else {
			// Turn all lights off
			for (size_t i = 0; i < TBase::baseColors.size(); i++) {
				brightnesses[i] = 0.f;
			}
			TBase::setBrightnesses(brightnesses);
			MultiLightWidget::step();
		}
	}
};
