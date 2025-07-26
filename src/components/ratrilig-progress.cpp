#include "components/ratrilig-progress.hpp"
#include "core/timeseq-core.hpp"
#include "util/scale.hpp"
#include <algorithm>
#include <array>
#include <cmath>

extern Plugin* pluginInstance;

RatriligProgress::RatriligProgress() {

}

void RatriligProgress::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1) {
		return;
	}

	nvgSave(args.vg);
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
	nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());

	int phraseCount = m_phraseCount;
	int groupCount = m_groupCount;
	int clusterCount = m_clusterCount;
	int count = 0;

	int separator;
	if ((groupCount * clusterCount) <= 70.f) {
		nvgStrokeWidth(args.vg, .5f);
		separator = 2;
	} else {
		nvgStrokeWidth(args.vg, .25f);
		separator = 1;
	}

	float height = (box.getHeight() - 4) / phraseCount;
	float width = (box.getWidth() - 4 - (groupCount - 1) * separator) / (groupCount * clusterCount);

	float x = 2.f;
	float y = 2.f;
	for (int phrase = 0; phrase < phraseCount; phrase++) {
		for (int group = 0; group < groupCount; group++) {
			for (int cluster = 0; cluster < clusterCount; cluster++) {
				nvgBeginPath(args.vg);
				nvgRect(args.vg, x, y, width, height);
				if (count <= m_position) {
					nvgStrokeColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xFF));
					nvgFillColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, m_values[count]));
					nvgFill(args.vg);
				} else {
					nvgStrokeColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0x55));
					// If this is the first item in the cluster, we'll have to draw the left line again since there is no overlap
					if (cluster == 0) {
						nvgMoveTo(args.vg, x, y);
						nvgLineTo(args.vg, x, y + height);
					}
					// If this is the last item in the cluster, we'll have to draw the right line again since there is no overlap
					if (cluster == clusterCount - 1) {
						nvgMoveTo(args.vg, x + width, y);
						nvgLineTo(args.vg, x + width, y + height);
					}
				}
				nvgStroke(args.vg);
				count++;
				x += width;
			}

			x += separator;
		}
		x = 2.f;
		y += height;
	}

	nvgResetScissor(args.vg);
	nvgRestore(args.vg);
}

void RatriligProgress::setClusterCount(int clusterCount) {
	m_clusterCount = clusterCount;
}

void RatriligProgress::setGroupCount(int groupCount) {
	m_groupCount = groupCount;
}

void RatriligProgress::setPhraseCount(int phraseCount) {
	m_phraseCount = phraseCount;
}

void RatriligProgress::setPositionValue(int position, float value) {
	m_position = position;
	m_values[position] = powScale(value, .75f) * 0xFF;
}
