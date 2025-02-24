#pragma once

#include <string>
#include <vector>
#include <unordered_map>


namespace timeseq {

struct ScriptTimeline;
struct ScriptLane;
struct ScriptSegment;

struct ValueProcessor {
	virtual float process();
};

struct ActionProcessor {
	virtual void process();
};

struct ActionSetValueProcessor : ActionProcessor {
	void process() override;

	private:
		ValueProcessor m_valueProcessor;
		int m_outputPort;
		int m_outputChannel;

};

struct ActionSetPolyphonyProcessor : ActionProcessor {
	void process() override;

	private:
		int m_outputPort;
		int m_channelCount;

};

struct ActionTriggerProcessor : ActionProcessor {
	void process() override;

	private:
		std::string trigger;

};

struct ActionGlideProcessor {
	void start();
	void process(long glidePosition, long glideLength);

	private:
		ValueProcessor m_startValueProcessor;
		ValueProcessor m_endValueProcessor;
		float m_startValue;
		float m_endValue;
};

struct DurationProcessor {
	enum State { STATE_IDLE, STATE_START, STATE_PROGRESS, STATE_STOP };

	State process();

	private:
		long m_duration;
		long m_position;
};

struct SegmentProcessor {
	void process();

	private:
		ScriptSegment* m_scriptSegment;
		DurationProcessor m_durationProcessor;

		std::vector<ActionProcessor> m_startActions;
		std::vector<ActionProcessor> m_endActions;
		std::vector<ActionGlideProcessor> m_glideActions;
};

struct LaneProcessor {
	void process();

	private:
		ScriptLane* m_scriptLane;
		std::vector<SegmentProcessor> m_segments;

		bool m_active;
		int m_repeatCount;
};
	
struct TimelineProcessor {
	void process();

	private:
		ScriptTimeline* m_scriptTimeline;
		std::vector<LaneProcessor> m_laneProcessors;

		std::unordered_map<std::string, std::vector<LaneProcessor*>> m_startTriggers;
		std::unordered_map<std::string, std::vector<LaneProcessor*>> m_stopTriggers;
};

}
