#pragma once
#include <rack.hpp>
using namespace rack;


struct NoteDisplay : widget::Widget {
    int scale = 5;
    int note = 0;

	void drawLayer(const DrawArgs& args, int layer) override;
};
