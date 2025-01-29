#pragma once
#include <array>
#include "not-things.hpp"


struct SolimRandomModule : NTModule, DrawListener {
	enum ParamId {
		PARAM_TRIG_MOVE,
		PARAM_TRIG_ONE,
		PARAM_TRIG_ALL,
		PARAM_TRIG_RESET,
		NUM_PARAMS
	};
	enum InputId {
		INPUT_TRIG_MOVE,
		INPUT_TRIG_ONE,
		INPUT_TRIG_ALL,
		INPUT_TRIG_RESET,
		NUM_INPUTS
	};
	enum OutputId {
		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_TRIG_MOVE,
		LIGHT_TRIG_ONE,
		LIGHT_TRIG_ALL,
		LIGHT_TRIG_RESET,
		LIGHT_CONNECTED_LEFT,
		LIGHT_CONNECTED_RIGHT,
		NUM_LIGHTS
	};
	enum TriggerId {
		TRIG_MOVE,
		TRIG_ONE,
		TRIG_ALL,
		TRIG_RESET,
		NUM_TRIGGERS
	};

	std::array<int, 8> m_moveCounters = { 0, 0, 0, 0, 0, 0, 0, 0 };
	std::array<int, 8> m_oneCounters = { 0, 0, 0, 0, 0, 0, 0, 0 };
	std::array<int, 8> m_allCounters = { 0, 0, 0, 0, 0, 0, 0, 0 };
	std::array<int, 8> m_resetCounters = { 0, 0, 0, 0, 0, 0, 0, 0 };

	SolimRandomModule();

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;

	private:
		dsp::BooleanTrigger m_buttonTrigger[TriggerId::NUM_TRIGGERS];
		dsp::TSchmittTrigger<float> m_trigTriggers[TriggerId::NUM_TRIGGERS][8];

		bool processTriggers(ParamId paramId, InputId inputId, TriggerId triggerId, std::array<int, 8>& counters);
};

struct SolimRandomWidget : NTModuleWidget {
	SolimRandomWidget(SolimRandomModule* module);
};