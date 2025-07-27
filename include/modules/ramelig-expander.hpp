#pragma once
#include <array>
#include "not-things.hpp"


struct RameligExpanderModule : NTModule {
	enum ParamId {
		PARAM_QUANTIZE,
		NUM_PARAMS
	};
	enum InputId {
		IN_TRIG_JUMP,
		IN_TRIG_MOVE,
		IN_GUIDE_CV,
		IN_GUIDE_GATE,
		NUM_INPUTS
	};
	enum OutputId {
		OUT_TRIG_JUMP,
		OUT_TRIG_MOVE,
		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_TRIG_JUMP,
		LIGHT_TRIG_MOVE,
		LIGHT_QUANTIZE,
		NUM_LIGHTS
	};

	RameligExpanderModule();

	void process(const ProcessArgs& args) override;

	void triggerJump();
	void triggerMove();

	private:
		rack::dsp::PulseGenerator m_jumpPulse;
		rack::dsp::PulseGenerator m_movePulse;
};

struct RameligExpanderWidget : NTModuleWidget {
	RameligExpanderWidget(RameligExpanderModule* module);
};