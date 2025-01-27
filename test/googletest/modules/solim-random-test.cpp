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

void initializeSolimRandomModule(SolimRandomModule& solimRandomModule) {
    for (int i = 0; i < SolimRandomModule::LightsIds::NUM_LIGHTS; i++) {
        solimRandomModule.lights[i].setBrightness(-99.f);
    }
    for (int i = 0; i < SolimRandomModule::InputsIds::NUM_INPUTS; i++) {
        solimRandomModule.inputs[i].setChannels(0);
        solimRandomModule.inputs[i].setVoltage(0.f);
    }
    for (int i = 0; i < SolimRandomModule::ParamsIds::NUM_PARAMS; i++) {
        solimRandomModule.params[i].setValue(0.f);
    }
}

void expectConnected(SolimRandomModule& solimRandomModule, bool connectedLeft, bool connectedRight) {
    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_CONNECTED_LEFT].getBrightness(), connectedLeft ? 1.f : 0.f, 0.0001f);
    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_CONNECTED_RIGHT].getBrightness(), connectedRight ? 1.f : 0.f, 0.0001f);
}

TEST(SolimRandomTest, WithNoExpandersShouldDeactivateLed) {
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, false);
}

TEST(SolimRandomTest, WithSolimExpanderOnRightShouldActivateRightLed) {
    SolimModule solimModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModule(solimRandomModule, ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT));
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, true);
}

TEST(SolimRandomTest, WithSolimExpanderOnLeftShouldActivateLeftLed) {
    SolimModule solimModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModule(solimRandomModule, ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT));
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, true, false);
}

TEST(SolimRandomTest, WithSolimExpanderOnBothSidesShouldActivateBothLed) {
    SolimModule solimModule[2];
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModule(solimRandomModule, ExpanderData(solimModule[0], modelSolim, ExpanderSide::LEFT));
    registerExpanderModule(solimRandomModule, ExpanderData(solimModule[1], modelSolim, ExpanderSide::RIGHT));
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, true, true);
}

TEST(SolimRandomTest, WithUnknownExpanderBeforeRightSolimExpanderShouldDectivateLed) {
    Module module;
    SolimModule solimModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
        ExpanderData(module, createModel<Module, DummyWidget>("dummy"), ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, false);
}

TEST(SolimRandomTest, WithUnknownExpanderBeforeLeftSolimExpanderShouldDectivateLed) {
    Module module;
    SolimModule solimModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
        ExpanderData(module, createModel<Module, DummyWidget>("dummy"), ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, false);
}

TEST(SolimRandomTest, WithSolimOutputOnRightBeforeSolimExpanderShouldDectivateLed) {
    SolimModule solimModule;
    SolimOutputModule solimOutputModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
        ExpanderData(solimOutputModule, modelSolimOutput, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT),
    });
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, false);
}

TEST(SolimRandomTest, WithSolimInputOnLeftBeforeSolimExpanderShouldDectivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
        ExpanderData(solimInputModule, modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT),
    });
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, false);
}

TEST(SolimRandomTest, WithSolimOutputOctaverOnRightBeforeSolimExpanderShouldDectivateLed) {
    SolimModule solimModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
        ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, false);
}

TEST(SolimRandomTest, WithSolimOutputOctaverOnLeftBeforeSolimExpanderShouldDectivateLed) {
    SolimModule solimModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
        ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, true, false);
}

TEST(SolimRandomTest, WithSolimInputOctaverOnLeftBeforeSolimExpanderShouldDectivateLed) {
    SolimModule solimModule;
    SolimInputOctaverModule solimInputOctaverModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
        ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, false);
}

TEST(SolimRandomTest, WithSolimInputOctaverOnRightBeforeSolimExpanderShouldActivateLed) {
    SolimModule solimModule;
    SolimInputOctaverModule solimInputOctaverModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
        ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, true);
}

TEST(SolimRandomTest, WithSingleInputAndSolimExpanderOnRightShouldActivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputExpanderModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
        ExpanderData(solimInputExpanderModule, modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, true);
}

TEST(SolimRandomTest, WithSingleOutputAndSolimExpanderOnLeftShouldActivateLed) {
    SolimModule solimModule;
    SolimOutputModule solimOutputExpanderModule;
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
        ExpanderData(solimOutputExpanderModule, modelSolimOutput, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, true, false);
}

