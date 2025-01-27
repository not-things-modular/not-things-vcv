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

void initializeSolimOutputOctaverModule(SolimOutputOctaverModule& solimOutputOctaverModule) {
    for (int i = 0; i < SolimOutputOctaverModule::LightIds::NUM_LIGHTS; i++) {
        solimOutputOctaverModule.lights[i].setBrightness(-99.f);
    }
}

void expectConnected(SolimOutputOctaverModule& solimOutputOctaverModule, bool connected) {
    if (connected) {
        EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_CONNECTED].getBrightness(), 1.f, 0.0001f);
        EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_NOT_CONNECTED].getBrightness(), 0.f, 0.0001f);
    } else {
        EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_CONNECTED].getBrightness(), 0.f, 0.0001f);
        EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_NOT_CONNECTED].getBrightness(), 1.f, 0.0001f);
    }
}

TEST(SolimOutputOctaverTest, WithNoExpandersShouldDeactivateLed) {
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, false);
}

TEST(SolimOutputOctaverTest, WithSolimExpanderOnLeftShouldActivateLed) {
    SolimModule solimModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModule(solimOutputOctaverModule, ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT));
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, true);
}

TEST(SolimOutputOctaverTest, WithSolimExpanderOnRightShouldDeactivateLed) {
    SolimModule solimModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModule(solimOutputOctaverModule, ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT));
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, false);
}

TEST(SolimOutputOctaverTest, WithUnknownExpanderBeforeSolimExpanderShouldDectivateLed) {
    Module module;
    SolimModule solimModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModules(solimOutputOctaverModule, {
        ExpanderData(module, createModel<Module, DummyWidget>("dummy"), ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, false);
}

TEST(SolimOutputOctaverTest, WithSolimInputBeforeSolimExpanderShouldDectivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModules(solimOutputOctaverModule, {
        ExpanderData(solimInputModule, modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT),
    });
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, false);
}

TEST(SolimOutputOctaverTest, WithSolimInputOctaverBeforeSolimExpanderShouldDectivateLed) {
    SolimModule solimModule;
    SolimInputOctaverModule solimInputOctaverModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModules(solimOutputOctaverModule, {
        ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, false);
}

TEST(SolimOutputOctaverTest, WithSingleOutputAndSolimExpanderOnLeftShouldActivateLed) {
    SolimModule solimModule;
    SolimOutputModule solimOutputExpanderModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModules(solimOutputOctaverModule, {
        ExpanderData(solimOutputExpanderModule, modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, true);
}

TEST(SolimOutputOctaverTest, WithUpToSevenOutputExpanderAndASolimExpanderOnLeftShouldActivateLed) {
    SolimModule solimModule;
    SolimOutputModule solimOutputExpanderModule[7];
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    for (int i = 0; i < 7; i++) {
        if (i == 0) {
            registerExpanderModule(solimOutputOctaverModule, ExpanderData(solimOutputExpanderModule[i], modelSolimOutput, ExpanderSide::LEFT));
        } else {
            registerExpanderModule(solimOutputExpanderModule[i - 1], ExpanderData(solimOutputExpanderModule[i], modelSolimOutput, ExpanderSide::LEFT));
        }
        registerExpanderModule(solimOutputExpanderModule[i], ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT));

        solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

        expectConnected(solimOutputOctaverModule, true);
    }
}

TEST(SolimOutputOctaverTest, WithEightOutputExpandersAndASolimExpanderOnLeftShouldDeactivateLed) {
    SolimModule solimModule;
    SolimOutputModule solimOutputExpanderModule[8];
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModules(solimOutputOctaverModule, {
        ExpanderData(solimOutputExpanderModule[0], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[1], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[2], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[3], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[4], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[5], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[6], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[7], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, false);
}

TEST(SolimOutputOctaverTest, WithSolimRandomAndSolimExpanderOnLeftShouldActivateLed) {
    SolimModule solimModule;
    SolimRandomModule solimRandomModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModules(solimOutputOctaverModule, {
        ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, true);
}

TEST(SolimOutputOctaverTest, WithSolimRandomAndEightOutputExpandersAndASolimExpanderOnLeftShouldDeactivateLed) {
    SolimModule solimModule;
    SolimRandomModule solimRandomModule;
    SolimOutputModule solimOutputExpanderModule[8];
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModules(solimOutputOctaverModule, {
        ExpanderData(solimOutputExpanderModule[0], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[1], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[2], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[3], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[4], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[5], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[6], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimOutputExpanderModule[7], modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, false);
}

TEST(SolimOutputOctaverTest, WithTwoSolimRandomAndSolimExpanderOnLeftShouldDeactivateLed) {
    SolimModule solimModule;
    SolimRandomModule solimRandomModule[2];
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModules(solimOutputOctaverModule, {
        ExpanderData(solimRandomModule[0], modelSolimRandom, ExpanderSide::LEFT),
        ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, false);
}

TEST(SolimOutputOctaverTest, WithAnotherOutputOctaverBeforSolimShouldDeactivateLed) {
    SolimModule solimModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    SolimOutputOctaverModule solimOutputOctaverModuleExpander;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModules(solimOutputOctaverModule, {
        ExpanderData(solimOutputOctaverModuleExpander, modelSolimOutputOctaver, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, false);
}

TEST(SolimOutputOctaverTest, WithSolimOutputExpanderAndSolimRandomAndSolimOnLeftShouldActivateLed) {
    SolimModule solimModule;
    SolimOutputModule solimOutputModule;
    SolimRandomModule solimRandomModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModules(solimOutputOctaverModule, {
        ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT),
        ExpanderData(solimOutputModule, modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, true);
}

TEST(SolimOutputOctaverTest, WithSolimOutputExpanderAndSolimRandomAndNoSolimOnLeftShouldDeactivateLed) {
    SolimOutputModule solimOutputModule;
    SolimRandomModule solimRandomModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimOutputOctaverModule(solimOutputOctaverModule);

    registerExpanderModules(solimOutputOctaverModule, {
        ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT),
        ExpanderData(solimOutputModule, modelSolimOutput, ExpanderSide::LEFT),
    });
    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimOutputOctaverModule, false);
}

TEST(SolimOutputOctaverTest, ShouldLightReplaceOriginalLedsBasedOnParams) {
    SolimOutputOctaverModule solimOutputOctaverModule;

    // Enable buttons 1, 3, 5 and 7
    solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 1].setValue(1.0f);
    solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 3].setValue(1.0f);
    solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 5].setValue(1.0f);
    solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 7].setValue(1.0f);

    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    for (int i = 0; i < 8; i += 2) {
        // Even lights should be turned off, uneven ones turned on
        EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + i].getBrightness(), 0.f, 0.0001f);
        EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + i + 1].getBrightness(), 1.f, 0.0001f);
    }
}

TEST(SolimOutputOctaverTest, ShouldLightReplaceOriginalLedsBasedOnOutputs) {
    SolimOutputOctaverModule solimOutputOctaverModule;

    // Connnect Outputs 2, 4, 6, and set voltage for 2 and 6 to on
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 2].channels = 1;
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 2].setVoltage(1.f);
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 4].channels = 1;
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 4].setVoltage(0.f);
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 6].channels = 1;
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 6].setVoltage(1.f);

    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    for (int i = 0; i < 8; i++) {
        if (i == 2 || i == 6) {
            EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + i].getBrightness(), 1.f, 0.0001f);
        } else {
            EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + i].getBrightness(), 0.f, 0.0001f);
        }
    }
}

