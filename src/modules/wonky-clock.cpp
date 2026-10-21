#include "modules/wonky-clock.hpp"
#include "modules/wonky-clock-cv-expander.hpp"
#include "modules/wonky-clock-output-expander.hpp"
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
const std::vector<Model*> expanderModels = { modelWonkyClockCVExpander, modelWonkyClockOutputExpander };

constexpr float minBpm = 10.f;
constexpr float maxBpm = 400.f;
constexpr float defaultBpm = 120.f;
constexpr float maxAmount = 40.f;

constexpr ClockRatioData noOutputClock = ClockRatioData(ClockRatioId::NO_RATE, ClockRatioType::RATIO_DIVIDE, 1);

float determineCVImpact(float value, WonkyClockCVExpanderModule* expanderModule, WonkyClockCVExpanderModule::InputId inputId, float min, float max) {
	float result = value;

	float cv = expanderModule->getInput(inputId).getVoltage();
	if (cv != 0.f) {
		result += (max - min) * (cv / 5.f);
		result = std::max(std::min(result, max), min);
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

	configParam(PARAM_WIGGLE_AMOUNT, 0.f, maxAmount, 9.f, "Wiggle Amount");
	configParam(PARAM_WIGGLE_RATE, 0.f, 100.f, 25.f, "Wiggle Rate");
	configParam(PARAM_WANDER_AMOUNT, 0.f, 100.f, 50.f, "Wander Amount");
	configParam(PARAM_WANDER_RATE, 0.f, 100.f, 25.f, "Wander Rate");
	configParam(PARAM_WOBBLE_AMOUNT, 0.f, maxAmount, 5.f, "Wobble Amount");
	configParam(PARAM_WOBBLE_RATE, 0.f, 100.f, 25.f, "Wobble Rate");

	configOutput(OUT_CLOCK, "Clock");

	m_displayedBpm = defaultBpm;

	m_core.reset(new WonkyCore(this, this));

	lights[LightId::LIGHT_RUN].setBrightness(m_running);
}

json_t *WonkyClockModule::dataToJson() {
	json_t *rootJ = NTModule::dataToJson();
	json_object_set_new(rootJ, "ntWonkyClockWiggleMode", json_integer(m_wiggleMode));
	json_object_set_new(rootJ, "ntWonkyClockWanderRandom", json_boolean(m_wanderRandom));
	return rootJ;
}

void WonkyClockModule::dataFromJson(json_t *rootJ) {
	NTModule::dataFromJson(rootJ);

	json_t *ntWonkyClockWiggleModeJson = json_object_get(rootJ, "ntWonkyClockWiggleMode");
	if (ntWonkyClockWiggleModeJson) {
		json_int_t wiggleMode = json_integer_value(ntWonkyClockWiggleModeJson);
		if (wiggleMode > 0 && wiggleMode < WonkyWiggleMode::WIGGLE_MODE_COUNT) {
			setWiggleMode(static_cast<WonkyWiggleMode>(wiggleMode));
		} else {
			setWiggleMode(WonkyWiggleMode::MODE_WALK);
		}
	}

	json_t *ntWonkyClockWanderRandomJson = json_object_get(rootJ, "ntWonkyClockWanderRandom");
	if (ntWonkyClockWanderRandomJson) {
		if (json_is_boolean(ntWonkyClockWanderRandomJson)) {
			bool wanderRandom = json_boolean_value(ntWonkyClockWanderRandomJson);
			setWanderRandom(wanderRandom);
		}
	}
}

void WonkyClockModule::process(const ProcessArgs& args) {
	WonkyInputData inputData;

	updateExpanders();
	m_processing = true;

	bool resetTriggered = m_buttonTrigger[TriggerId::TRIG_RESET].process(params[ParamId::PARAM_RESET].getValue()) || m_trigTriggers[TriggerId::TRIG_RESET].process(inputs[InputId::IN_RESET].getVoltage(), 0.f, 1.f);
	bool runTriggered = m_buttonTrigger[TriggerId::TRIG_RUN].process(params[ParamId::PARAM_RUN].getValue()) || m_trigTriggers[TriggerId::TRIG_RUN].process(inputs[InputId::IN_RUN].getVoltage(), 0.f, 1.f);

	if (resetTriggered) {
		m_core->reset();

		lights[LightId::LIGHT_RESET].setBrightnessSmooth(1.f, .01f);
		m_resetPulse.trigger(0.001f);
	}

	if (runTriggered) {
		m_running = !m_running;

		lights[LightId::LIGHT_RUN].setBrightness(m_running);
		m_runPulse.trigger(0.001f);
	}

	float runOutputVoltage = m_runPulse.process(args.sampleTime) ? 10.f : 0.f;
	float resetOutputVoltage = m_resetPulse.process(args.sampleTime) ? 10.f : 0.f;

	WonkyClockCVExpanderModule* cvExpanderModule = getCVExpander();
	if (cvExpanderModule != nullptr) {
		cvExpanderModule->getOutput(WonkyClockCVExpanderModule::OutputId::OUT_RUN).setVoltage(runOutputVoltage);
		cvExpanderModule->getOutput(WonkyClockCVExpanderModule::OutputId::OUT_RESET).setVoltage(resetOutputVoltage);
	}

	if (m_running) {
		inputData.wiggleMode = m_wiggleMode;
		inputData.wanderRandom = m_wanderRandom;

		inputData.bpm = params[PARAM_BPM].getValue();
		inputData.wiggleAmount = params[PARAM_WIGGLE_AMOUNT].getValue();
		inputData.wiggleRate = params[PARAM_WIGGLE_RATE].getValue();
		inputData.wanderAmount = params[PARAM_WANDER_AMOUNT].getValue();
		inputData.wanderRate = params[PARAM_WANDER_RATE].getValue();
		inputData.wobbleAmount = params[PARAM_WOBBLE_AMOUNT].getValue();
		inputData.wobbleRate = params[PARAM_WOBBLE_RATE].getValue();

		if (cvExpanderModule != nullptr) {
			inputData.bpm = determineCVImpact(inputData.bpm, cvExpanderModule, WonkyClockCVExpanderModule::InputId::IN_BPM, minBpm, maxBpm);
			inputData.wiggleAmount = determineCVImpact(inputData.wiggleAmount, cvExpanderModule, WonkyClockCVExpanderModule::InputId::IN_WIGGLE_AMOUNT, 0.f, maxAmount);
			inputData.wiggleRate = determineCVImpact(inputData.wiggleRate, cvExpanderModule, WonkyClockCVExpanderModule::InputId::IN_WIGGLE_RATE, 0.f, 100.f);
			inputData.wanderAmount = determineCVImpact(inputData.wanderAmount, cvExpanderModule, WonkyClockCVExpanderModule::InputId::IN_WANDER_AMOUNT, 0.f, 100.f);
			inputData.wanderRate = determineCVImpact(inputData.wanderRate, cvExpanderModule, WonkyClockCVExpanderModule::InputId::IN_WANDER_RATE, 0.f, 100.f);
			inputData.wobbleAmount = determineCVImpact(inputData.wobbleAmount, cvExpanderModule, WonkyClockCVExpanderModule::InputId::IN_WOBBLE_AMOUNT, 0.f, maxAmount);
			inputData.wobbleRate = determineCVImpact(inputData.wobbleRate, cvExpanderModule, WonkyClockCVExpanderModule::InputId::IN_WOBBLE_RATE, 0.f, 100.f);

			cvExpanderModule->setCurrentBpm(inputData.bpm);
		}

		WonkyClockOutputExpanderModule* outputExpanderModule = getOutputExpander();
		if (outputExpanderModule != nullptr) {
			for (int i = 0; i < 8; i++) {
				int index = static_cast<int>(outputExpanderModule->params[WonkyClockOutputExpanderModule::PARAM_RATIOS + i].getValue());
				if (index >= 0 && index < wonkyClockRatioCount) {
					inputData.clockRates[i] = &wonkyClockRatios[index].data;
				} else {
					inputData.clockRates[i] = &noOutputClock;
				}
			}
		} else {
			inputData.clockRates.fill(&noOutputClock);
		}
		m_core->process(inputData);
	}

	m_processing = false;
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

void WonkyClockModule::onSampleRateChange(const SampleRateChangeEvent& e) {
	m_core->sampleRateChanged();
}

void WonkyClockModule::onReset(const ResetEvent& e) {
	NTModule::onReset(e);

	m_wiggleMode = WonkyWiggleMode::MODE_WALK;
	m_wanderRandom = true;
}

float WonkyClockModule::getSampleRate() const {
	return APP->engine->getSampleRate();
}

void WonkyClockModule::clockGateChanged(int index, bool high) {
	if (index == -1) {
		outputs[OUT_CLOCK].setVoltage(high ? 10.f : 0.f);
	} else {
		WonkyClockOutputExpanderModule* outputExpanderModule = getOutputExpander();
		if (outputExpanderModule != nullptr) {
			outputExpanderModule->outputs[WonkyClockOutputExpanderModule::OUT_CLOCKS + index].setVoltage(high ? 10.f : 0.f);
		}
	}
}

void WonkyClockModule::wiggleChanged(float wander, float max) {
	if (m_wiggleDisplay != nullptr) {
		m_wiggleDisplay->setWonkiness(wander, max);
	}
}

void WonkyClockModule::wanderChanged(float waver, float max) {
	if (m_wanderDisplay != nullptr) {
		m_wanderDisplay->setWonkiness(waver, max);
	}
}

void WonkyClockModule::wobbleChanged(float wobble, float max) {
	if (m_wobbleDisplay != nullptr) {
		m_wobbleDisplay->setWonkiness(wobble, max);
	}
}

WonkyWiggleMode WonkyClockModule::getWiggleMode() const {
	return m_wiggleMode;
}

void WonkyClockModule::setWiggleMode(WonkyWiggleMode wiggleMode) {
	m_wiggleMode = wiggleMode;
}

bool WonkyClockModule::getWanderRandom() const {
	return m_wanderRandom;
}

void WonkyClockModule::setWanderRandom(bool wanderRandom) {
	m_wanderRandom = wanderRandom;
}

WonkyClockCVExpanderModule* WonkyClockModule::getCVExpander() {
	updateExpanders();
	return m_cvExpander;
}

WonkyClockOutputExpanderModule* WonkyClockModule::getOutputExpander() {
	updateExpanders();
	return m_outputExpander;
}

void WonkyClockModule::updateExpanders() {
	if (!m_processing) {
		std::vector<Module*> expanders;

		m_cvExpander = nullptr;
		m_outputExpander = nullptr;

		// First look for the expander modules on the right side
		getExpanders(expanderModels, expanders, true);
		// If there is at least one expander to the right, check which one it is
		if (!expanders.empty()) {
			if (expanders[0]->getModel() == modelWonkyClockCVExpander) {
				m_cvExpander = dynamic_cast<WonkyClockCVExpanderModule*>(expanders[0]);
			} else if (expanders[0]->getModel() == modelWonkyClockOutputExpander) {
				m_outputExpander = dynamic_cast<WonkyClockOutputExpanderModule*>(expanders[0]);
			}
		}
		// If there are two expanders to the right, check if the other type of expander is also present (but don't override the already-found one)
		if (expanders.size() > 1) {
			if ((expanders[1]->getModel() == modelWonkyClockCVExpander) && (m_cvExpander == nullptr)) {
				m_cvExpander = dynamic_cast<WonkyClockCVExpanderModule*>(expanders[1]);
			} else if ((expanders[1]->getModel() == modelWonkyClockOutputExpander) && (m_outputExpander == nullptr)) {
				m_outputExpander = dynamic_cast<WonkyClockOutputExpanderModule*>(expanders[1]);
			}
		}

		// If either of the expander modules have not been found yet, also look to the left side
		if ((m_cvExpander == nullptr) || (m_outputExpander == nullptr)) {
			getExpanders(expanderModels, expanders, false);
			// If there is at least one expander to the left, check if it is one we haven't found yet
			if (!expanders.empty()) {
				if ((expanders[0]->getModel() == modelWonkyClockCVExpander) && (m_cvExpander == nullptr)) {
					m_cvExpander = dynamic_cast<WonkyClockCVExpanderModule*>(expanders[0]);
				} else if ((expanders[0]->getModel() == modelWonkyClockOutputExpander) && (m_outputExpander == nullptr)) {
					m_outputExpander = dynamic_cast<WonkyClockOutputExpanderModule*>(expanders[0]);
				}
			}
			// If there are two expanders to the left and we haven't found both types of expanders yet, check if it is one we haven't found yet
			if ((m_cvExpander == nullptr) || (m_outputExpander == nullptr)) {
				if (expanders.size() > 1) {
					if ((expanders[1]->getModel() == modelWonkyClockCVExpander) && (m_cvExpander == nullptr)) {
						m_cvExpander = dynamic_cast<WonkyClockCVExpanderModule*>(expanders[1]);
					} else if ((expanders[1]->getModel() == modelWonkyClockOutputExpander) && (m_outputExpander == nullptr)) {
						m_outputExpander = dynamic_cast<WonkyClockOutputExpanderModule*>(expanders[1]);
					}
				}
			}
		}
	}
}


WonkyClockWidget::WonkyClockWidget(WonkyClockModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "wonky-clock") {
	NTKnob40* bpmKnob = createParamCentered<NTKnob40>(Vec(127.f, 107.f), module, WonkyClockModule::PARAM_BPM);
	bpmKnob->setAngles(-.85 * M_PI, .85 * M_PI);
	addParam(bpmKnob);

	addParam(createParamCentered<NTKnob35>(Vec(46.f, 163.5f), module, WonkyClockModule::PARAM_WIGGLE_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(54.57f, 208.f), module, WonkyClockModule::PARAM_WIGGLE_RATE));

	addParam(createParamCentered<NTKnob35>(Vec(132.55f, 225.5f), module, WonkyClockModule::PARAM_WANDER_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(141.7f, 270.f), module, WonkyClockModule::PARAM_WANDER_RATE));

	addParam(createParamCentered<NTKnob35>(Vec(46.f, 288.5f), module, WonkyClockModule::PARAM_WOBBLE_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(54.57f, 333.f), module, WonkyClockModule::PARAM_WOBBLE_RATE));

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
		module->m_wiggleDisplay = pDisplay;
	}

	pDisplay = new WonkyClockDisplay();
	pDisplay->box.pos = Vec(101.5f, 208.f);
	pDisplay->box.size = Vec(6.f, 80.f);
	addChild(pDisplay);
	if (module) {
		module->m_wanderDisplay = pDisplay;
	}

	pDisplay = new WonkyClockDisplay();
	pDisplay->box.pos = Vec(15.f, 271.f);
	pDisplay->box.size = Vec(6.f, 80.f);
	addChild(pDisplay);
	if (module) {
		module->m_wobbleDisplay = pDisplay;
	}
}

void WonkyClockWidget::appendContextMenu(Menu* menu) {
	NTModuleWidget::appendContextMenu(menu);

	menu->addChild(new MenuSeparator);

	WonkyWiggleMode wiggleMode = getModule() ? dynamic_cast<WonkyClockModule *>(getModule())->getWiggleMode() : WonkyWiggleMode::MODE_WALK;
	menu->addChild(createSubmenuItem("Wiggle Mode", "",
		[=](Menu* menu) {
			menu->addChild(createCheckMenuItem("Walk", "", [wiggleMode]() { return wiggleMode == WonkyWiggleMode::MODE_WALK; }, [this]() { setWiggleMode(WonkyWiggleMode::MODE_WALK); }));
			menu->addChild(createCheckMenuItem("Random", "", [wiggleMode]() { return wiggleMode == WonkyWiggleMode::MODE_RANDOM; }, [this]() { setWiggleMode(WonkyWiggleMode::MODE_RANDOM); }));
		}
	));

	bool wanderRandom = getModule() ? dynamic_cast<WonkyClockModule *>(getModule())->getWanderRandom() : true;
	menu->addChild(createSubmenuItem("Wander Amount is", "",
		[=](Menu* menu) {
			menu->addChild(createCheckMenuItem("Maximum amount of wiggle (with random percentage)", "", [wanderRandom]() { return wanderRandom; }, [this]() { setWanderRandom(true); }));
			menu->addChild(createCheckMenuItem("Exact amount of wiggle (absolute percentage)", "", [wanderRandom]() { return !wanderRandom; }, [this]() { setWanderRandom(false); }));
		}
	));
}

void WonkyClockWidget::setWiggleMode(wonky::WonkyWiggleMode wiggleMode) {
	WonkyClockModule* wonkyClockModule = dynamic_cast<WonkyClockModule *>(getModule());
	if (wonkyClockModule != nullptr) {
		wonkyClockModule->setWiggleMode(wiggleMode);
	}
}

void WonkyClockWidget::setWanderRandom(bool wanderRandom) {
	WonkyClockModule* wonkyClockModule = dynamic_cast<WonkyClockModule *>(getModule());
	if (wonkyClockModule != nullptr) {
		wonkyClockModule->setWanderRandom(wanderRandom);
	}
}


Model* modelWonkyClock = createModel<WonkyClockModule, WonkyClockWidget>("wonky-clock");