TEST(SolimRandomTest, WithUpToSevenInputExpanderAndASolimExpanderOnRightShouldActivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputExpanderModule[7];
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    for (int i = 0; i < 7; i++) {
        if (i == 0) {
            registerExpanderModule(solimRandomModule, ExpanderData(solimInputExpanderModule[i], modelSolimInput, ExpanderSide::RIGHT));
        } else {
            registerExpanderModule(solimInputExpanderModule[i - 1], ExpanderData(solimInputExpanderModule[i], modelSolimInput, ExpanderSide::RIGHT));
        }
        registerExpanderModule(solimInputExpanderModule[i], ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT));

        solimRandomModule.draw(widget::Widget::DrawArgs());

        expectConnected(solimRandomModule, false, true);
    }
}

TEST(SolimRandomTest, WithUpToSevenOutputExpanderAndASolimExpanderOnLeftShouldActivateLed) {
    SolimModule solimModule;
    SolimOutputModule solimOutputExpanderModule[7];
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    for (int i = 0; i < 7; i++) {
        if (i == 0) {
            registerExpanderModule(solimRandomModule, ExpanderData(solimOutputExpanderModule[i], modelSolimOutput, ExpanderSide::LEFT));
        } else {
            registerExpanderModule(solimOutputExpanderModule[i - 1], ExpanderData(solimOutputExpanderModule[i], modelSolimOutput, ExpanderSide::LEFT));
        }
        registerExpanderModule(solimOutputExpanderModule[i], ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT));

        solimRandomModule.draw(widget::Widget::DrawArgs());

        expectConnected(solimRandomModule, true, false);
    }
}

TEST(SolimRandomTest, WithEightInputExpandersAndASolimExpanderOnRightShouldDeactivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputExpanderModule[8];
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
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
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, false);
}

TEST(SolimRandomTest, WithEightOutputExpandersAndASolimExpanderOnLeftShouldDeactivateLed) {
    SolimModule solimModule;
    SolimOutputModule solimOutputExpanderModule[8];
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    registerExpanderModules(solimRandomModule, {
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
    solimRandomModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule, false, false);
}

TEST(SolimRandomTest, WithAnotherRandomModuleOnRightShouldDeactivateLed) {
    SolimModule solimModule;
    SolimRandomModule solimRandomModule[2];
    initializeSolimRandomModule(solimRandomModule[0]);

    registerExpanderModules(solimRandomModule[0], {
        ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimRandomModule[0].draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule[0], false, false);
}

TEST(SolimRandomTest, WithAnotherRandomModuleOnLeftShouldDeactivateLed) {
    SolimModule solimModule;
    SolimRandomModule solimRandomModule[2];
    initializeSolimRandomModule(solimRandomModule[0]);

    registerExpanderModules(solimRandomModule[0], {
        ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::LEFT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT)
    });
    solimRandomModule[0].draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule[0], false, false);
}

TEST(SolimRandomTest, WithSolimExpanderOnRightAndOtherRandomBeyondThatShouldPrioratizeThisLeftRandomAndActivateRightLed) {
    SolimModule solimModule;
    SolimRandomModule solimRandomModule[2];
    initializeSolimRandomModule(solimRandomModule[0]);

    registerExpanderModules(solimRandomModule[0], {
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT),
        ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::RIGHT)
    });
    solimRandomModule[0].draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule[0], false, true);
}

TEST(SolimRandomTest, WithSolimExpanderOnLeftAndOtherRandomBeyondThatShouldPrioratizeOtherLeftRandomAndDeactivateLeftLed) {
    SolimModule solimModule;
    SolimRandomModule solimRandomModule[2];
    initializeSolimRandomModule(solimRandomModule[0]);

    registerExpanderModules(solimRandomModule[0], {
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT),
        ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::LEFT)
    });
    solimRandomModule[0].draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule[0], false, false);
}

TEST(SolimRandomTest, WithSolimExpanderOnLeftAndOtherRandomBeyondThatWhichIsNotConnectedShouldActivateLeftLed) {
    SolimModule solimModule;
    SolimOutputModule solimOutputModule;
    SolimRandomModule solimRandomModule[2];
    initializeSolimRandomModule(solimRandomModule[0]);

    registerExpanderModules(solimRandomModule[0], {
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT),
        ExpanderData(solimOutputModule, modelSolimOutput, ExpanderSide::LEFT), // Inserting an output module between the main solim and the other random breaks their chain
        ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::LEFT)
    });
    solimRandomModule[0].draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule[0], true, false);
}

