#pragma once
#include <array>
#include "not-things.hpp"

#include <random>


struct RameligData;

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
		OUT_RANDOM,
		NUM_OUTPUTS
	};
	enum LightId {
		ENUMS(LIGHT_SCALE_NOTES, 12),
		LIGHT_RANDOM_JUMP,
		NUM_LIGHTS
	};

	RameligModule();
	~RameligModule();

	void process(const ProcessArgs& args) override;

	private:
		rack::dsp::TSchmittTrigger<float> m_trigger;

		RameligData* m_data;

		std::minstd_rand m_generator;
		std::uniform_real_distribution<float> m_distribution;

		std::vector<float> m_notes;
};

struct RameligWidget : NTModuleWidget {
	RameligWidget(RameligModule* module);
};