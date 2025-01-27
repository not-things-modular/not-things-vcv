#include "components/dualbefacoswitch.hpp"

DualBefacoSwitch::DualBefacoSwitch() {
    addFrame(Svg::load(asset::system("res/ComponentLibrary/BefacoSwitch_0.svg")));
    addFrame(Svg::load(asset::system("res/ComponentLibrary/BefacoSwitch_2.svg")));
}
