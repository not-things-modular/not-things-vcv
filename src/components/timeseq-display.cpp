#include "components/timeseq-display.hpp"
#include "core/timeseq-core.hpp"
#include <algorithm>
#include <array>
#include <cmath>

#define CHANNEL_FROM_CHANNEL_PORT_IDENTIFIER(identifier) (identifier >> 5)
#define PORT_FROM_CHANNEL_PORT_IDENTIFIER(identifier) (identifier & 0xF)

float START_ARC = nvgDegToRad(225);
float END_ARC = nvgDegToRad(540);

extern Plugin* pluginInstance;

void TimeSeqDisplay::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1) {
		return;
	}

	nvgSave(args.vg);
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
	nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());

	if (m_timeSeqCore) {
		uint32_t tripSampleRate = m_timeSeqCore->getCurrentSampleRate() * 3;
		uint32_t sampleRemainder = m_timeSeqCore->getElapsedSamples() % tripSampleRate * 4;
		float pos = (float) sampleRemainder / tripSampleRate;
		float index;
		float fraction = std::modf(pos, &index);

		nvgStrokeColor(args.vg, nvgRGBA(0x50, 0xAA, 0x50, 0x2A + 0x70));
		nvgFillColor(args.vg, nvgRGBA(0x00, 0xFF, 0x00, 0x40));

		if ((m_timeSeqCore->getStatus() == timeseq::TimeSeqCore::Status::IDLE) || (m_timeSeqCore->getStatus() == timeseq::TimeSeqCore::Status::PAUSED)) {
			std::shared_ptr<window::Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-Bold.ttf"));
			if (font && font->handle >= 0) {
				nvgBeginPath(args.vg);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0.0);
				nvgFontSize(args.vg, 8.f);
				nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
				nvgFillColor(args.vg, nvgRGBA(0xFF, 0x50, 0x50, 0xAA));
				nvgText(args.vg, (box.getWidth() / 2), 4.5f, "PAUSED", NULL);
				nvgFill(args.vg);
			}
		} else if (m_timeSeqCore->getStatus() == timeseq::TimeSeqCore::Status::RUNNING) {
			nvgStrokeWidth(args.vg, 1.f);
			float arcDelta = box.getWidth() / 4;
			float arcOffset = 0.f + arcDelta / 2;

			float offset1, offset2, offset3;
			if (index == 0.f) {
				offset1 = arcOffset;
				offset2 = arcOffset + arcDelta * 3;
				offset3 = arcOffset + arcDelta * 2;
			} else if (index == 1.f) {
				offset1 = arcOffset + arcDelta;
				offset2 = arcOffset;
				offset3 = arcOffset + arcDelta * 3;
			} else if (index == 2.f) {
				offset1 = arcOffset + arcDelta * 2;
				offset2 = arcOffset + arcDelta;
				offset3 = arcOffset;
			} else {
				offset1 = arcOffset + arcDelta * 3;
				offset2 = arcOffset + arcDelta * 2;
				offset3 = arcOffset + arcDelta;
			}

			nvgStrokeColor(args.vg, nvgRGBA(0xFF, 0x50, 0x50, (0x55 + 0x55)));
			nvgBeginPath(args.vg);
			// nvgCircle(args.vg, arcOffset + arcDelta * 0, 4.5, 3.5f);
			// nvgCircle(args.vg, arcOffset + arcDelta * 1, 4.5, 3.5f);
			// nvgCircle(args.vg, arcOffset + arcDelta * 2, 4.5, 3.5f);
			// nvgCircle(args.vg, arcOffset + arcDelta * 3, 4.5, 3.5f);
			nvgArc(args.vg, offset1, 4.5f, 3.5f, START_ARC, START_ARC + (END_ARC - START_ARC) * fraction, NVG_CW);
			nvgStroke(args.vg);
			nvgBeginPath(args.vg);
			nvgArc(args.vg, offset2, 4.5f, 3.5f, START_ARC + (END_ARC - START_ARC) * fraction, END_ARC, NVG_CW);
			nvgMoveTo(args.vg, offset2 - 3.5, 4.5f);
			nvgLineTo(args.vg, offset2 - 3.5 + (arcDelta) * fraction, 4.5f);
			nvgStroke(args.vg);
			nvgBeginPath(args.vg);
			nvgStrokeColor(args.vg, nvgRGBA(0xFF, 0x50, 0x50, (0x55 + 0x55) * (1 - fraction)));
			nvgMoveTo(args.vg, offset3 - 3.5, 4.5f);
			nvgLineTo(args.vg, offset3 - 3.5 + arcDelta, 4.5f);
			nvgStroke(args.vg);
		} else {
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0x50, 0x50, 0xDD));
			nvgBeginPath(args.vg);
			nvgStrokeWidth(args.vg, 1.f);
			nvgRoundedRect(args.vg, 1.f, 1.f, 37.f, 9.f, 2.f);
			nvgFill(args.vg);

			std::shared_ptr<window::Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-Bold.ttf"));
			if (font && font->handle >= 0) {
				nvgBeginPath(args.vg);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0.0);
				nvgFontSize(args.vg, 8.f);
				nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

				nvgFillColor(args.vg, nvgRGBA(0xFF, 0x50, 0x50, 0x55 + 0x55));
				nvgFillColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, 0xAA));
				nvgText(args.vg, box.getWidth() / 2, 5.5f, m_error ? "ERROR" : "NO:SCR", NULL);
				nvgFill(args.vg);
			}
		}
	}

	/*if (m_message.length() > 0) {
		nvgRotate(args.vg, nvgDegToRad(-90.f));
		nvgScissor(args.vg, 0, 0, box.getHeight(), box.getWidth());

		std::shared_ptr<window::Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-Bold.ttf"));
		if (font && font->handle >= 0) {
			nvgBeginPath(args.vg);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0.0);
			nvgFontSize(args.vg, 10.f);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

			nvgFillColor(args.vg, nvgRGBA(0xFF, 0x50, 0x50, 0xAA));
			nvgText(args.vg, -getBox().getHeight() / 2, getBox().getWidth() / 2, m_message.c_str(), NULL);
			nvgFill(args.vg);
		}

		nvgResetScissor(args.vg);
		nvgResetTransform(args.vg);
	} else */if (m_voltagePoints.size() > 0) {
		int offset = 0;
		for (std::vector<TimeSeqVoltagePoints>::iterator it = m_voltagePoints.begin(); it != m_voltagePoints.end(); it++) {
			float v = std::min(std::max(it->voltage, -10.f), 10.f);
			float factor = it->age < 31 ? (30.f - it->age) / 30.f : 0.f;

			nvgStrokeColor(args.vg, nvgRGBA(0xFF, 0x50, 0x50, 0x55 + 0x55 * factor));
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0, 0, 0x20 + 0x20 * factor));

			nvgBeginPath(args.vg);
			nvgStrokeWidth(args.vg, 1.f);
			nvgRoundedRect(args.vg, 1.f, 166.f - offset, 37.f, 9.f, 2.f);
			nvgStroke(args.vg);

			nvgBeginPath(args.vg);
			nvgStrokeWidth(args.vg, 0.f);
			nvgRoundedRect(args.vg, 19.5f, 167.f - offset, 17.5f * (v / 10.f), 7.f, 1.f);
			nvgFill(args.vg);
			nvgStroke(args.vg);

			offset += 11;
		}
	}

	nvgResetScissor(args.vg);
	nvgRestore(args.vg);
}

