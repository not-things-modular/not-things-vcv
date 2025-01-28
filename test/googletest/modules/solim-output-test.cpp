#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "modules/solim-output.hpp"
#include "modules/solim.hpp"
#include "modules/solim-random.hpp"
#include "modules/solim-output-octaver.hpp"
#include "modules/solim-input.hpp"
#include "modules/solim-input-octaver.hpp"

#include "../common/vcv-registration.hpp"
#include "../common/save-data.hpp"

struct DummyWidget : ModuleWidget {
	DummyWidget(Module* module) {}
};

void initializeSolimOutputModule(SolimOutputModule& solimOutputModule) {
	for (int i = 0; i < SolimOutputModule::LightIds::NUM_LIGHTS; i++) {
		solimOutputModule.lights[i].setBrightness(-99.f);
	}
}

void expectConnected(SolimOutputModule& solimOutputModule, bool connected) {
	if (connected) {
		EXPECT_NEAR(solimOutputModule.lights[SolimOutputModule::LightIds::LIGHT_CONNECTED].getBrightness(), 1.f, 0.0001f);
		EXPECT_NEAR(solimOutputModule.lights[SolimOutputModule::LightIds::LIGHT_NOT_CONNECTED].getBrightness(), 0.f, 0.0001f);
		for (int i = SolimOutputModule::LightIds::OUT_LIGHTS; i < SolimOutputModule::LightIds::OUT_LIGHTS + 8; i++) {
			EXPECT_NEAR(solimOutputModule.lights[i].getBrightness(), -99.f, 0.0001f);
		}
	} else {
		EXPECT_NEAR(solimOutputModule.lights[SolimOutputModule::LightIds::LIGHT_CONNECTED].getBrightness(), 0.f, 0.0001f);
		EXPECT_NEAR(solimOutputModule.lights[SolimOutputModule::LightIds::LIGHT_NOT_CONNECTED].getBrightness(), 1.f, 0.0001f);
		for (int i = SolimOutputModule::LightIds::OUT_LIGHTS; i < SolimOutputModule::LightIds::OUT_LIGHTS + 8; i++) {
			EXPECT_NEAR(solimOutputModule.lights[i].getBrightness(), 0.f, 0.0001f);
		}
	}
}

TEST(SolimOutputTest, WithNoExpandersShouldDeactivateLed) {
	SolimOutputModule solimOutputModule;
	initializeSolimOutputModule(solimOutputModule);

	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, false);
}

TEST(SolimOutputTest, WithSolimExpanderOnLeftShouldActivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModule(solimOutputModule, ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT));
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, true);
}

TEST(SolimOutputTest, WithSolimExpanderOnRightShouldDeactivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModule(solimOutputModule, ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT));
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, false);
}

TEST(SolimOutputTest, WithUnknownExpanderBeforeSolimExpanderShouldDectivateLed) {
	Module module;
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(module, createModel<Module, DummyWidget>("dummy"), ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, false);
}

TEST(SolimOutputTest, WithSolimInputBeforeSolimExpanderShouldDectivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimInputModule solimInputModule;
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimInputModule, modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT),
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, false);
}

TEST(SolimOutputTest, WithSolimInputOctaverBeforeSolimExpanderShouldDectivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, false);
}

TEST(SolimOutputTest, WithSingleOutputAndSolimExpanderOnLeftShouldActivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimOutputModule solimOutputExpanderModule;
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimOutputExpanderModule, modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, true);
}

TEST(SolimOutputTest, WithUpToSixOutputExpanderAndASolimExpanderOnLeftShouldActivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimOutputModule solimOutputExpanderModule[6];
	initializeSolimOutputModule(solimOutputModule);

	for (int i = 0; i < 6; i++) {
		if (i == 0) {
			registerExpanderModule(solimOutputModule, ExpanderData(solimOutputExpanderModule[i], modelSolimOutput, ExpanderSide::LEFT));
		} else {
			registerExpanderModule(solimOutputExpanderModule[i - 1], ExpanderData(solimOutputExpanderModule[i], modelSolimOutput, ExpanderSide::LEFT));
		}
		registerExpanderModule(solimOutputExpanderModule[i], ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT));

		solimOutputModule.draw(widget::Widget::DrawArgs());

		expectConnected(solimOutputModule, true);
	}
}

TEST(SolimOutputTest, WithSevenOutputExpandersAndASolimExpanderOnLeftShouldDeactivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimOutputModule solimOutputExpanderModule[7];
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimOutputExpanderModule[0], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[1], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[2], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[3], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[4], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[5], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[6], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, false);
}

TEST(SolimOutputTest, WithSolimRandomAndSolimExpanderOnLeftShouldActivateLed) {
	SolimModule solimModule;
	SolimRandomModule solimRandomModule;
	SolimOutputModule solimOutputModule;
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, true);
}

TEST(SolimOutputTest, WithSolimRandomAndSevenOutputExpandersAndASolimExpanderOnLeftShouldDeactivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimRandomModule solimRandomModule;
	SolimOutputModule solimOutputExpanderModule[7];
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimOutputExpanderModule[0], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[1], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[2], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[3], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[4], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[5], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[6], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, false);
}

