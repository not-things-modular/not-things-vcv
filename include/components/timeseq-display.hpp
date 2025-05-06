#pragma once
#include <rack.hpp>
using namespace rack;

const int TIMESEQ_DISPLAY_WINDOW_SIZE = 256;

namespace timeseq {
	struct TimeSeqCore;
}


struct TimeSeqVoltagePoints {
	TimeSeqVoltagePoints(int id): id(id), age(0), voltage(0.f) {}
	int id;
	int age;
	float voltage;
};

struct TimeSeqDisplay : widget::Widget {
	void drawLayer(const DrawArgs& args, int layer) override;

	// void onResize(const ResizeEvent& e) override;

	void processChangedVoltages(std::vector<int>& changedVoltages, std::array<std::array<float, 16>, 8>& outputVoltages);
	void ageVoltages();
	void reset();

	void setError(bool error);
	void setTimeSeqCore(timeseq::TimeSeqCore* timeSeqCore);

	private:
		timeseq::TimeSeqCore* m_timeSeqCore = nullptr;
		std::vector<TimeSeqVoltagePoints> m_voltagePoints;
		bool m_error = false;

		float m_arcDelta = 0.f;
};
