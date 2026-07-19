#include "modules/timeseq-output.hpp"
#include "components/ntport.hpp"
#include "components/leddisplay.hpp"
#include "components/lights.hpp"
#include "modules/timeseq.hpp"

extern Model* modelTimeSeq;
extern Model* modelTimeSeqOutputExpander;

TimeSeqOutputModule::TimeSeqOutputModule() {
	config(NUM_PARAMS, NUM_INPUTS, 8, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configOutput(TimeSeqModule::OUT_OUTPUTS + i, string::f("Output %d", i + 1));
	}
}

void TimeSeqOutputModule::draw(const widget::Widget::DrawArgs& args) {
	int offset = 9;

	Expander* expander = &getLeftExpander();
	while ((expander->module != nullptr) && (expander->module->getModel() == modelTimeSeqOutputExpander)) {
		offset += 8;
		expander = &expander->module->getLeftExpander();
	}

	if ((expander->module != nullptr) && (expander->module->getModel() == modelTimeSeq) && offset < 90) {
		if (m_lastOffset != offset) {
			m_lastOffset = offset;
			for (int i = 0; i < 8; i++) {
				m_ledDisplays[i]->setForegroundText(string::f("%d", i + offset));
			}
		}
	} else if (m_lastOffset != -1) {
		m_lastOffset = -1;
		for (int i = 0; i < 8; i++) {
			m_ledDisplays[i]->setForegroundText("--");
			outputs[TimeSeqModule::OutputId::OUT_OUTPUTS + i].setChannels(1);
			outputs[TimeSeqModule::OutputId::OUT_OUTPUTS + i].setVoltage(0.f);
		}
	}
}

void TimeSeqOutputModule::onPortChange(const PortChangeEvent& e) {
	Expander* expander = &getLeftExpander();
	for (int i = 0; i < 11; i++) {
		if ((expander->module != nullptr) && (expander->module->getModel() == modelTimeSeqOutputExpander)) {
			expander = &expander->module->getLeftExpander();
		} else if ((expander->module != nullptr) && (expander->module->getModel() == modelTimeSeq)) {
			dynamic_cast<TimeSeqModule*>(expander->module)->setOutputsDirty();
		}
	}
}

TimeSeqOutputWidget::TimeSeqOutputWidget(TimeSeqOutputModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "timeseq-output") {
	float xIn = 25;
	float y = 41.5;
	float yDelta = 40;
	for (int i = 0; i < 8; i++) {
		addOutput(createOutputCentered<NTPort>(Vec(xIn, y), module, TimeSeqModule::OUT_OUTPUTS + i));
		y += yDelta;

		LEDDisplay* pDisplay = new LEDDisplay(nvgRGB(0xFF, 0x60, 0x60), nvgRGB(0x40, 0x40, 0x40), "88", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, true);
		pDisplay->box.pos = Vec(3.f + 15.5f - 1.5f, 52.f + (yDelta * i) + 2.5f);
		pDisplay->box.size = Vec(13.f + 3.f, 12.5f);
		pDisplay->setForegroundText("1");
		addChild(pDisplay);

		if (module) {
			module->m_ledDisplays[i] = pDisplay;
		}
	}
}


Model* modelTimeSeqOutputExpander = createModel<TimeSeqOutputModule, TimeSeqOutputWidget>("timeseq-output-expander");