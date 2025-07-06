#pragma once
#include <array>
#include "not-things.hpp"

#include "core/ramelig-core.hpp"


struct RameligModule : NTModule {
	enum ParamId {
		PARAM_LOWER_LIMIT,
		PARAM_UPPER_LIMIT,

		PARAM_CHANCE_RANDOM_JUMP,
		PARAM_TRIG_RANDOM_JUMP,
		PARAM_FACTOR_JUMP_REMAIN,
		
		PARAM_CHANCE_MOVE_UP,
		PARAM_CHANCE_REMAIN,
		PARAM_CHANCE_MOVE_DOWN,
		PARAM_FACTOR_MOVE_TWO,
		PARAM_FACTOR_REMAIN_REPEAT,
		
		PARAM_SCALE,
		ENUMS(PARAM_SCALE_NOTES, 12),
		
		NUM_PARAMS
	};
	enum InputId {
		IN_GATE,
		
		IN_LOWER_LIMIT,
		IN_UPPER_LIMIT,

		IN_CHANCE_RANDOM_JUMP,

		IN_CHANCE_MOVE_UP,
		IN_CHANCE_REMAIN,
		IN_CHANCE_MOVE_DOWN,

		IN_SCALE,
		
		NUM_INPUTS
	};
	enum OutputId {
		OUT_CV,
		OUT_GATE,
		OUT_RANDOM_JUMP,
		OUT_RANDOM_REMAIN,
		NUM_OUTPUTS
	};
	enum LightId {
		ENUMS(LIGHT_SCALE_NOTES, 12),
		LIGHT_RANDOM_JUMP,
		NUM_LIGHTS
	};

	RameligModule();

	void process(const ProcessArgs& args) override;

	private:
		RameligCore m_rameligCore;
		RameligCoreData m_rameligCoreData;

		rack::dsp::TSchmittTrigger<float> m_trigger;
};

struct RameligWidget : NTModuleWidget {
	RameligWidget(RameligModule* module);
};