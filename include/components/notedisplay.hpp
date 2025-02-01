#pragma once
#include <rack.hpp>
using namespace rack;


struct NoteDisplay : widget::Widget {
	int getScale();
	void setScale(int scale);
	int getNote();
	void setNote(int note);
	
	void drawLayer(const DrawArgs& args, int layer) override;

	private:
		int m_scale = 5;
		int m_note = 0;
};