TEST(SolimRandomTest, WithSolimExpanderOnLeftAndOtherRandomBeyondThatWithUpToSevenInputsAndOneInputOctaverShouldDectivateLeftLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule[7];
    SolimRandomModule solimRandomModule[2];
    SolimInputOctaverModule solimInputOctaverModule;
    initializeSolimRandomModule(solimRandomModule[0]);

    registerExpanderModules(solimRandomModule[0], {
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[3], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[4], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[5], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[6], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::LEFT)
    });
    solimRandomModule[0].draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule[0], false, false);
}

TEST(SolimRandomTest, WithSolimExpanderOnLeftAndOtherRandomBeyondThatWithMoreThenSevenInputsShouldConsiderItDisconnectedAndActivateLeftLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule[8];
    SolimRandomModule solimRandomModule[2];
    initializeSolimRandomModule(solimRandomModule[0]);

    registerExpanderModules(solimRandomModule[0], {
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[3], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[4], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[5], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[6], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimInputModule[7], modelSolimInput, ExpanderSide::LEFT),
        ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::LEFT)
    });
    solimRandomModule[0].draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule[0], true, false);
}

TEST(SolimRandomTest, WithSolimExpanderOnLeftAndOtherRandomBeyondThatWithMoreThenOneInputOctaverInBetweenShouldActivateLeftLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule[7];
    SolimRandomModule solimRandomModule[2];
    SolimInputOctaverModule solimInputOctaverModule[2];
    initializeSolimRandomModule(solimRandomModule[0]);

    registerExpanderModules(solimRandomModule[0], {
        ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT),
        ExpanderData(solimInputOctaverModule[0], modelSolimInputOctaver, ExpanderSide::LEFT),
        ExpanderData(solimInputOctaverModule[1], modelSolimInputOctaver, ExpanderSide::LEFT),
        ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::LEFT)
    });
    solimRandomModule[0].draw(widget::Widget::DrawArgs());

    expectConnected(solimRandomModule[0], true, false);
}

void expectNotTriggered(SolimRandomModule& solimRandomModule, bool checkMove, bool checkOne, bool checkAll, bool checkReset) {
    for (int i = 0; i < 8; i++) {
        if (checkMove) {
            EXPECT_EQ(solimRandomModule.m_moveCounters[i], 0);
            EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_MOVE].getBrightness(), 0.f, 0.000001f);
        }
        if (checkOne) {
            EXPECT_EQ(solimRandomModule.m_oneCounters[i], 0);
            EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ONE].getBrightness(), 0.f, 0.000001f);
        }
        if (checkAll) {
            EXPECT_EQ(solimRandomModule.m_allCounters[i], 0);
            EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ALL].getBrightness(), 0.f, 0.000001f);
        }
        if (checkReset) {
            EXPECT_EQ(solimRandomModule.m_resetCounters[i], 0);
            EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_RESET].getBrightness(), 0.f, 0.000001f);
        }
    }
}

TEST(SolimRandomTest, WithNoTriggersShouldNotLightTriggerLeds) {
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    solimRandomModule.process(Module::ProcessArgs()); // Initialize the triggers
    solimRandomModule.process(Module::ProcessArgs()); // The actual cycle we want to test

    expectNotTriggered(solimRandomModule, true, true, true, true);
}

