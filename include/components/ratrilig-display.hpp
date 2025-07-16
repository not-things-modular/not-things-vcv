#pragma once
#include <rack.hpp>
using namespace rack;

#include <array>

struct RatriligData;

struct RatriligDisplay : widget::Widget {
	RatriligDisplay();
	~RatriligDisplay();

	void drawLayer(const DrawArgs& args, int layer) override;

	void onResize(const ResizeEvent& e) override;

	void setData(RatriligData* data);

	private:
		RatriligData* m_data;
		RatriligData* m_drawData;
};
