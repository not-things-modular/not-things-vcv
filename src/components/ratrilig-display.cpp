#include "components/ratrilig-display.hpp"
#include "core/ratrilig-core.hpp"

// NVGcolor LIGHT_RED = nvgRGBA(0xBB, 0x45, 0x45, 0xFF);
// NVGcolor DARK_RED = nvgRGBA(0x20, 0x20, 0x20, 0xFF);


extern Plugin* pluginInstance;

RatriligDisplay::RatriligDisplay() {
	m_data = new RatriligData();
	m_drawData = new RatriligData();
}

RatriligDisplay::~RatriligDisplay() {
	delete m_data;
	delete m_drawData;
}

void RatriligDisplay::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1) {
		return;
	}

	float width = box.getWidth();
	float height = box.getHeight();

	nvgSave(args.vg);
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
	nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());

	nvgStrokeWidth(args.vg, 0.1f);
	nvgStrokeColor(args.vg, nvgRGBA(0xFF, 0x0, 0x0, 0x80));
	nvgFillColor(args.vg, nvgRGBA(0x0, 0xFF, 0x0, 0x80));

	*m_drawData = *m_data;
	int totalCount = m_drawData->phraseSize * m_drawData->groupSize * m_drawData->clusterSize;
	int count = 0;
	for (int pi = 0; pi < m_drawData->phraseSize; pi++) {
		for (int gi = 0; gi < m_drawData->groupSize; gi++) {
			for (int ci = 0; ci < m_drawData->clusterSize; ci++) {
				nvgBeginPath(args.vg);
				nvgFillColor(args.vg, nvgRGBA(0x0, 0xFF, 0x0, 0x80));
				nvgRect(args.vg, width / totalCount * count, 0, width / totalCount * (count + 1), height * m_drawData->density / 100);
				nvgStroke(args.vg);
				count++;
			}
		}
	}

	nvgResetScissor(args.vg);
	nvgRestore(args.vg);

	// float animationPos;
	// timeseq::TimeSeqCore::Status status;
	// std::vector<TimeSeqVoltagePoints>* voltagePoints;
	// if (m_timeSeqCore) {
	// 	// If there is a TimeSeq core assigned, there is actual data to draw
	// 	status = m_timeSeqCore->getStatus();
	// 	uint32_t tripSampleRate = m_timeSeqCore->getCurrentSampleRate() * 3; // Let each animation loop take 3 seconds
	// 	uint32_t sampleRemainder = m_timeSeqCore->getElapsedSamples() % tripSampleRate * 4; // Multiply by 4 so we can divide it over the four circles
	// 	animationPos = (float) sampleRemainder / tripSampleRate; // How far along we are in the animation, between 0.f and 4.f

	// 	voltagePoints = &m_voltagePoints;
	// } else {
	// 	// There is no TimeSeq core, so we're in "screenshot" mode.
	// 	status = timeseq::TimeSeqCore::Status::RUNNING;
	// 	animationPos = 3.33;
	// 	voltagePoints = &m_dummyVoltagePoints;
	// }

	// nvgSave(args.vg);
	// nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
	// nvgScissor(args.vg, 0, 0, box.getWidth(), box.getHeight());

	// if ((status == timeseq::TimeSeqCore::Status::RUNNING) && (!m_assert)) {
	// 	float index;
	// 	float fraction = std::modf(animationPos, &index); // How far along we are in the animation of the currently active circle
	// 	float *offsets = m_animCoords.m_offsetCircles[(int) index];

	// 	nvgStrokeWidth(args.vg, 1.f);
	// 	nvgLineCap(args.vg, NVG_ROUND);
	// 	nvgBeginPath(args.vg);
	// 	nvgStrokeColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0xFF * (1 - fraction)));
	// 	nvgMoveTo(args.vg, offsets[2] - 3.5, 4.5f);
	// 	nvgLineTo(args.vg, offsets[2] - 3.5 + m_animCoords.m_arcDelta, 4.5f);
	// 	nvgStroke(args.vg);
	// 	nvgStrokeColor(args.vg, LIGHT_RED);
	// 	nvgBeginPath(args.vg);
	// 	nvgArc(args.vg, offsets[0], 4.5f, 3.5f, START_ARC, START_ARC + (END_ARC - START_ARC) * fraction, NVG_CW);
	// 	nvgStroke(args.vg);
	// 	nvgBeginPath(args.vg);
	// 	nvgArc(args.vg, offsets[1], 4.5f, 3.5f, START_ARC + (END_ARC - START_ARC) * fraction, END_ARC, NVG_CW);
	// 	nvgMoveTo(args.vg, offsets[1] - 3.5, 4.5f);
	// 	nvgLineTo(args.vg, offsets[1] - 3.5 + (m_animCoords.m_arcDelta) * fraction, 4.5f);
	// 	nvgStroke(args.vg);
	// } else {
	// 	nvgFillColor(args.vg, LIGHT_RED);
	// 	if (status == timeseq::TimeSeqCore::Status::EMPTY) {
	// 		nvgBeginPath(args.vg);
	// 		nvgStrokeWidth(args.vg, 1.f);
	// 		nvgRoundedRect(args.vg, 0.f, 0.f, 39.f, 11.f, 2.f);
	// 		nvgFill(args.vg);
	// 		nvgFillColor(args.vg, DARK_RED);
	// 	}

	// 	std::shared_ptr<window::Font> font = APP->window->loadFont("res/fonts/Nunito-Bold.ttf");
	// 	if (font && font->handle >= 0) {
	// 		nvgBeginPath(args.vg);
	// 		nvgFontFaceId(args.vg, font->handle);
	// 		nvgTextLetterSpacing(args.vg, 0.0);
	// 		nvgFontSize(args.vg, 11.5f);
	// 		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	// 		nvgText(args.vg, box.getWidth() / 2, 5.5f, status == timeseq::TimeSeqCore::Status::EMPTY ? (m_error ? "ERROR" : "EMPTY") : (m_assert ? "ASSERT" : "PAUSED"), NULL);
	// 		nvgFill(args.vg);
	// 	}
	// }

	// if (voltagePoints->size() > 0) {
	// 	int offset = 0;
	// 	for (std::vector<TimeSeqVoltagePoints>::iterator it = voltagePoints->begin(); it != voltagePoints->end(); it++) {
	// 		float v = std::min(std::max(it->voltage, -10.f), 10.f);
	// 		float factor = it->age < 31 ? (30.f - it->age) / 30.f : 0.f;

	// 		nvgStrokeColor(args.vg, nvgRGBA(0xBB, 0x45, 0x45, 0x60 + 0x9F * factor));
	// 		nvgFillColor(args.vg, nvgRGBA(0x6D, 0x2D, 0x2D, 0x60 + 0x9F * factor));

	// 		nvgBeginPath(args.vg);
	// 		nvgStrokeWidth(args.vg, 1.f);
	// 		nvgRoundedRect(args.vg, 1.f, 166.f - offset, 37.f, 9.f, 2.f);
	// 		nvgStroke(args.vg);

	// 		nvgBeginPath(args.vg);
	// 		nvgStrokeWidth(args.vg, 0.f);
	// 		nvgRoundedRect(args.vg, 19.5f, 167.f - offset, 17.5f * (v / 10.f), 7.f, 1.f);
	// 		nvgFill(args.vg);
	// 		nvgStroke(args.vg);

	// 		offset += 11;
	// 	}
	// }

	// nvgResetScissor(args.vg);
	// nvgRestore(args.vg);
}

