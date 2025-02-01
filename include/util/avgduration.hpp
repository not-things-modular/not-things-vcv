#pragma once

#include <list>
#include <chrono>

template <int buffer_size, typename ratio = std::micro> 
struct AvgDuration {
	std::chrono::high_resolution_clock::time_point startTime;
	std::array<double, buffer_size> durations = {};
	int currentIndex = 0;

	void start() {
		startTime = std::chrono::high_resolution_clock::now();
	}

	void stop() {
		std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, ratio> duration = endTime - startTime;
		durations[currentIndex] = duration.count();
		currentIndex = (currentIndex + 1) % buffer_size;
	}

	double getAvgDuration() {
		return std::accumulate(durations.begin(), durations.end(), 0.) / buffer_size;
	}
};
