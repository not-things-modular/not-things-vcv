#pragma once
#include <array>
#include "not-things.hpp"


struct SolimOutputOctaverModule : NTModule, DrawListener {
	enum ParamId {
		ENUMS(PARAM_ADD_OCTAVE, 8),
		ENUMS(PARAM_REPLACE_ORIGINAL, 8),
		PARAM_RESORT,
		NUM_PARAMS
	};
	enum InputId {
		ENUMS(IN_ADD_OCTAVE, 8),
		ENUMS(IN_REPLACE_ORIGINAL, 8),
		NUM_INPUTS
	};
	enum OutputId {
		NUM_OUTPUTS
	};
	enum LightId {
		ENUMS(LIGHT_REPLACE_ORIGINAL, 8),
		LIGHT_CONNECTED,
		LIGHT_NOT_CONNECTED,
		LIGHT_RESORT,
		LIGHT_DONT_RESORT,
		NUM_LIGHTS
	};
	enum SortMode {
		SORT_ALL,
		SORT_CONNECTED,
		NUM_SORT_MODES
	};

	SolimOutputOctaverModule();

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;

	void draw(const widget::Widget::DrawArgs& args) override;

	SortMode getSortMode();
	void setSortMode(SortMode sortMode);

	private:
		SortMode m_sortMode = SortMode::SORT_ALL;
};

struct SolimOutputOctaverWidget : NTModuleWidget {
	SolimOutputOctaverWidget(SolimOutputOctaverModule* module);

	virtual void appendContextMenu(Menu* menu) override;
	void switchSortMode();
};