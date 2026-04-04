#pragma once
#include <array>
#include "not-things.hpp"

#include "core/ramelig-core.hpp"


struct RameligExpanderModule;
struct RameligDistribution;


struct RameligModule : NTModule, DrawListener, RameligActionListener {
	enum ParamId {
		PARAM_LOWER_LIMIT,
		PARAM_UPPER_LIMIT,

		PARAM_CHANCE_RANDOM_JUMP,
		PARAM_TRIG_RANDOM_JUMP,
		PARAM_CHANCE_RANDOM_SHIFT,
		PARAM_TRIG_RANDOM_SHIFT,

		PARAM_CHANCE_MOVE_UP,
		PARAM_CHANCE_STAY,
		PARAM_CHANCE_MOVE_DOWN,
		PARAM_FACTOR_MOVE_TWO,
		PARAM_FACTOR_STAY_REPEAT,

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
		IN_CHANCE_RANDOM_SHIFT,

		IN_CHANCE_MOVE_UP,
		IN_CHANCE_STAY,
		IN_CHANCE_MOVE_DOWN,

		IN_SCALE,

		NUM_INPUTS
	};
	enum OutputId {
		OUT_CV,
		OUT_TRIGGER,
		NUM_OUTPUTS
	};
	enum LightId {
		ENUMS(LIGHT_SCALE_NOTES, 12),
		LIGHT_TRIGGER,
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
	void draw(const widget::Widget::DrawArgs& args) override;
	void onPortChange(const PortChangeEvent& e) override;
	void onUnBypass(const UnBypassEvent& e) override;
	void onExpanderChange(const ExpanderChangeEvent& e) override;

	void rameligActionPerformed(int channel, RameligActions action) override;

	ScaleMode getScaleMode();
	void setScaleMode(ScaleMode scaleMode);

	void setRameligDistribution(RameligDistribution* rameligDistribution);

	private:
		RameligCore m_rameligCore;
		int m_channelCount;

		RameligDistribution* m_rameligDistribution;

		ScaleMode m_scaleMode;

		rack::dsp::TSchmittTrigger<float> m_scaleButtonTriggers[12] = {};
		std::array<bool, 12> m_scales[12] = {};
		int m_activeScaleIndex;
		std::vector<int> m_activeScaleIndices = {};

		rack::dsp::TSchmittTrigger<float> m_inputTrigger[16] = {};
		rack::dsp::BooleanTrigger m_buttonTrigger;
		rack::dsp::PulseGenerator m_triggerPulse;

		rack::dsp::BooleanTrigger m_buttonJump;
		rack::dsp::TSchmittTrigger<float> m_triggerJump[16] = {};
		bool m_forceJump[16] = {};
		rack::dsp::BooleanTrigger m_buttonShift;
		rack::dsp::TSchmittTrigger<float> m_triggerShift[16] = {};
		bool m_forceShift[16] = {};

		dsp::ClockDivider m_triggerLightDivider;

		std::array<float, 16> m_values = {};
		std::array<bool, 16> m_jumped = {};
		std::array<bool, 16> m_shifted = {};

		void readDistributionData(int channel, RameligDistributionData& rameligDistributionData);

		int determineActiveScale();
		void updateScale();

		void updatePolyphony(bool forceUpdateOutputs);

		float getParamValue(ParamId paramId, int channel, float lowerLimit, float upperLimit, InputId inputId, float inputScaling);
		RameligExpanderModule* getRameligExpander();
};

struct RameligWidget : NTModuleWidget {
	RameligWidget(RameligModule* module);

	void appendContextMenu(Menu* menu) override;
	void setScaleMode(RameligModule::ScaleMode scaleMode);
};