TEST(SolimOutputTest, WithTwoSolimRandomAndSolimExpanderOnLeftShouldDeactivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimRandomModule solimRandomModule[2];
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimRandomModule[0], modelSolimRandom, ExpanderSide::LEFT),
		ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, false);
}

TEST(SolimOutputTest, WithSolimOutputOctaverAndSolimExpanderOnLeftShouldActivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimOutputOctaverModule solimOutputOctaverModule;
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, true);
}

TEST(SolimOutputTest, WithSolimOutputOctaverAndSevenOutputExpandersAndASolimExpanderOnLeftShouldDeactivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimOutputModule solimOutputExpanderModule[7];
	SolimOutputOctaverModule solimOutputOctaverModule;
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimOutputExpanderModule[0], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[1], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[2], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[3], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[4], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[5], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule[6], modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, false);
}

TEST(SolimOutputTest, WithTwoSolimOutputOctaverAndSolimExpanderOnLeftShouldDeactivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimOutputOctaverModule solimOutputOctaverModule[2];
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimOutputOctaverModule[0], modelSolimOutputOctaver, ExpanderSide::LEFT),
		ExpanderData(solimOutputOctaverModule[1], modelSolimOutputOctaver, ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, false);
}

TEST(SolimOutputTest, WithSolimOutputExpanderAndSolimRandomAndSolimOutputOctaverAndSolimOnLeftShouldActivateLed) {
	SolimModule solimModule;
	SolimOutputModule solimOutputModule;
	SolimRandomModule solimRandomModule;
	SolimOutputModule solimOutputExpanderModule;
	SolimOutputOctaverModule solimOutputOctaverModule;
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule, modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::LEFT),
		ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, true);
}

TEST(SolimOutputTest, WithSolimOutputExpanderAndSolimRandomAndSolimOutputOctaverButNoSolimOnLeftShouldDeactivateLed) {
	SolimOutputModule solimOutputModule;
	SolimRandomModule solimRandomModule;
	SolimOutputModule solimOutputExpanderModule;
	SolimOutputOctaverModule solimOutputOctaverModule;
	initializeSolimOutputModule(solimOutputModule);

	registerExpanderModules(solimOutputModule, {
		ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT),
		ExpanderData(solimOutputExpanderModule, modelSolimOutput, ExpanderSide::LEFT),
		ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::LEFT)
	});
	solimOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(solimOutputModule, false);
}


TEST(SolimOutputTest, ShouldDefaultToMonophonicOutputMode) {
	SolimOutputModule solimOutputModule;

	EXPECT_EQ(solimOutputModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);
	json_t* jsonData = solimOutputModule.dataToJson();
	checkJsonSolimOutputMode(solimOutputModule.dataToJson(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);

	delete jsonData;
}

TEST(SolimOutputTest, ShouldSwitchToPolyphonicModeFromJsonData) {
	SolimOutputModule solimOutputModule;

	json_t *jsonData = json_object();
	json_object_set_new(jsonData, "ntSolimOutputMode", json_integer(static_cast<SolimOutputMode>(SolimOutputMode::OUTPUT_MODE_POLYPHONIC)));
	solimOutputModule.dataFromJson(jsonData);

	EXPECT_EQ(solimOutputModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_POLYPHONIC);

	delete jsonData;
}

TEST(SolimOutputTest, ShouldSwitchToMonophonicModeFromJsonData) {
	SolimOutputModule solimOutputModule;

	json_t *jsonData = json_object();
	json_object_set_new(jsonData, "ntSolimOutputMode", json_integer(static_cast<SolimOutputMode>(SolimOutputMode::OUTPUT_MODE_MONOPHONIC)));
	solimOutputModule.dataFromJson(jsonData);
	EXPECT_EQ(solimOutputModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);

	solimOutputModule.setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);

	solimOutputModule.dataFromJson(jsonData);
	EXPECT_EQ(solimOutputModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);

	delete jsonData;
}

TEST(SolimOutputTest, ShouldPersistPolyphonicOutputMode) {
	SolimOutputModule solimOutputModule;

	solimOutputModule.setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);
	EXPECT_EQ(solimOutputModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_POLYPHONIC);
	json_t* jsonData = solimOutputModule.dataToJson();
	checkJsonSolimOutputMode(solimOutputModule.dataToJson(), SolimOutputMode::OUTPUT_MODE_POLYPHONIC);

	delete jsonData;
}

TEST(SolimOutputTest, ShouldPersistMonophonicOutputMode) {
	SolimOutputModule solimOutputModule;

	solimOutputModule.setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);
	solimOutputModule.setOutputMode(SolimOutputMode::OUTPUT_MODE_MONOPHONIC);
	EXPECT_EQ(solimOutputModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);
	json_t* jsonData = solimOutputModule.dataToJson();
	checkJsonSolimOutputMode(solimOutputModule.dataToJson(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);

	delete jsonData;
}
