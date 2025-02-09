#pragma once
#include <rack.hpp>
using namespace rack;

// #define __NT_DEBUG__



enum ThemeId {
	VCV,
	LIGHT,
	DARK,
	NUM_THEMES
};

enum SolimOutputMode {
	OUTPUT_MODE_MONOPHONIC,
	OUTPUT_MODE_POLYPHONIC,
	NUM_OUTPUT_MODES
};


struct ThemeChangeListener {
	virtual void themeChanged(const ThemeId& themeId) = 0;
};

struct DrawListener {
	virtual void draw(const widget::Widget::DrawArgs& args) = 0;
};

struct NTModule : Module {
	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;

	ThemeId getTheme();
	void setTheme(ThemeId themeId);
	void addThemeChangeListener(ThemeChangeListener* listener);

	private:
		ThemeId m_themeId = VCV;
		std::vector<ThemeChangeListener*> m_themeChangeListeners;
};

struct NTModuleWidget : ModuleWidget {
	NTModuleWidget(Module* module, std::string slug);
	void setPanel(widget::Widget* panel);
	void addChild(Widget* child);
	void addInput(PortWidget* input);
	void addOutput(PortWidget* output);
	void addParam(ParamWidget* output);

	void appendContextMenu(Menu* menu) override;

	void addThemeChangeListener(Widget* widget);
	void setTheme(ThemeId themeId);

	NTModule* getNTModule();
};