TEST(SolimOutputOctaverTest, ShouldOnlyUseParamForLightReplaceOriginalIfOutputIsNotConnected) {
    SolimOutputOctaverModule solimOutputOctaverModule;

    // The buttons for 1, 2, 5 and 6 are turned on.
    solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 1].setValue(1.0f);
    solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 2].setValue(1.0f);
    solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 4].setValue(1.0f);
    solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamsIds::PARAM_REPLACE_ORIGINAL + 6].setValue(1.0f);

    // Connnect Outputs 2, 4, 6, and set voltage for 2 and 6 to on
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 2].channels = 1;
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 2].setVoltage(1.f);
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 4].channels = 1;
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 4].setVoltage(0.f);
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 6].channels = 1;
    solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputsIds::IN_REPLACE_ORIGINAL + 6].setVoltage(1.f);

    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 0].getBrightness(), 0.f, 0.0001f);
    EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 1].getBrightness(), 1.f, 0.0001f);
    EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 2].getBrightness(), 1.f, 0.0001f);
    EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 3].getBrightness(), 0.f, 0.0001f);
    EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 4].getBrightness(), 0.f, 0.0001f);
    EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 5].getBrightness(), 0.f, 0.0001f);
    EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 6].getBrightness(), 1.f, 0.0001f);
    EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_REPLACE_ORIGINAL + 7].getBrightness(), 0.f, 0.0001f);
}

TEST(SolimOutputOctaverTest, ShouldLightUpResortWhenPressed) {
    SolimOutputOctaverModule solimOutputOctaverModule;

    solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamsIds::PARAM_RESORT].setValue(1.f);

    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_RESORT].getBrightness(), 1.f, 0.0001f);
}

TEST(SolimOutputOctaverTest, ShouldNotLightUpResortWhenNotPressed) {
    SolimOutputOctaverModule solimOutputOctaverModule;

    solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamsIds::PARAM_RESORT].setValue(0.f);

    solimOutputOctaverModule.draw(widget::Widget::DrawArgs());

    EXPECT_NEAR(solimOutputOctaverModule.lights[SolimOutputOctaverModule::LightIds::LIGHT_RESORT].getBrightness(), 0.f, 0.0001f);
}
