#include "components/ramelig-distribution.hpp"
#include "core/ramelig-core.hpp"

const int rameligDistributionActionDisplayOrder[] = {
	RameligActions::DOWN_TWO,
	RameligActions::DOWN_ONE,
	RameligActions::UP_ONE,
	RameligActions::UP_TWO,
	RameligActions::STAY,
	RameligActions::RANDOM_JUMP,
	RameligActions::RANDOM_SHIFT
};

NVGcolor rameligDistributionActionFillColors[] = {
	nvgRGBA(0x45, 0xDB, 0xCC, 0xFF), // RANDOM_JUMP = orange
	nvgRGBA(0xCC, 0xC0, 0x45, 0xFF), // RANDOM_SHIFT = yellow
	nvgRGBA(0xBB, 0x45, 0x45, 0xAA), // UP_TWO = dark red
	nvgRGBA(0xBB, 0x45, 0x45, 0xFF), // UP_ONE = red
	nvgRGBA(0x45, 0x45, 0xBB, 0xFF), // DOWN_ONE = blue
	nvgRGBA(0x45, 0x45, 0xBB, 0xAA), // DOWN_TWO = dark blue
	nvgRGBA(0x45, 0x99, 0x45, 0xFF)  // STAY = green
};

RameligDistribution::RameligDistribution() {
	m_lastAction = 6;
}

void RameligDistribution::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1) {
		return;
	}

	nvgSave(args.vg);
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
	nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());

	float factor = (box.getWidth() - 4) / m_distribution[RameligActions::STAY];
	float y = 2.f;
	float height = (box.getHeight() - 6) / 2;

	float previous = 0.f;
	for (int i = 0; i < 7; i++) {
		float current = m_distribution[i] * factor;

		if (current > previous) {
			nvgBeginPath(args.vg);
			nvgRect(args.vg, previous + 2.f, y, current - previous, height);
			nvgFillColor(args.vg, rameligDistributionActionFillColors[rameligDistributionActionDisplayOrder[i]]);
			nvgFill(args.vg);
			previous = current;
		}
	}

	y = 4.5f + height;
	nvgBeginPath(args.vg);
	nvgRect(args.vg, 2.5f, y, box.getWidth() - 5.f, height - 1.f);
	nvgStrokeColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xFF));
	nvgStroke(args.vg);

	if (m_lastUpper > m_lastLower) {
		float x = (box.getWidth() - 12) * (m_lastValue - m_lastLower) / (m_lastUpper - m_lastLower);
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 3.f + x, y + .5f, 6.f, height - 2.f);
		nvgFillColor(args.vg, rameligDistributionActionFillColors[m_lastAction]);
		nvgFill(args.vg);
	}

	nvgResetScissor(args.vg);
	nvgRestore(args.vg);
}

void RameligDistribution::setDistribution(std::array<float, 7>& distribution) {
	// The order that's optimal for processing is different from the order in which we want to display them,
	// so update that order now.
	float offset = 0.f;
	for (int i = 0; i < 7; i++) {
		if (rameligDistributionActionDisplayOrder[i] == 0) {
			offset += distribution[rameligDistributionActionDisplayOrder[i]];
		} else {
			offset += distribution[rameligDistributionActionDisplayOrder[i]] - distribution[rameligDistributionActionDisplayOrder[i] - 1];
		}
		m_distribution[i] = offset;
	}
}

void RameligDistribution::setLastAction(int lastAction) {
	m_lastAction = lastAction;
}

void RameligDistribution::setLastValue(float value, float lower, float upper) {
	m_lastValue = value;
	m_lastLower = lower;
	m_lastUpper = upper;
}
