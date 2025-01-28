#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "modules/solim-output.hpp"
#include "modules/solim.hpp"
#include "modules/solim-random.hpp"
#include "modules/solim-output-octaver.hpp"
#include "modules/solim-input.hpp"
#include "modules/solim-input-octaver.hpp"

#include "../common/vcv-registration.hpp"

struct DummyWidget : ModuleWidget {
	DummyWidget(Module* module) {}
};

void initializeSolimInputOctaverModule(SolimInputOctaverModule& solimInputOctaverModule) {
	for (int i = 0; i < SolimInputOctaverModule::LightIds::NUM_LIGHTS; i++) {
		solimInputOctaverModule.lights[i].setBrightness(-99.f);
	}
	for (int i = 0; i < SolimInputOctaverModule::InputsIds::NUM_INPUTS; i++) {
		solimInputOctaverModule.inputs[i].setChannels(0);
		solimInputOctaverModule.inputs[i].setVoltage(0.f);
	}
	for (int i = 0; i < SolimInputOctaverModule::ParamsIds::NUM_PARAMS; i++) {
		solimInputOctaverModule.params[i].setValue(0.f);
	}
}

void expectConnected(SolimInputOctaverModule& solimInputOctaverModule, bool connected) {
	if (connected) {
		EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_CONNECTED].getBrightness(), 1.f, 0.0001f);
		EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_NOT_CONNECTED].getBrightness(), 0.f, 0.0001f);
	} else {
		EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_CONNECTED].getBrightness(), 0.f, 0.0001f);
		EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_NOT_CONNECTED].getBrightness(), 1.f, 0.0001f);
	}
}

TEST(SolimInputOctaverTest, WithNoExpandersShouldDeactivateLed) {
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, false);
}

TEST(SolimInputOctaverTest, WithSolimExpanderOnRightShouldActivateLed) {
	SolimModule solimModule;
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModule(solimInputOctaverModule, ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT));
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, true);
}

TEST(SolimInputOctaverTest, WithSolimExpanderOnLeftShouldDeactivateLed) {
	SolimModule solimModule;
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModule(solimInputOctaverModule, ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT));
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, false);
}

TEST(SolimInputOctaverTest, WithUnknownExpanderBeforeSolimExpanderShouldDectivateLed) {
	Module module;
	SolimModule solimModule;
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModules(solimInputOctaverModule, {
		ExpanderData(module, createModel<Module, DummyWidget>("dummy"), ExpanderSide::RIGHT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
	});
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, false);
}

TEST(SolimInputOctaverTest, WithSolimOutputBeforeSolimExpanderShouldDectivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModules(solimInputOctaverModule, {
		ExpanderData(solimOutputModule, modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT),
	});
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, false);
}

TEST(SolimInputOctaverTest, WithSolimOutputOctaverBeforeSolimExpanderShouldDectivateLed) {
	SolimModule solimModule;
	SolimOutputOctaverModule solimOutputOctaverModule;
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModules(solimInputOctaverModule, {
		ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::RIGHT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
	});
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, false);
}

TEST(SolimInputOctaverTest, WithSingleInputAndSolimExpanderOnRightShouldActivateLed) {
	SolimModule solimModule;
	SolimInputModule solimInputExpanderModule;
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModules(solimInputOctaverModule, {
		ExpanderData(solimInputExpanderModule, modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
	});
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, true);
}

TEST(SolimInputOctaverTest, WithUpToSevenInputExpanderAndASolimExpanderOnRightShouldActivateLed) {
	SolimModule solimModule;
	SolimInputModule solimInputExpanderModule[7];
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	for (int i = 0; i < 7; i++) {
		if (i == 0) {
			registerExpanderModule(solimInputOctaverModule, ExpanderData(solimInputExpanderModule[i], modelSolimInput, ExpanderSide::RIGHT));
		} else {
			registerExpanderModule(solimInputExpanderModule[i - 1], ExpanderData(solimInputExpanderModule[i], modelSolimInput, ExpanderSide::RIGHT));
		}
		registerExpanderModule(solimInputExpanderModule[i], ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT));

		solimInputOctaverModule.draw(widget::Widget::DrawArgs());

		expectConnected(solimInputOctaverModule, true);
	}
}

TEST(SolimInputOctaverTest, WithEightInputExpandersAndASolimExpanderOnRightShouldDeactivateLed) {
	SolimModule solimModule;
	SolimInputModule solimInputExpanderModule[8];
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModules(solimInputOctaverModule, {
		ExpanderData(solimInputExpanderModule[0], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[1], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[2], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[3], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[4], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[5], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[6], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[7], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
	});
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, false);
}

TEST(SolimInputOctaverTest, WithSolimRandomAndSolimExpanderOnRightShouldActivateLed) {
	SolimModule solimModule;
	SolimRandomModule solimRandomModule;
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModules(solimInputOctaverModule, {
		ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::RIGHT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
	});
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, true);
}

TEST(SolimInputOctaverTest, WithSolimRandomAndEightInputExpandersAndASolimExpanderOnRightShouldDeactivateLed) {
	SolimModule solimModule;
	SolimRandomModule solimRandomModule;
	SolimInputModule solimInputExpanderModule[8];
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModules(solimInputOctaverModule, {
		ExpanderData(solimInputExpanderModule[0], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[1], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[2], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[3], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[4], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[5], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[6], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimInputExpanderModule[7], modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
	});
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, false);
}

