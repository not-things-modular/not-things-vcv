#include "modules/wonky-clock.hpp"
#include "modules/wonky-clock-cv-expander.hpp"
#include "components/leddisplay.hpp"
#include "components/lights.hpp"
#include "components/ntknob.hpp"
#include "components/ntport.hpp"
#include "components/wonkyclock-display.hpp"

#include "core/wonky-core.hpp"

using namespace wonky;

extern Model* modelWonkyClock;
extern Model* modelWonkyClockCVExpander;
extern Model* modelWonkyClockOutputExpander;

constexpr float minBpm = 10.f;
constexpr float maxBpm = 400.f;
constexpr float defaultBpm = 120.f;
constexpr float maxAmount = 40.f;

float determineCVImpact(float value, WonkyClockCVExpanderModule* expanderModule, WonkyClockCVExpanderModule::InputId inputId, float min, float max) {
	float result = value;

	float cv = expanderModule->getInput(inputId).getVoltage();
	if (cv != 0.f) {
		result += (max - min) * (cv / 5.f);
		result = std::max(std::min(result, maxBpm), minBpm);
	}

	return result;
}

WonkyClockModule::WonkyClockModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	ParamQuantity* pq = configParam(PARAM_BPM, minBpm, maxBpm, defaultBpm, "Clock Speed (Beats per Minute)");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;

	configInput(IN_RUN, "Run");
	configInput(IN_RESET, "Reset");

	configButton(PARAM_RUN, "Run");
	configButton(PARAM_RESET, "Reset");

	configParam(PARAM_WOBBLE_AMOUNT, 0.f, maxAmount, 9.f, "Wobble Amount");
	configParam(PARAM_WOBBLE_PROBABILITY, 0.f, 100.f, 50.f, "Wobble Probability");
	configParam(PARAM_WAVER_AMOUNT, 0.f, maxAmount, 5.f, "Waver Amount");
	configParam(PARAM_WAVER_PROBABILITY, 0.f, 100.f, 25.f, "Waver Probability");
	configParam(PARAM_WANDER_AMOUNT, 0.f, maxAmount, 5.f, "Wander Amount");
	configParam(PARAM_WANDER_RATE, 0.f, 100.f, 25.f, "Wander Rate");

	configSwitch(PARAM_LINK, 0.f, 1.f, 1.f, "Link Wobble and Waver", { "Unlinked", "Linked" });

	configOutput(OUT_CLOCK, "Clock");

	m_displayedBpm = defaultBpm;

	m_core.reset(new WonkyCore(this, this));

	lights[LightId::LIGHT_RUN].setBrightness(m_running);
}

void WonkyClockModule::process(const ProcessArgs& args) {
	WonkyInputData inputData;

	bool resetTriggered = m_buttonTrigger[TriggerId::TRIG_RESET].process(params[ParamId::PARAM_RESET].getValue()) || m_trigTriggers[TriggerId::TRIG_RESET].process(inputs[InputId::IN_RESET].getVoltage(), 0.f, 1.f);
	bool runTriggered = m_buttonTrigger[TriggerId::TRIG_RUN].process(params[ParamId::PARAM_RUN].getValue()) || m_trigTriggers[TriggerId::TRIG_RUN].process(inputs[InputId::IN_RUN].getVoltage(), 0.f, 1.f);

	if (resetTriggered) {
		m_core->reset();
		lights[LightId::LIGHT_RESET].setBrightnessSmooth(1.f, .01f);
	}

	if (runTriggered) {
		m_running = !m_running;
		lights[LightId::LIGHT_RUN].setBrightness(m_running);
	}

	if (m_running) {
		inputData.bpm = params[PARAM_BPM].getValue();
		inputData.wobbleAmount = params[PARAM_WOBBLE_AMOUNT].getValue();
		inputData.wobbleProbability = params[PARAM_WOBBLE_PROBABILITY].getValue();
		inputData.waverAmount = params[PARAM_WAVER_AMOUNT].getValue();
		inputData.waverProbability = params[PARAM_WAVER_PROBABILITY].getValue();
		inputData.wanderAmount = params[PARAM_WANDER_AMOUNT].getValue();
		inputData.wanderRate = params[PARAM_WANDER_RATE].getValue() / 100.f;
		inputData.linked = params[PARAM_LINK].getValue() > 0.f;

		WonkyClockCVExpanderModule* expanderModule = nullptr;
		for (int i = 0; i < 2; i++) {
			std::vector<Module*> expanders = getExpanders({ modelWonkyClockCVExpander, modelWonkyClockOutputExpander }, (i == 0));
			if ((expanders.size() > 0) && (expanders[0]->getModel() == modelWonkyClockCVExpander)) {
				expanderModule = dynamic_cast<WonkyClockCVExpanderModule*>(expanders[0]);
				break;
			} else if ((expanders.size() > 1) && (expanders[0]->getModel() == modelWonkyClockCVExpander)) {
				expanderModule = dynamic_cast<WonkyClockCVExpanderModule*>(expanders[1]);
				break;
			}
		}
		if (expanderModule != nullptr) {
			inputData.bpm = determineCVImpact(inputData.bpm, expanderModule, WonkyClockCVExpanderModule::InputId::IN_BPM, minBpm, maxBpm);
			inputData.wobbleAmount = determineCVImpact(inputData.wobbleAmount, expanderModule, WonkyClockCVExpanderModule::InputId::IN_WOBBLE_AMOUNT, 0.f, maxAmount);
			inputData.wobbleProbability = determineCVImpact(inputData.wobbleProbability, expanderModule, WonkyClockCVExpanderModule::InputId::IN_WOBBLE_PROBABILITY, 0.f, 100.f);
			inputData.waverAmount = determineCVImpact(inputData.waverAmount, expanderModule, WonkyClockCVExpanderModule::InputId::IN_WAVER_AMOUNT, 0.f, maxAmount);
			inputData.waverProbability = determineCVImpact(inputData.waverProbability, expanderModule, WonkyClockCVExpanderModule::InputId::IN_WAVER_PROBABILITY, 0.f, 100.f);
			inputData.wanderAmount = determineCVImpact(inputData.wanderAmount, expanderModule, WonkyClockCVExpanderModule::InputId::IN_WANDER_AMOUNT, 0.f, maxAmount);
			inputData.wanderRate = determineCVImpact(inputData.wanderRate, expanderModule, WonkyClockCVExpanderModule::InputId::IN_WANDER_RATE, 0.f, 100.f);

			expanderModule->setCurrentBpm(inputData.bpm);
		}

		m_core->process(inputData);
	}
}

