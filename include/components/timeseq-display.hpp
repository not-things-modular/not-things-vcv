#pragma once
#include <rack.hpp>
using namespace rack;


#define TIMESEQ_SAMPLE_COUNT 236

struct TimeSeqVoltagePoints {
	TimeSeqVoltagePoints(int id): id(id), age(0) {}
	int id;
	int age;
	float points[TIMESEQ_SAMPLE_COUNT];
};

struct BufferedDisplay;

struct TimeSeqDisplay : widget::Widget {
	TimeSeqDisplay();
	~TimeSeqDisplay();
	
	void drawLayer(const DrawArgs& args, int layer) override;

	std::vector<TimeSeqVoltagePoints> m_voltagePoints;
	int m_currentVoltagePointIndex = 0;

	private:
		BufferedDisplay* m_bufferedDisplay;
};
