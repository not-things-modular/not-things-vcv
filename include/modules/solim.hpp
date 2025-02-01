#pragma once
#include <array>
#include "not-things.hpp"

#include "core/solim-core.hpp"
#include "components/notedisplay.hpp"
#include "modules/solim-random.hpp"
#ifdef __NT_DEBUG__
	#include "util/avgduration.hpp"
#endif


struct SolimExpanders {
	SolimRandomModule* solimRandom;
	Module* solimInputOctaver;
	Module* solimOutputOctaver;

	int inputCount;
	int outputCount;
	std::array<std::vector<Input>::iterator, 8> inputIterators;
	std::array<std::vector<Output>::iterator, 8> outputIterators;
	std::array<std::array<bool, 8>*, 8> connectedOutputPorts;
	std::array<SolimOutputMode, 8> outputModes;
	std::array<std::vector<Light>::iterator, 8> lightIterators;
	std::array<Light*, 8> polyphonicLights;

	int activeOutputs;
};


struct SolimModule : NTModule, DrawListener {
	enum ParamId {
		PARAM_LOWER_LIMIT,
		PARAM_UPPER_LIMIT,
		PARAM_SORT,
		NUM_PARAMS
	};
	enum InputId {
		ENUMS(IN_INPUTS, 8),
		IN_LOWER_LIMIT,
		IN_UPPER_LIMIT,
		IN_SORT,
		NUM_INPUTS
	};
	enum OutputId {
		ENUMS(OUT_OUTPUTS, 8),
		OUT_DEBUG_PROCESS_DURATION,
		NUM_OUTPUTS
	};
	enum LightId {
		OUT_POLYPHONIC_LIGHT,
		ENUMS(OUT_LIGHTS, 8),
		NUM_LIGHTS
	};

	enum Limits {
		LIMIT_UPPER,
		LIMIT_LOWER
	};

	enum ProcessRate {
		DIVIDED,
		AUDIO,
		RATE_COUNT
	};

	NoteDisplay* m_upperDisplay = nullptr;
	NoteDisplay* m_lowerDisplay = nullptr;

	SolimModule();
	SolimModule(SolimCore* solimCore); // For unit testing with mocks
	~SolimModule();

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;

	void process(const ProcessArgs& args) override;
	void draw(const widget::Widget::DrawArgs& args) override;
	void onSampleRateChange(const SampleRateChangeEvent& sampleRateChangeEvent) override;
	void onPortChange(const PortChangeEvent& event) override;

	ProcessRate getProcessRate();
	void setProcessRate(ProcessRate processRate);

	SolimOutputMode getOutputMode();
	void setOutputMode(SolimOutputMode outputMode);

	std::array<bool, 8>& getConnectedPorts();

	private:
		dsp::ClockDivider clockDivider;
		ProcessRate m_processRate = ProcessRate::AUDIO;

		SolimCore *m_solimCore;
		SolimExpanders m_solimExpanders;
		bool m_hasRandomModule = false;

		bool m_lastRandom = false;
		std::array<int, 8> m_lastRandMoveCounters = { 0, 0, 0, 0, 0, 0, 0, 0 };
		std::array<int, 8> m_lastRandOneCounters = { 0, 0, 0, 0, 0, 0, 0, 0 };
		std::array<int, 8> m_lastRandAllCounters = { 0, 0, 0, 0, 0, 0, 0, 0 };
		std::array<int, 8> m_lastRandResetCounters = { 0, 0, 0, 0, 0, 0, 0, 0 };
		std::array<RandomTrigger, 8> m_randomTriggers = { RandomTrigger::NONE, RandomTrigger::NONE, RandomTrigger::NONE, RandomTrigger::NONE, RandomTrigger::NONE, RandomTrigger::NONE, RandomTrigger::NONE, RandomTrigger::NONE };

		float m_lastUpperDisplayed = -1.f;
		float m_lastLowerDisplayed = -1.f;

		SolimOutputMode m_outputMode = OUTPUT_MODE_MONOPHONIC;
		std::array<bool, 8> m_connectedPorts = { false };

		#ifdef __NT_DEBUG__
			AvgDuration<256> m_avgDuration;
		#endif

		float getCvOrParamVoltage(InputId inputId, ParamId paramId, int channel);
		void detectExpanders();
		void readValues();
		void writeValues();
};

struct SolimWidget : NTModuleWidget {
	SolimWidget(SolimModule* module);

	virtual void appendContextMenu(Menu* menu) override;
	void switchProcessRate();
	void switchOutputMode();
};