#pragma once
#include <array>
#include "not-things.hpp"


struct PolySameDiffModule : NTModule {
	enum ParamId {
		PARAM_DELTA,
		PARAM_MODE,
		NUM_PARAMS
	};
	enum InputId {
		IN_A,
		IN_B,
		NUM_INPUTS
	};
	enum OutputId {
		OUT_A,
		OUT_AB,
		OUT_B,
		NUM_OUTPUTS
	};
	enum LightId {
		NUM_LIGHTS
	};

	PolySameDiffModule();

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;

	void process(const ProcessArgs& args) override;

	void onAdd(const AddEvent& e) override;
	void onPortChange(const PortChangeEvent& event) override;
	void onUnBypass(const UnBypassEvent& e) override;

	bool getOutputDuplicates();
	void setOutputDuplicates(bool outputDuplicates);

	void setModuleWidget(ModuleWidget *moduleWidget);

	private:
		bool m_outputDuplicates = false;
		float m_floatBuffA[16];
		float m_floatBuffB[16];

		ModuleWidget *m_moduleWidget = nullptr;
		dsp::ClockDivider m_clockDivider;
		bool m_aConnected = true;
		bool m_bConnected = true;
		bool m_abConnected = true;

		void updateConnectionStatus();
	};

struct PolySameDiffWidget : NTModuleWidget {
	PolySameDiffWidget(PolySameDiffModule* module);

	virtual void appendContextMenu(Menu* menu) override;
	void switchOutputDuplicates();
};
