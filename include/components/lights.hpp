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