void RatriligDisplay::onResize(const ResizeEvent& e) {
	// m_animCoords.m_arcDelta = box.getWidth() / 4;
	// m_animCoords.m_arcOffset = m_animCoords.m_arcDelta / 2;
	// m_animCoords.m_offsetCircles[0][0] = m_animCoords.m_arcOffset;
	// m_animCoords.m_offsetCircles[0][1] = m_animCoords.m_arcOffset + m_animCoords.m_arcDelta * 3;
	// m_animCoords.m_offsetCircles[0][2] = m_animCoords.m_arcOffset + m_animCoords.m_arcDelta * 2;
	// m_animCoords.m_offsetCircles[1][0] = m_animCoords.m_arcOffset + m_animCoords.m_arcDelta;
	// m_animCoords.m_offsetCircles[1][1] = m_animCoords.m_arcOffset;
	// m_animCoords.m_offsetCircles[1][2] = m_animCoords.m_arcOffset + m_animCoords.m_arcDelta * 3;
	// m_animCoords.m_offsetCircles[2][0] = m_animCoords.m_arcOffset + m_animCoords.m_arcDelta * 2;
	// m_animCoords.m_offsetCircles[2][1] = m_animCoords.m_arcOffset + m_animCoords.m_arcDelta;
	// m_animCoords.m_offsetCircles[2][2] = m_animCoords.m_arcOffset;
	// m_animCoords.m_offsetCircles[3][0] = m_animCoords.m_arcOffset + m_animCoords.m_arcDelta * 3;
	// m_animCoords.m_offsetCircles[3][1] = m_animCoords.m_arcOffset + m_animCoords.m_arcDelta * 2;
	// m_animCoords.m_offsetCircles[3][2] = m_animCoords.m_arcOffset + m_animCoords.m_arcDelta;
}

#define PROB_IDX_DENSITY = 0
#define PROB_IDX_BIAS = 1

void RatriligDisplay::setData(RatriligData* data) {
	*m_data = *data;
}