void TimeSeqDisplay::processChangedVoltages(std::vector<int>& changedVoltages, std::array<std::array<float, 16>, 8>& outputVoltages) {
	// Remove voltage points that haven't changed recently, and update & age those that are recent enough
	for (int i = m_voltagePoints.size() - 1; i >= 0; i--) {
		if (m_voltagePoints[i].age >= TIMESEQ_DISPLAY_WINDOW_SIZE * 2) {
			m_voltagePoints.erase(m_voltagePoints.begin() + i);
		} else {
			m_voltagePoints[i].voltage = outputVoltages[CHANNEL_FROM_CHANNEL_PORT_IDENTIFIER(m_voltagePoints[i].id)][PORT_FROM_CHANNEL_PORT_IDENTIFIER(m_voltagePoints[i].id)];
			m_voltagePoints[i].age++;
		}
	}

	// Update/add the voltage points for the recently changed ports
	for (std::vector<int>::iterator it = changedVoltages.begin(); it != changedVoltages.end(); it++) {
		bool found = false;
		// See if the port&channel combination is already in the current list of voltage points
		for (std::vector<TimeSeqVoltagePoints>::iterator vpIt = m_voltagePoints.begin(); vpIt != m_voltagePoints.end(); vpIt++) {
			if (vpIt->id == *it) {
				// The voltage point is already in there, so it was already captured. Just reset its age.
				found = true;
				vpIt->age = 0;
				break;
			}
		}
		// It's a new voltage point
		if (!found)
		{
			if (m_voltagePoints.size() < 15) {
				// We haven't reached the limit of trackable voltages yet, so just add a new one to the list.
				m_voltagePoints.emplace_back(*it);
				TimeSeqVoltagePoints& voltagePoints = m_voltagePoints.back();
				voltagePoints.voltage = outputVoltages[CHANNEL_FROM_CHANNEL_PORT_IDENTIFIER(voltagePoints.id)][PORT_FROM_CHANNEL_PORT_IDENTIFIER(voltagePoints.id)];
			} else {
				// We have reached the limit of trackable voltages. See if there is one that hasn't updated in the last cycle (start from the end, i.e. the most recent changing one)
				for (std::vector<TimeSeqVoltagePoints>::reverse_iterator vpIt = m_voltagePoints.rbegin(); vpIt != m_voltagePoints.rend(); vpIt++) {
					if (vpIt->age > TIMESEQ_DISPLAY_WINDOW_SIZE) {
						// Replace this tracked output with the newly changed one
						vpIt->id = *it;
						vpIt->age = 0;
						vpIt->voltage = outputVoltages[CHANNEL_FROM_CHANNEL_PORT_IDENTIFIER(vpIt->id)][PORT_FROM_CHANNEL_PORT_IDENTIFIER(vpIt->id)];
						break;
					}
				}
			}
		}
	}
}

void TimeSeqDisplay::ageVoltages() {
	// Remove voltage points that haven't changed recently, age those that are recent enough
	for (int i = m_voltagePoints.size() - 1; i >= 0; i--) {
		if (m_voltagePoints[i].age >= TIMESEQ_DISPLAY_WINDOW_SIZE * 2) {
			m_voltagePoints.erase(m_voltagePoints.begin() + i);
		} else {
			m_voltagePoints[i].age++;
		}
	}
}

void TimeSeqDisplay::reset() {
	m_voltagePoints.clear();
}

void TimeSeqDisplay::setError(bool error) {
	m_error = error;
}

void TimeSeqDisplay::setTimeSeqCore(timeseq::TimeSeqCore* timeSeqCore) {
	m_timeSeqCore = timeSeqCore;
}