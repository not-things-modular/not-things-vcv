#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "modules/pipo-input.hpp"
#include "modules/pipo-output.hpp"
#include "components/leddisplay.hpp"

#include "../common/vcv-registration.hpp"

extern Model *modelPipoOutput;
extern Model *modelPipoInput;

struct DummyWidget : ModuleWidget {
	DummyWidget(Module* module) {}
};

void initializePipoInputModule(PipoInputModule& pipoInputModule) {
	for (int i = 0; i < 8; i++) {
		pipoInputModule.m_ledDisplays[i] = new LEDDisplay(nvgRGB(0xFF, 0x50, 0x50), nvgRGB(0x40, 0x40, 0x40), "18", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP, true);
		pipoInputModule.m_ledDisplays[i]->setForegroundText("-1");
	}
	pipoInputModule.lights[PipoInputModule::LIGHT_CONNECTED].setBrightness(4.20f);
	pipoInputModule.lights[PipoInputModule::LIGHT_NOT_CONNECTED].setBrightness(4.20f);
}

void expectConnected(PipoInputModule& pipoInputModule, bool connected) {
	if (connected) {
		EXPECT_NEAR(pipoInputModule.lights[PipoInputModule::LightId::LIGHT_CONNECTED].getBrightness(), 1.f, 0.0001f);
		EXPECT_NEAR(pipoInputModule.lights[PipoInputModule::LightId::LIGHT_NOT_CONNECTED].getBrightness(), 0.f, 0.0001f);
	} else {
		EXPECT_NEAR(pipoInputModule.lights[PipoInputModule::LightId::LIGHT_CONNECTED].getBrightness(), 0.f, 0.0001f);
		EXPECT_NEAR(pipoInputModule.lights[PipoInputModule::LightId::LIGHT_NOT_CONNECTED].getBrightness(), 1.f, 0.0001f);
	}
}

TEST(PipoInputTest, WithNoExpandersShouldDeactivateLed) {
	PipoInputModule pipoInputModule;
	initializePipoInputModule(pipoInputModule);

	pipoInputModule.draw(widget::Widget::DrawArgs());

	expectConnected(pipoInputModule, false);
}

TEST(PipoInputTest, WithPipoOutputOnRightShouldActivateLed) {
	PipoInputModule pipoInputModule;
	PipoOutputModule pipoOutputModule;
	initializePipoInputModule(pipoInputModule);

	registerExpanderModule(pipoInputModule, ExpanderData(pipoOutputModule, modelPipoOutput, ExpanderSide::RIGHT));
	pipoInputModule.draw(widget::Widget::DrawArgs());

	expectConnected(pipoInputModule, true);
}

TEST(PipoInputTest, WithPipoOutputOnLeftShouldDeactivateLed) {
	PipoInputModule pipoInputModule;
	PipoOutputModule pipoOutputModule;
	initializePipoInputModule(pipoInputModule);

	registerExpanderModule(pipoInputModule, ExpanderData(pipoOutputModule, modelPipoOutput, ExpanderSide::LEFT));
	pipoInputModule.draw(widget::Widget::DrawArgs());

	expectConnected(pipoInputModule, false);
}

TEST(PipoInputTest, WithUnknownModuleBeforePipoOutputOnRightShouldDectivateLed) {
	Module module;
	PipoInputModule pipoInputModule;
	PipoOutputModule pipoOutputModule;
	initializePipoInputModule(pipoInputModule);

	registerExpanderModules(pipoInputModule, {
		ExpanderData(module, createModel<Module, DummyWidget>("dummy"), ExpanderSide::RIGHT),
		ExpanderData(pipoOutputModule, modelPipoOutput, ExpanderSide::RIGHT)
	});
	pipoInputModule.draw(widget::Widget::DrawArgs());

	expectConnected(pipoInputModule, false);
}

TEST(PipoInputTest, InChainOfInputModulesWithPipoOutputOnRightShouldActivateLed) {
	PipoInputModule pipoInputModule[3];
	PipoOutputModule pipoOutputModule;
	initializePipoInputModule(pipoInputModule[0]);

	registerExpanderModules(pipoInputModule[0], {
		ExpanderData(pipoInputModule[0], modelPipoInput, ExpanderSide::RIGHT),
		ExpanderData(pipoInputModule[1], modelPipoInput, ExpanderSide::RIGHT),
		ExpanderData(pipoOutputModule, modelPipoOutput, ExpanderSide::RIGHT)
	});
	pipoInputModule[0].draw(widget::Widget::DrawArgs());

	expectConnected(pipoInputModule[0], true);
}

TEST(PipoInputTest, InChainOfInputModulesWithNoPipoOutputOnRightShouldDeactivateLed) {
	PipoInputModule pipoInputModule[3];
	initializePipoInputModule(pipoInputModule[0]);

	registerExpanderModules(pipoInputModule[0], {
		ExpanderData(pipoInputModule[0], modelPipoInput, ExpanderSide::RIGHT),
		ExpanderData(pipoInputModule[1], modelPipoInput, ExpanderSide::RIGHT)
	});
	pipoInputModule[0].draw(widget::Widget::DrawArgs());

	expectConnected(pipoInputModule[0], false);
}

TEST(PipoInputTest, InChainOfInputModulesWithUnknownModuleBeforePipoOutputOnRightShouldDeactivateLed) {
	Module module;
	PipoInputModule pipoInputModule[3];
	PipoOutputModule pipoOutputModule;
	initializePipoInputModule(pipoInputModule[0]);

	registerExpanderModules(pipoInputModule[0], {
		ExpanderData(pipoInputModule[0], modelPipoInput, ExpanderSide::RIGHT),
		ExpanderData(pipoInputModule[1], modelPipoInput, ExpanderSide::RIGHT),
		ExpanderData(module, createModel<Module, DummyWidget>("dummy"), ExpanderSide::RIGHT),
		ExpanderData(pipoOutputModule, modelPipoOutput, ExpanderSide::RIGHT)
	});
	pipoInputModule[0].draw(widget::Widget::DrawArgs());

	expectConnected(pipoInputModule[0], false);
}

TEST(PipoInputTest, ShouldSetLEDDisplayToInputChannelCount) {
	PipoInputModule pipoInputModule;
	initializePipoInputModule(pipoInputModule);
	std::array<int, 8> channels = { 1, 3, 4, 6, 2, 10, 16, 8 };

	for (int i = 0; i < 8; i++) {
		pipoInputModule.inputs[i].channels = channels[i];
	}

	pipoInputModule.draw(widget::Widget::DrawArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(pipoInputModule.m_ledDisplays[i]->getForegroundText(), string::f("%d", channels[i]));
	}
}

TEST(PipoInputTest, ShouldSetLEDDisplayForNotConnectedInputToOneCHannel) {
	PipoInputModule pipoInputModule;
	initializePipoInputModule(pipoInputModule);

	for (int i = 0; i < 8; i++) {
		pipoInputModule.inputs[i].channels = 0;
	}

	pipoInputModule.draw(widget::Widget::DrawArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(pipoInputModule.m_ledDisplays[i]->getForegroundText(), "1");
	}
}
