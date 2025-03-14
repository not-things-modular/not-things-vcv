#pragma once
#include <rack.hpp>
using namespace rack;


const int TIMESEQ_DISPLAY_WINDOW_SIZE = 256;


struct TimeSeqVoltagePoints {
	TimeSeqVoltagePoints(int id): id(id), age(0), voltage(0.f) {}
	int id;
	int age;
	float voltage;
};

struct TimeSeqDisplay : widget::Widget {
	void drawLayer(const DrawArgs& args, int layer) override;

	std::string m_time = "04:20";
	std::vector<TimeSeqVoltagePoints> m_voltagePoints;
};
