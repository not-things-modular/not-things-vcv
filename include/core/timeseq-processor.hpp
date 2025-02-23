#pragma once

#include <vector>


namespace timeseq {

struct ScriptTimeline;

struct LaneProcessor {

};
	
struct TimelineProcessor {
	void process();

	private:
		ScriptTimeline *m_scriptTimeline;
		std::vector<LaneProcessor> m_laneProcessors;
};

}
