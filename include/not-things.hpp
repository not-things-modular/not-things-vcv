#pragma once
#include <rack.hpp>
using namespace rack;

// #define __NT_DEBUG__



enum ThemeIds {
    VCV,
    LIGHT,
    DARK,
    THEME_COUNT
};

enum SolimOutputMode {
	OUTPUT_MODE_MONOPHONIC,
	OUTPUT_MODE_POLYPHONIC,
	OUTPUT_MODE_COUNT
};


struct ThemeChangeListener {
	virtual void themeChanged(const ThemeIds& themeId) = 0;
};

struct DrawListener {
	virtual void draw(const widget::Widget::DrawArgs& args) = 0;
};

struct NTModule : Module {
    ThemeIds themeId = VCV;
	std::vector<ThemeChangeListener*> themeChangeListeners;

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;

	void setTheme(ThemeIds& themeId);
	void addThemeChangeListener(ThemeChangeListener* listener);
};

struct NTModuleWidget : ModuleWidget {
    NTModuleWidget(Module* module, std::string slug);
	void setPanel(widget::Widget* panel);
	void addChild(Widget* child);
	void addInput(PortWidget* input);
	void addOutput(PortWidget* output);

    void appendContextMenu(Menu* menu) override;

	void addThemeChangeListener(Widget* widget);
    void setTheme(ThemeIds themeId);

	NTModule* getNTModule();
};