TEST(SolimRandomTest, ShouldHandleMoveTrigger) {
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    for (int i = 0; i < 8; i++) {
        solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_MOVE].channels = 8;
    }

    // Initialize the triggers
    solimRandomModule.process(Module::ProcessArgs());

    // Now press the button
    solimRandomModule.params[SolimRandomModule::ParamsIds::PARAM_TRIG_MOVE].setValue(1.f);
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, false, true, true, true);

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(solimRandomModule.m_moveCounters[i], 1);
        EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_MOVE].getBrightness(), 1.f, 0.000001f);
    }

    // Release the button
    solimRandomModule.params[SolimRandomModule::ParamsIds::PARAM_TRIG_MOVE].setValue(0.f);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_MOVE].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, false, true, true, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_MOVE].getBrightness(), 0.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(solimRandomModule.m_moveCounters[i], 1);
    }

    // Trigger through the input
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_MOVE].setVoltage(1.f, 3);
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, false, true, true, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_MOVE].getBrightness(), 1.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if (i != 3) {
            EXPECT_EQ(solimRandomModule.m_moveCounters[i], 1);
        } else { // Only the fourth channel should have been triggered
            EXPECT_EQ(solimRandomModule.m_moveCounters[i], 2);
        }
    }

    // Remove the input trigger again, and trigger another
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_MOVE].setVoltage(0.f, 3);
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_MOVE].setVoltage(1.f, 4);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_MOVE].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, false, true, true, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_MOVE].getBrightness(), 1.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if ((i != 3) && (i != 4)) {
            EXPECT_EQ(solimRandomModule.m_moveCounters[i], 1);
        } else { // Only the fourth channel and fifth channel should now have an increased counter
            EXPECT_EQ(solimRandomModule.m_moveCounters[i], 2);
        }
    }

    // Remove the last input trigger
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_MOVE].setVoltage(0.f, 4);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_MOVE].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, false, true, true, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_MOVE].getBrightness(), 0.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if ((i != 3) && (i != 4)) {
            EXPECT_EQ(solimRandomModule.m_moveCounters[i], 1);
        } else { // Only the fourth channel and fifth channel should now have an increased counter
            EXPECT_EQ(solimRandomModule.m_moveCounters[i], 2);
        }
    }
}

TEST(SolimRandomTest, ShouldHandleOneTrigger) {
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    for (int i = 0; i < 8; i++) {
        solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_ONE].channels = 8;
    }

    // Initialize the triggers
    solimRandomModule.process(Module::ProcessArgs());

    // Now press the button
    solimRandomModule.params[SolimRandomModule::ParamsIds::PARAM_TRIG_ONE].setValue(1.f);
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, false, true, true);

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(solimRandomModule.m_oneCounters[i], 1);
        EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ONE].getBrightness(), 1.f, 0.000001f);
    }

    // Release the button
    solimRandomModule.params[SolimRandomModule::ParamsIds::PARAM_TRIG_ONE].setValue(0.f);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ONE].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, false, true, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ONE].getBrightness(), 0.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(solimRandomModule.m_oneCounters[i], 1);
    }

    // Trigger through the input
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_ONE].setVoltage(1.f, 3);
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, false, true, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ONE].getBrightness(), 1.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if (i != 3) {
            EXPECT_EQ(solimRandomModule.m_oneCounters[i], 1);
        } else { // Only the fourth channel should have been triggered
            EXPECT_EQ(solimRandomModule.m_oneCounters[i], 2);
        }
    }

    // Remove the input trigger again, and trigger another
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_ONE].setVoltage(0.f, 3);
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_ONE].setVoltage(1.f, 4);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ONE].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, false, true, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ONE].getBrightness(), 1.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if ((i != 3) && (i != 4)) {
            EXPECT_EQ(solimRandomModule.m_oneCounters[i], 1);
        } else { // Only the fourth channel and fifth channel should now have an increased counter
            EXPECT_EQ(solimRandomModule.m_oneCounters[i], 2);
        }
    }

    // Remove the last input trigger
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_ONE].setVoltage(0.f, 4);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ONE].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, false, true, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ONE].getBrightness(), 0.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if ((i != 3) && (i != 4)) {
            EXPECT_EQ(solimRandomModule.m_oneCounters[i], 1);
        } else { // Only the fourth channel and fifth channel should now have an increased counter
            EXPECT_EQ(solimRandomModule.m_oneCounters[i], 2);
        }
    }
}

