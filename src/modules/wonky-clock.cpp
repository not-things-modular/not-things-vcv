#include "modules/wonky-clock.hpp"
#include "components/leddisplay.hpp"
#include "components/lights.hpp"
#include "components/ntknob.hpp"
#include "components/ntport.hpp"

#include "core/wonky-core.hpp"

using namespace wonky;

WonkyClockModule::WonkyClockModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	ParamQuantity* pq = configParam(PARAM_BPM, 10.f, 400.f, 120.f, "Clock Speed (Beats per Minute)");
	pq->snapEnabled = true;
	pq->smoothEnabled = false;

	configInput(IN_BPM, "Bpm");
	configInput(IN_RUN, "Run");
	configInput(IN_RESET, "Reset");

	configButton(PARAM_RUN, "Run");
	configButton(PARAM_RESET, "Reset");

	configParam(PARAM_WOBBLE_AMOUNT, 0.f, 40.f, 9.f, "Wobble Amount");
	configParam(PARAM_WOBBLE_PROBABILITY, 0.f, 100.f, 50.f, "Wobble Probability");
	configParam(PARAM_WAVER_AMOUNT, 0.f, 40.f, 5.f, "Waver Amount");
	configParam(PARAM_WAVER_PROBABILITY, 0.f, 100.f, 25.f, "Waver Probability");
	configParam(PARAM_WANDER_AMOUNT, 0.f, 40.f, 5.f, "Wander Amount");
	configParam(PARAM_WANDER_RATE, 0.f, 100.f, 25.f, "Wander Rate");

	configSwitch(PARAM_LINK, 0.f, 1.f, 1.f, "Link", { "Unlinked", "Linked" });

	configParam(PARAM_WEIGHT, 0.f, 100.f, 10.f, "Distribution Weight");

	configOutput(OUT_CLOCK, "Clock");

	m_displayedBpm = 120;

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
		lights[LightId::LIGHT_CLOCK].setBrightness(0.f);
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
		inputData.wanderRate = params[PARAM_WANDER_RATE].getValue();

		m_core->process(inputData);
	}
}

void WonkyClockModule::draw(const widget::Widget::DrawArgs& args) {
	int bpm = params[PARAM_BPM].getValue();

	if (bpm != m_displayedBpm) {
		m_displayedBpm = bpm;
		if (m_bmpLed != nullptr) {
			m_bmpLed->setForegroundText(string::f("%d", m_displayedBpm));
		}
	}

	lights[LightId::LIGHT_RESET].setBrightnessSmooth(0.f, .01f, 20.f);
}

float WonkyClockModule::getSampleRate() const {
	return APP->engine->getSampleRate();
}

void WonkyClockModule::clockGateChanged(bool high) {
	outputs[OUT_CLOCK].setVoltage(high ? 10.f : 0.f);
	lights[LightId::LIGHT_CLOCK].setBrightness(high);
}

WonkyClockWidget::WonkyClockWidget(WonkyClockModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "wonky-clock") {
	NTKnob50* bpmKnob = createParamCentered<NTKnob50>(Vec(42.f, 66.f), module, WonkyClockModule::PARAM_BPM);
	bpmKnob->setAngles(-.85 * M_PI, .85 * M_PI);
	addParam(bpmKnob);

	addParam(createParamCentered<NTKnob35>(Vec(113.5f, 166.f), module, WonkyClockModule::PARAM_WOBBLE_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(120.f, 210.f), module, WonkyClockModule::PARAM_WOBBLE_PROBABILITY));

	addParam(createParamCentered<NTKnob35>(Vec(113.5f - 80.f, 166.f), module, WonkyClockModule::PARAM_WAVER_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(120.f - 80.f, 210.f), module, WonkyClockModule::PARAM_WAVER_PROBABILITY));

	addParam(createParamCentered<NTKnob35>(Vec(113.5f, 166.f - 112.4f), module, WonkyClockModule::PARAM_WANDER_AMOUNT));
	addParam(createParamCentered<Trimpot>(Vec(120.f, 210.f - 112.4f), module, WonkyClockModule::PARAM_WANDER_RATE));

	addParam(createParamCentered<CKSSThreeHorizontal>(Vec(80.5f, 252.f), module, WonkyClockModule::PARAM_LINK));

	addParam(createParamCentered<Trimpot>(Vec(118.f, 293.5f), module, WonkyClockModule::PARAM_WEIGHT));

	addInput(createInputCentered<NTPort>(Vec(28.f, 333.f), module, WonkyClockModule::IN_RUN));
	addParam(createLightParamCentered<LEDLightBezel<RedLight>>(Vec(28.f, 285.f), module, WonkyClockModule::PARAM_RUN, WonkyClockModule::LIGHT_RUN));
	addInput(createInputCentered<NTPort>(Vec(70.f, 333.f), module, WonkyClockModule::IN_RESET));
	addParam(createLightParamCentered<LEDLightBezel<DimmedLight<RedLight>>>(Vec(70.f, 285.f), module, WonkyClockModule::PARAM_RESET, WonkyClockModule::LIGHT_RESET));

	addOutput(createOutputCentered<NTPort>(Vec(136.f, 333.f), module, WonkyClockModule::OUT_CLOCK));

	LEDDisplay* bmpLed = new LEDDisplay(nvgRGB(0xFF, 0x50, 0x50), nvgRGB(0x40, 0x40, 0x40), "888", 20, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, true);
	bmpLed->box.pos = Vec(17.f, 93.f);
	bmpLed->box.size = Vec(50.f, 25.f);
	bmpLed->setForegroundText("120");
	addChild(bmpLed);
	if (module) {
		module->m_bmpLed = bmpLed;
	}

	addChild(createLightCentered<TinyLight<DimmedLight<GreenLight>>>(Vec(148.5f, 320.5f), module, WonkyClockModule::LIGHT_CLOCK));
}


Model* modelWonkyClock = createModel<WonkyClockModule, WonkyClockWidget>("wonky-clock");