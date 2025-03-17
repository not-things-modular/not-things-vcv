#include "components/timeseq-display.hpp"
#include <algorithm>
#include <array>

#define CHANNEL_FROM_CHANNEL_PORT_IDENTIFIER(identifier) (identifier >> 5)
#define PORT_FROM_CHANNEL_PORT_IDENTIFIER(identifier) (identifier & 0xF)

void TimeSeqDisplay::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1) {
		return;
	}

	nvgSave(args.vg);

	if (m_voltagePoints.size() > 0) {
		nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);

		nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());
		nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);

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

		nvgResetScissor(args.vg);
	}

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
			if (m_voltagePoints.size() < 16) {
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

void TimeSeqDisplay::reset() {
	m_voltagePoints.clear();
}