TEST(SolimRandomTest, ShouldHandleAllTrigger) {
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    for (int i = 0; i < 8; i++) {
        solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_ALL].channels = 8;
    }

    // Initialize the triggers
    solimRandomModule.process(Module::ProcessArgs());

    // Now press the button
    solimRandomModule.params[SolimRandomModule::ParamsIds::PARAM_TRIG_ALL].setValue(1.f);
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, true, false, true);

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(solimRandomModule.m_allCounters[i], 1);
        EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ALL].getBrightness(), 1.f, 0.000001f);
    }

    // Release the button
    solimRandomModule.params[SolimRandomModule::ParamsIds::PARAM_TRIG_ALL].setValue(0.f);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ALL].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, true, false, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ALL].getBrightness(), 0.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(solimRandomModule.m_allCounters[i], 1);
    }

    // Trigger through the input
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_ALL].setVoltage(1.f, 3);
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, true, false, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ALL].getBrightness(), 1.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if (i != 3) {
            EXPECT_EQ(solimRandomModule.m_allCounters[i], 1);
        } else { // Only the fourth channel should have been triggered
            EXPECT_EQ(solimRandomModule.m_allCounters[i], 2);
        }
    }

    // Remove the input trigger again, and trigger another
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_ALL].setVoltage(0.f, 3);
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_ALL].setVoltage(1.f, 4);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ALL].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, true, false, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ALL].getBrightness(), 1.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if ((i != 3) && (i != 4)) {
            EXPECT_EQ(solimRandomModule.m_allCounters[i], 1);
        } else { // Only the fourth channel and fifth channel should now have an increased counter
            EXPECT_EQ(solimRandomModule.m_allCounters[i], 2);
        }
    }

    // Remove the last input trigger
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_ALL].setVoltage(0.f, 4);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ALL].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, true, false, true);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_ALL].getBrightness(), 0.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if ((i != 3) && (i != 4)) {
            EXPECT_EQ(solimRandomModule.m_allCounters[i], 1);
        } else { // Only the fourth channel and fifth channel should now have an increased counter
            EXPECT_EQ(solimRandomModule.m_allCounters[i], 2);
        }
    }
}

TEST(SolimRandomTest, ShouldHandleResetTrigger) {
    SolimRandomModule solimRandomModule;
    initializeSolimRandomModule(solimRandomModule);

    for (int i = 0; i < 8; i++) {
        solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_RESET].channels = 8;
    }

    // Initialize the triggers
    solimRandomModule.process(Module::ProcessArgs());

    // Now press the button
    solimRandomModule.params[SolimRandomModule::ParamsIds::PARAM_TRIG_RESET].setValue(1.f);
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, true, true, false);

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(solimRandomModule.m_resetCounters[i], 1);
        EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_RESET].getBrightness(), 1.f, 0.000001f);
    }

    // Release the button
    solimRandomModule.params[SolimRandomModule::ParamsIds::PARAM_TRIG_RESET].setValue(0.f);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_RESET].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, true, true, false);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_RESET].getBrightness(), 0.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(solimRandomModule.m_resetCounters[i], 1);
    }

    // Trigger through the input
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_RESET].setVoltage(1.f, 3);
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, true, true, false);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_RESET].getBrightness(), 1.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if (i != 3) {
            EXPECT_EQ(solimRandomModule.m_resetCounters[i], 1);
        } else { // Only the fourth channel should have been triggered
            EXPECT_EQ(solimRandomModule.m_resetCounters[i], 2);
        }
    }

    // Remove the input trigger again, and trigger another
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_RESET].setVoltage(0.f, 3);
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_RESET].setVoltage(1.f, 4);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_RESET].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, true, true, false);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_RESET].getBrightness(), 1.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if ((i != 3) && (i != 4)) {
            EXPECT_EQ(solimRandomModule.m_resetCounters[i], 1);
        } else { // Only the fourth channel and fifth channel should now have an increased counter
            EXPECT_EQ(solimRandomModule.m_resetCounters[i], 2);
        }
    }

    // Remove the last input trigger
    solimRandomModule.inputs[SolimRandomModule::InputsIds::INPUT_TRIG_RESET].setVoltage(0.f, 4);
    solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_RESET].setBrightness(-1.f); // Set the light brightness to a low value, so that the module setting it to 0.f will be applied immediately insteady of smoothly
    solimRandomModule.process(Module::ProcessArgs());
    expectNotTriggered(solimRandomModule, true, true, true, false);

    EXPECT_NEAR(solimRandomModule.lights[SolimRandomModule::LightsIds::LIGHT_TRIG_RESET].getBrightness(), 0.f, 0.000001f);
    for (int i = 0; i < 8; i++) {
        if ((i != 3) && (i != 4)) {
            EXPECT_EQ(solimRandomModule.m_resetCounters[i], 1);
        } else { // Only the fourth channel and fifth channel should now have an increased counter
            EXPECT_EQ(solimRandomModule.m_resetCounters[i], 2);
        }
    }
}
