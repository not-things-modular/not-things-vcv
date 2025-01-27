#include "not-things.hpp"
#include "components/ntpanel.hpp"
#include "components/ntscrew.hpp"

extern Plugin* pluginInstance;


json_t *NTModule::dataToJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "ntTheme", json_integer(this->themeId));
	return rootJ;
}

void NTModule::dataFromJson(json_t *rootJ) {
	json_t *ntThemeJson = json_object_get(rootJ, "ntTheme");
	if (ntThemeJson) {
		json_int_t ntThemeNumber = json_integer_value(ntThemeJson);
		if (ntThemeNumber > 0 && ntThemeNumber < ThemeIds::THEME_COUNT) {
			this->themeId = static_cast<ThemeIds>(ntThemeNumber);
		} else {
			this->themeId = ThemeIds::VCV;
		}
	}
}	

void NTModule::setTheme(ThemeIds& themeId) {
	this->themeId = themeId;
	for (ThemeChangeListener* listener : this->themeChangeListeners) {
		listener->themeChanged(themeId);
	}
}

void NTModule::addThemeChangeListener(ThemeChangeListener* listener) {
	this->themeChangeListeners.push_back(listener);
	listener->themeChanged(themeId);
}

NTModuleWidget::NTModuleWidget(Module* module, std::string slug) {
	setModule(module);
    std::string svgPath = "res/" + slug;
	setPanel(createNTPanel(asset::plugin(pluginInstance, svgPath + ".svg"), asset::plugin(pluginInstance, svgPath + "-dark.svg"), dynamic_cast<DrawListener*>(module)));

    addChild(createWidget<NTScrew>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
    addChild(createWidget<NTScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	if (getPanel()->box.getWidth() > 45) {
        addChild(createWidget<NTScrew>(Vec(0, 0)));
        addChild(createWidget<NTScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }
}

void NTModuleWidget::setPanel(Widget* panel) {
	ModuleWidget::setPanel(panel);
	addThemeChangeListener(panel);
}

void NTModuleWidget::addChild(Widget* child) {
	ModuleWidget::addChild(child);
	addThemeChangeListener(child);
}

void NTModuleWidget::addInput(PortWidget* input) {
	ModuleWidget::addInput(input);
	addThemeChangeListener(input);
}

void NTModuleWidget::addOutput(PortWidget* output) {
	ModuleWidget::addOutput(output);
	addThemeChangeListener(output);
}

void NTModuleWidget::appendContextMenu(Menu* menu) {
	ThemeIds currentThemeId = getModule() ? getNTModule()->themeId : ThemeIds::VCV;
	menu->addChild(new MenuSeparator);
	menu->addChild(createSubmenuItem("Panel theme", "",
		[this, currentThemeId](Menu* menu) {
			menu->addChild(createCheckMenuItem("Follow VCV Panel Theme", "", [currentThemeId]() { return currentThemeId == ThemeIds::VCV; }, [this]() { this->setTheme(ThemeIds::VCV); }));
			menu->addChild(createCheckMenuItem("Light", "", [currentThemeId]() { return currentThemeId == ThemeIds::LIGHT; }, [this]() { this->setTheme(ThemeIds::LIGHT); }));
			menu->addChild(createCheckMenuItem("Dark", "", [currentThemeId]() { return currentThemeId == ThemeIds::DARK; }, [this]() { this->setTheme(ThemeIds::DARK); }));
		}
	));
}

void NTModuleWidget::addThemeChangeListener(Widget* widget) {
	if (getModule()) {
		ThemeChangeListener* listener = dynamic_cast<ThemeChangeListener*>(widget);
		if ((listener) && (getModule())) {
			NTModule* ntModule = getNTModule();
			assert(ntModule);
			ntModule->addThemeChangeListener(listener);
		}
	}
}

void NTModuleWidget::setTheme(ThemeIds themeId) {
	if (getModule()) {
		getNTModule()->setTheme(themeId);
	}
}

NTModule* NTModuleWidget::getNTModule() {
	if (getModule()) {
		NTModule* module = dynamic_cast<NTModule*>(getModule());
		assert(module);
		return module;
	}

	return nullptr;
}