TEST(SolimInputOctaverTest, WithTwoSolimRandomAndSolimExpanderOnRightShouldDeactivateLed) {
	SolimModule solimModule;
	SolimRandomModule solimRandomModule[2];
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModules(solimInputOctaverModule, {
		ExpanderData(solimRandomModule[0], modelSolimRandom, ExpanderSide::RIGHT),
		ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::RIGHT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
	});
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, false);
}

TEST(SolimInputOctaverTest, WithAnotherInputOctaverBeforSolimShouldDeactivateLed) {
	SolimModule solimModule;
	SolimInputOctaverModule solimInputOctaverModule;
	SolimInputOctaverModule solimInputOctaverModuleExpander;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModules(solimInputOctaverModule, {
		ExpanderData(solimInputOctaverModuleExpander, modelSolimInputOctaver, ExpanderSide::RIGHT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
	});
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, false);
}

TEST(SolimInputOctaverTest, WithSolimInputExpanderAndSolimRandomAndSolimOnRightShouldActivateLed) {
	SolimModule solimModule;
	SolimInputModule solimInputModule;
	SolimRandomModule solimRandomModule;
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModules(solimInputOctaverModule, {
		ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::RIGHT),
		ExpanderData(solimInputModule, modelSolimInput, ExpanderSide::RIGHT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
	});
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, true);
}

TEST(SolimInputOctaverTest, WithSolimInputExpanderAndSolimRandomAndNoSolimOnRightShouldDeactivateLed) {
	SolimInputModule solimInputModule;
	SolimRandomModule solimRandomModule;
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimInputOctaverModule(solimInputOctaverModule);

	registerExpanderModules(solimInputOctaverModule, {
		ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::RIGHT),
		ExpanderData(solimInputModule, modelSolimInput, ExpanderSide::RIGHT),
	});
	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimInputOctaverModule, false);
}

TEST(SolimInputOctaverTest, ShouldLightReplaceOriginalLedsBasedOnParams) {
	SolimInputOctaverModule solimInputOctaverModule;

	// Enable buttons 1, 3, 5 and 7
	solimInputOctaverModule.params[SolimInputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 1].setValue(1.0f);
	solimInputOctaverModule.params[SolimInputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 3].setValue(1.0f);
	solimInputOctaverModule.params[SolimInputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 5].setValue(1.0f);
	solimInputOctaverModule.params[SolimInputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 7].setValue(1.0f);

	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	for (int i = 0; i < 8; i += 2) {
		// Even lights should be turned off, uneven ones turned on
		EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + i].getBrightness(), 0.f, 0.0001f);
		EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + i + 1].getBrightness(), 1.f, 0.0001f);
	}
}

TEST(SolimInputOctaverTest, ShouldLightReplaceOriginalLedsBasedOnInputs) {
	SolimInputOctaverModule solimInputOctaverModule;

	// Connnect inputs 2, 4, 6, and set voltage for 2 and 6 to on
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 2].channels = 1;
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 2].setVoltage(1.f);
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 4].channels = 1;
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 4].setVoltage(0.f);
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 6].channels = 1;
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 6].setVoltage(1.f);

	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	for (int i = 0; i < 8; i++) {
		if (i == 2 || i == 6) {
			EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + i].getBrightness(), 1.f, 0.0001f);
		} else {
			EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + i].getBrightness(), 0.f, 0.0001f);
		}
	}
}

TEST(SolimInputOctaverTest, ShouldOnlyUseParamForLightReplaceOriginalIfInputIsNotConnected) {
	SolimInputOctaverModule solimInputOctaverModule;

	// The buttons for 1, 2, 5 and 6 are turned on.
	solimInputOctaverModule.params[SolimInputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 1].setValue(1.0f);
	solimInputOctaverModule.params[SolimInputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 2].setValue(1.0f);
	solimInputOctaverModule.params[SolimInputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 4].setValue(1.0f);
	solimInputOctaverModule.params[SolimInputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 6].setValue(1.0f);

	// Connnect inputs 2, 4, 6, and set voltage for 2 and 6 to on
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 2].channels = 1;
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 2].setVoltage(1.f);
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 4].channels = 1;
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 4].setVoltage(0.f);
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 6].channels = 1;
	solimInputOctaverModule.inputs[SolimInputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 6].setVoltage(1.f);

	solimInputOctaverModule.draw(widget::Widget::DrawArgs());

	EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 0].getBrightness(), 0.f, 0.0001f);
	EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 1].getBrightness(), 1.f, 0.0001f);
	EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 2].getBrightness(), 1.f, 0.0001f);
	EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 3].getBrightness(), 0.f, 0.0001f);
	EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 4].getBrightness(), 0.f, 0.0001f);
	EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 5].getBrightness(), 0.f, 0.0001f);
	EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 6].getBrightness(), 1.f, 0.0001f);
	EXPECT_NEAR(solimInputOctaverModule.lights[SolimInputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 7].getBrightness(), 0.f, 0.0001f);
}
