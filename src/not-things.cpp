#include "not-things.hpp"
#include "components/ntpanel.hpp"
#include "components/ntscrew.hpp"

extern Plugin* pluginInstance;


json_t *NTModule::dataToJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "ntTheme", json_integer(m_themeId));
	return rootJ;
}

void NTModule::dataFromJson(json_t *rootJ) {
	json_t *ntThemeJson = json_object_get(rootJ, "ntTheme");
	if (ntThemeJson) {
		json_int_t ntThemeNumber = json_integer_value(ntThemeJson);
		if (ntThemeNumber > 0 && ntThemeNumber < ThemeId::NUM_THEMES) {
			m_themeId = static_cast<ThemeId>(ntThemeNumber);
		} else {
			m_themeId = ThemeId::VCV;
		}
	}
}

ThemeId NTModule::getTheme() {
	return m_themeId;
}

void NTModule::setTheme(ThemeId themeId) {
	m_themeId = themeId;
	for (ThemeChangeListener* listener : m_themeChangeListeners) {
		listener->themeChanged(themeId);
	}
}

void NTModule::addThemeChangeListener(ThemeChangeListener* listener) {
	m_themeChangeListeners.push_back(listener);
	listener->themeChanged(m_themeId);
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
	ThemeId currentThemeId = getModule() ? getNTModule()->getTheme() : ThemeId::VCV;
	menu->addChild(new MenuSeparator);
	menu->addChild(createSubmenuItem("Panel theme", "",
		[this, currentThemeId](Menu* menu) {
			menu->addChild(createCheckMenuItem("Follow VCV Panel Theme", "", [currentThemeId]() { return currentThemeId == ThemeId::VCV; }, [this]() { setTheme(ThemeId::VCV); }));
			menu->addChild(createCheckMenuItem("Light", "", [currentThemeId]() { return currentThemeId == ThemeId::LIGHT; }, [this]() { setTheme(ThemeId::LIGHT); }));
			menu->addChild(createCheckMenuItem("Dark", "", [currentThemeId]() { return currentThemeId == ThemeId::DARK; }, [this]() { setTheme(ThemeId::DARK); }));
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

void NTModuleWidget::setTheme(ThemeId themeId) {
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
