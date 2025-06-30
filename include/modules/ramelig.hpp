#pragma once
#include <array>
#include "not-things.hpp"

#include <random>


struct RameligData;

struct RameligModule : NTModule {
	enum ParamId {
		NUM_PARAMS
	};
	enum InputId {
		IN_GATE,
		IN_LOWER_LIMIT,
		IN_UPPER_LIMIT,
		NUM_INPUTS
	};
	enum OutputId {
		OUT_CV,
		OUT_GATE,
		NUM_OUTPUTS
	};
	enum LightId {
		LIGHT_CONNECTED,
		LIGHT_NOT_CONNECTED,
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