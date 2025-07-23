#include "components/ratrilig-bias.hpp"
#include "core/timeseq-core.hpp"
#include <algorithm>
#include <array>
#include <cmath>

extern Plugin* pluginInstance;

RatriligBias::RatriligBias() {

}

void RatriligBias::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1) {
		return;
	}

	int index = m_bias == 1.f ? m_count - 1 : m_count * m_bias;
	float width = (box.getWidth() - 4) / m_count;

	nvgSave(args.vg);
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
	nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());

	nvgBeginPath(args.vg);
	nvgRect(args.vg, 2.f + width * index, 2.f, width, box.getHeight() - 4.f);
	nvgFillColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xFF));
	nvgFill(args.vg);

	nvgResetScissor(args.vg);
	nvgRestore(args.vg);
}

void RatriligBias::setBias(float bias, int count) {
	m_bias = bias;
	m_count = count;
}
