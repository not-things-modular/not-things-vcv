#include "components/ratrilig-probability.hpp"
#include "core/timeseq-core.hpp"
#include "util/scale.hpp"
#include <algorithm>
#include <array>

#define MOVE_LARGE_THRESHOLD (70.f / 180.0f * NVG_PI)
#define MOVE_LARGE (20.f / 180.0f * NVG_PI)
#define MOVE_SMALL (10.f / 180.0f * NVG_PI)
#define CENTER_TARGET_OFFSET (112.5f / 180.0f * NVG_PI)

void RatriligProbability::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1) {
		return;
	}

	nvgSave(args.vg);
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
	nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());
	nvgFillColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0x88));

	nvgBeginPath(args.vg);
	nvgCircle(args.vg, m_x, m_y, m_innerCircleRadius);
	if (m_enabled) {
		nvgFill(args.vg);
	} else {
		nvgStrokeWidth(args.vg, 1.f);
		nvgStrokeColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0x88));
		nvgStroke(args.vg);
	}

	float target = m_centerTarget; // Take a copy of the target, in case it's adjusted while we're updating m_current
	if (m_centerCurrent == 0) {
		m_centerCurrent = target;
	} else if (target == 0) {
		m_centerCurrent = 0;
	} else if (m_centerCurrent < target) {
		float diff = target - m_centerCurrent;
		m_centerCurrent += (diff > MOVE_LARGE_THRESHOLD ? MOVE_LARGE : MOVE_SMALL);
		if (m_centerCurrent > target) {
			m_centerCurrent = target;
		}
	} else if (m_centerCurrent > target) {
		float diff = m_centerCurrent - target;
		m_centerCurrent -= (diff > MOVE_LARGE_THRESHOLD ? MOVE_LARGE : MOVE_SMALL);
		if (m_centerCurrent < target) {
			m_centerCurrent = target;
		}
	}

	nvgStrokeWidth(args.vg, 3.f);
	nvgBeginPath(args.vg);
	nvgStrokeColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0x55));
	nvgArc(args.vg, m_x, m_y, m_outerCircleRadius, nvgDegToRad(270 - 157.5f), nvgDegToRad(270 + 157.5f), NVG_CW);
	nvgStroke(args.vg);
	nvgBeginPath(args.vg);
	nvgStrokeColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xAA));
	nvgArc(args.vg, m_x, m_y, m_outerCircleRadius, m_outerFrom, m_outerTo, NVG_CW);
	nvgStroke(args.vg);

	nvgStrokeWidth(args.vg, 5.f);
	nvgBeginPath(args.vg);
	nvgStrokeColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xFF));
	nvgArc(args.vg, m_x, m_y, m_centerCircleRadius, m_centerFrom, CENTER_TARGET_OFFSET + m_centerCurrent, CENTER_TARGET_OFFSET + m_centerCurrent > m_centerFrom ? NVG_CW : NVG_CCW);
	nvgStroke(args.vg);

	nvgResetScissor(args.vg);
	nvgRestore(args.vg);
}

void RatriligProbability::onResize(const ResizeEvent& e) {
	calculatePositions();
}

void RatriligProbability::setBidirectional(bool bidirectional) {
	m_bidirectional = bidirectional;
}

void RatriligProbability::setEnabled(bool enabled) {
	m_enabled = enabled;
}

void RatriligProbability::setDensity(float density) {
	m_density = density;
	calculateTargets();
}

void RatriligProbability::setDensityFactor(float factor) {
	m_factor = factor;
	calculateTargets();
}

void RatriligProbability::setBias(float bias) {
	m_bias = bias;
	calculateTargets();
}

void RatriligProbability::calculatePositions() {
	m_x = box.getWidth() / 2;
	m_y = box.getHeight() / 2 + .5f;

	m_innerCircleRadius = (box.getHeight() - 2) / 8;
	m_outerCircleRadius = box.getWidth() / 2 - 3;
	m_centerCircleRadius = (m_outerCircleRadius - m_innerCircleRadius) / 2 + m_innerCircleRadius - 1.f;
}

void RatriligProbability::calculateTargets() {
	if (m_bidirectional) {
		float range = powScale(m_factor, 0.6f) * 157.5f;
		m_outerFrom = nvgDegToRad(std::max(270.f - range + m_bias, 270.f - 157.5f));
		m_outerTo = nvgDegToRad(std::min(270.f + range + m_bias, 270.f + 157.5f));

		range = powScale(m_density, 0.6f) * 157.5f;
		m_centerFrom = nvgDegToRad(std::max(std::min(270.f + m_bias, 270.f + 157.5f), 270.f - 157.5f));
		m_centerTarget = nvgDegToRad(std::max(std::min(range + m_bias + 270.f, 270.f + 157.5f), 270.f - 157.5f) - 112.f);
	} else {
		float range = m_factor * 315.f;
		m_outerFrom = nvgDegToRad(270.f - 157.5f);
		m_outerTo = nvgDegToRad(std::min(270.f - 157.5f + range, 270 + 157.5f));

		range = m_density * 315.f;
		m_centerFrom = nvgDegToRad(270 - 157.5f);
		m_centerTarget = nvgDegToRad(std::min(range, 315.f));
	}
}