void WonkyClockModule::draw(const widget::Widget::DrawArgs& args) {
	int bpm = params[PARAM_BPM].getValue();

	if (bpm != m_displayedBpm) {
		m_displayedBpm = bpm;
		if (m_bpmLed != nullptr) {
			m_bpmLed->setForegroundText(string::f("%d", m_displayedBpm));
		}
	}

	lights[LightId::LIGHT_RESET].setBrightnessSmooth(0.f, .01f, 20.f);
}

float WonkyClockModule::getSampleRate() const {
	return APP->engine->getSampleRate();
}

void WonkyClockModule::clockGateChanged(bool high) {
	outputs[OUT_CLOCK].setVoltage(high ? 10.f : 0.f);
}

void WonkyClockModule::wanderChanged(float wander, float max) {
	if (m_wanderDisplay != nullptr) {
		m_wanderDisplay->setWonkiness(wander, max);
	}
}

void WonkyClockModule::waverChanged(float waver, float max) {
	if (m_waverDisplay != nullptr) {
		m_waverDisplay->setWonkiness(waver, max);
	}
}

void WonkyClockModule::wobbleChanged(float wobble, float max) {
	if (m_wobbleDisplay != nullptr) {
		m_wobbleDisplay->setWonkiness(wobble, max);
	}
}

WonkyClockWidget::WonkyClockWidget(WonkyClockModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "wonky-clock") {
	NTKnob40* bpmKnob = createParamCentered<NTKnob40>(Vec(127.f, 107.f), module, WonkyClockModule::PARAM_BPM);
	bpmKnob->setAngles(-.85 * M_PI, .85 * M_PI);
	addParam(bpmKnob);

	addParam(createParamCentered<NTKnob35>(Vec(46.f, 163.5f), module, WonkyClockModule::PARAM_WOBBLE_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(54.57f, 208.f), module, WonkyClockModule::PARAM_WOBBLE_PROBABILITY));

	addParam(createParamCentered<NTKnob35>(Vec(132.55f, 225.5f), module, WonkyClockModule::PARAM_WAVER_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(141.7f, 270.f), module, WonkyClockModule::PARAM_WAVER_PROBABILITY));

	addParam(createParamCentered<NTKnob35>(Vec(46.f, 288.5f), module, WonkyClockModule::PARAM_WANDER_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(54.57f, 333.f), module, WonkyClockModule::PARAM_WANDER_RATE));

	addParam(createParamCentered<CKSS>(Vec(82.5f, 200.f), module, WonkyClockModule::PARAM_LINK));

	addInput(createInputCentered<NTPort>(Vec(25.f, 45.f), module, WonkyClockModule::IN_RUN));
	addParam(createLightParamCentered<LEDLightBezel<RedLight>>(Vec(25.f, 91.5f), module, WonkyClockModule::PARAM_RUN, WonkyClockModule::LIGHT_RUN));
	addInput(createInputCentered<NTPort>(Vec(68.f, 45.f), module, WonkyClockModule::IN_RESET));
	addParam(createLightParamCentered<LEDLightBezel<DimmedLight<RedLight>>>(Vec(68.f, 91.5f), module, WonkyClockModule::PARAM_RESET, WonkyClockModule::LIGHT_RESET));

	addOutput(createOutputCentered<NTPort>(Vec(123.75f, 332.f), module, WonkyClockModule::OUT_CLOCK));

	LEDDisplay* bpmLed = new LEDDisplay(nvgRGB(0xFF, 0x50, 0x50), nvgRGB(0x40, 0x40, 0x40), "888", 20, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, true);
	bpmLed->box.pos = Vec(101.5f, 27.f);
	bpmLed->box.size = Vec(50.f, 25.f);
	bpmLed->setForegroundText(string::f("%d", static_cast<int>(defaultBpm)));
	addChild(bpmLed);
	if (module) {
		module->m_bpmLed = bpmLed;
	}

	WonkyClockDisplay *pDisplay = new WonkyClockDisplay();
	pDisplay->box.pos = Vec(15.f, 146.f);
	pDisplay->box.size = Vec(6.f, 80.f);
	addChild(pDisplay);
	if (module) {
		module->m_wobbleDisplay = pDisplay;
	}

	pDisplay = new WonkyClockDisplay();
	pDisplay->box.pos = Vec(101.5f, 208.f);
	pDisplay->box.size = Vec(6.f, 80.f);
	addChild(pDisplay);
	if (module) {
		module->m_waverDisplay = pDisplay;
	}

	pDisplay = new WonkyClockDisplay();
	pDisplay->box.pos = Vec(15.f, 271.f);
	pDisplay->box.size = Vec(6.f, 80.f);
	addChild(pDisplay);
	if (module) {
		module->m_wanderDisplay = pDisplay;
	}
}


Model* modelWonkyClock = createModel<WonkyClockModule, WonkyClockWidget>("wonky-clock");