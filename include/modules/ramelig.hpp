#pragma once
#include <array>
#include "not-things.hpp"

#include "core/ramelig-core.hpp"


struct RameligModule : NTModule, RameligActionListener {
	enum ParamId {
		PARAM_LOWER_LIMIT,
		PARAM_UPPER_LIMIT,

		PARAM_CHANCE_RANDOM_JUMP,
		PARAM_TRIG_RANDOM_JUMP,
		PARAM_CHANCE_RANDOM_MOVE,
		PARAM_TRIG_RANDOM_MOVE,

		PARAM_CHANCE_MOVE_UP,
		PARAM_CHANCE_REMAIN,
		PARAM_CHANCE_MOVE_DOWN,
		PARAM_FACTOR_MOVE_TWO,
		PARAM_FACTOR_REMAIN_REPEAT,

		PARAM_SCALE,
		ENUMS(PARAM_SCALE_NOTES, 12),

		PARAM_TRIGGER,

		NUM_PARAMS
	};
	enum InputId {
		IN_GATE,

		IN_LOWER_LIMIT,
		IN_UPPER_LIMIT,

		IN_CHANCE_RANDOM_JUMP,
		IN_CHANCE_RANDOM_MOVE,

		IN_CHANCE_MOVE_UP,
		IN_CHANCE_REMAIN,
		IN_CHANCE_MOVE_DOWN,

		IN_SCALE,

		NUM_INPUTS
	};
	enum OutputId {
		OUT_CV,
		OUT_TRIGGER,
		OUT_RANDOM_JUMP,
		OUT_RANDOM_MOVE,
		NUM_OUTPUTS
	};
	enum LightId {
		ENUMS(LIGHT_SCALE_NOTES, 12),
		LIGHT_TRIGGER,
		LIGHT_RANDOM_JUMP,
		LIGHT_RANDOM_MOVE,
		NUM_LIGHTS
	};

	enum ScaleMode {
		SCALE_MODE_DECIMAL,
		SCALE_MODE_CHROMATIC,
		NUM_SCALE_MODES
	};

	RameligModule();

	json_t* dataToJson() override;
	void dataFromJson(json_t* rootJ) override;

	void process(const ProcessArgs& args) override;
	void onPortChange(const PortChangeEvent& e) override;
	void onUnBypass(const UnBypassEvent& e) override;

	void rameligActionPerformed(int channel, RameligActions action) override;

	ScaleMode getScaleMode();
	void setScaleMode(ScaleMode scaleMode);

	private:
		RameligCore m_rameligCore;
		RameligDistributionData m_rameligDistributionData[16];
		int m_channelCount;

		ScaleMode m_scaleMode;

		rack::dsp::TSchmittTrigger<float> m_scaleButtonTriggers[12];
		std::array<bool, 12> m_scales[12];
		int m_activeScaleIndex;
		std::vector<int> m_activeScaleIndices;

		rack::dsp::TSchmittTrigger<float> m_inputTrigger[16];
		rack::dsp::BooleanTrigger m_buttonTrigger;
		rack::dsp::PulseGenerator m_triggerPulse;

		rack::dsp::BooleanTrigger m_buttonMove;
		bool m_forceMove[16];
		rack::dsp::PulseGenerator m_movePulse[16];
		rack::dsp::BooleanTrigger m_buttonJump;
		bool m_forceJump[16];
		rack::dsp::PulseGenerator m_jumpPulse[16];

		dsp::ClockDivider m_lightDivider;

		int determineActiveScale();
		void updateScale();

		void updatePolyphony(bool forceUpdateOutputs);

		float getParamValue(ParamId paramId, int channel, float lowerLimit, float upperLimit, InputId inputId, float inputScaling);
};

struct RameligWidget : NTModuleWidget {
	RameligWidget(RameligModule* module);

	void appendContextMenu(Menu* menu) override;
	void setScaleMode(RameligModule::ScaleMode scaleMode);
};