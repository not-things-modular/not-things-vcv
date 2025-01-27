#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "modules/solim-input.hpp"
#include "modules/solim.hpp"
#include "modules/solim-random.hpp"
#include "modules/solim-input-octaver.hpp"
#include "modules/solim-output.hpp"
#include "modules/solim-output-octaver.hpp"

#include "../common/vcv-registration.hpp"

struct DummyWidget : ModuleWidget {
    DummyWidget(Module* module) {}
};

void initializeSolimInputModule(SolimInputModule& solimInputModule) {
    for (int i = 0; i < SolimInputModule::LightIds::NUM_LIGHTS; i++) {
        solimInputModule.lights[i].setBrightness(-99.f);
    }
}

void expectConnected(SolimInputModule& solimInputModule, bool connected) {
    if (connected) {
        EXPECT_NEAR(solimInputModule.lights[SolimInputModule::LightIds::LIGHT_CONNECTED].getBrightness(), 1.f, 0.0001f);
        EXPECT_NEAR(solimInputModule.lights[SolimInputModule::LightIds::LIGHT_NOT_CONNECTED].getBrightness(), 0.f, 0.0001f);
    } else {
        EXPECT_NEAR(solimInputModule.lights[SolimInputModule::LightIds::LIGHT_CONNECTED].getBrightness(), 0.f, 0.0001f);
        EXPECT_NEAR(solimInputModule.lights[SolimInputModule::LightIds::LIGHT_NOT_CONNECTED].getBrightness(), 1.f, 0.0001f);
    }
}

TEST(SolimInputTest, WithNoExpandersShouldDeactivateLed) {
    SolimInputModule solimInputModule;
    initializeSolimInputModule(solimInputModule);

    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, false);
}

TEST(SolimInputTest, WithSolimExpanderOnRightShouldActivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    initializeSolimInputModule(solimInputModule);

    registerExpanderModule(solimInputModule, ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT));
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, true);
}

TEST(SolimInputTest, WithSolimExpanderOnLeftShouldDeactivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    initializeSolimInputModule(solimInputModule);

    registerExpanderModule(solimInputModule, ExpanderData(solimModule, modelSolim, ExpanderSide::LEFT));
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, false);
}

TEST(SolimInputTest, WithUnknownExpanderBeforeSolimExpanderShouldDectivateLed) {
    Module module;
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(module, createModel<Module, DummyWidget>("dummy"), ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, false);
}

TEST(SolimInputTest, WithSolimOutputBeforeSolimExpanderShouldDectivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimOutputModule solimOutputModule;
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimOutputModule, modelSolimOutput, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, false);
}

TEST(SolimInputTest, WithSolimOutputOctaverBeforeSolimExpanderShouldDectivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimOutputOctaverModule solimOutputOctaverModule;
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, false);
}

TEST(SolimInputTest, WithSingleInputAndSolimExpanderOnRightShouldActivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimInputModule solimInputExpanderModule;
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimInputExpanderModule, modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, true);
}

TEST(SolimInputTest, WithUpToSixInputExpanderAndASolimExpanderOnRightShouldActivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimInputModule solimInputExpanderModule[6];
    initializeSolimInputModule(solimInputModule);

    for (int i = 0; i < 6; i++) {
        if (i == 0) {
            registerExpanderModule(solimInputModule, ExpanderData(solimInputExpanderModule[i], modelSolimInput, ExpanderSide::RIGHT));
        } else {
            registerExpanderModule(solimInputExpanderModule[i - 1], ExpanderData(solimInputExpanderModule[i], modelSolimInput, ExpanderSide::RIGHT));
        }
        registerExpanderModule(solimInputExpanderModule[i], ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT));

        solimInputModule.draw(widget::Widget::DrawArgs());

        expectConnected(solimInputModule, true);
    }
}

TEST(SolimInputTest, WithSevenInputExpandersAndASolimExpanderOnRightShouldDeactivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimInputModule solimInputExpanderModule[7];
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimInputExpanderModule[0], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[1], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[2], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[3], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[4], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[5], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[6], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, false);
}

TEST(SolimInputTest, WithSolimRandomAndSolimExpanderOnRightShouldActivateLed) {
    SolimModule solimModule;
    SolimRandomModule solimRandomModule;
    SolimInputModule solimInputModule;
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, true);
}

TEST(SolimInputTest, WithSolimRandomAndSevenInputExpandersAndASolimExpanderOnRightShouldDeactivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimRandomModule solimRandomModule;
    SolimInputModule solimInputExpanderModule[7];
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimInputExpanderModule[0], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[1], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[2], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[3], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[4], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[5], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[6], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, false);
}

TEST(SolimInputTest, WithTwoSolimRandomAndSolimExpanderOnRightShouldDeactivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimRandomModule solimRandomModule[2];
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimRandomModule[0], modelSolimRandom, ExpanderSide::RIGHT),
        ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, false);
}

TEST(SolimInputTest, WithSolimInputOctaverAndSolimExpanderOnRightShouldActivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimInputOctaverModule solimInputOctaverModule;
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, true);
}

TEST(SolimInputTest, WithSolimInputOctaverAndSevenInputExpandersAndASolimOnRightShouldDeactivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimInputModule solimInputExpanderModule[7];
    SolimInputOctaverModule solimInputOctaverModule;
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimInputExpanderModule[0], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[1], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[2], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[3], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[4], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[5], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule[6], modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });

    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, false);
}

TEST(SolimInputTest, WithTwoSolimInputOctaverAndSolimExpanderOnRightShouldDeactivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimInputOctaverModule solimInputOctaverModule[2];
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimInputOctaverModule[0], modelSolimInputOctaver, ExpanderSide::RIGHT),
        ExpanderData(solimInputOctaverModule[1], modelSolimInputOctaver, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, false);
}

TEST(SolimInputTest, WithSolimInputExpanderAndSolimRandomAndSolimInputOctaverAndSolimOnRightShouldActivateLed) {
    SolimModule solimModule;
    SolimInputModule solimInputModule;
    SolimRandomModule solimRandomModule;
    SolimInputModule solimInputExpanderModule;
    SolimInputOctaverModule solimInputOctaverModule;
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule, modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::RIGHT),
        ExpanderData(solimModule, modelSolim, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, true);
}

TEST(SolimInputTest, WithSolimInputExpanderAndSolimRandomAndSolimInputOctaverButNoSolimOnRightShouldDeactivateLed) {
    SolimInputModule solimInputModule;
    SolimRandomModule solimRandomModule;
    SolimInputModule solimInputExpanderModule;
    SolimInputOctaverModule solimInputOctaverModule;
    initializeSolimInputModule(solimInputModule);

    registerExpanderModules(solimInputModule, {
        ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::RIGHT),
        ExpanderData(solimInputExpanderModule, modelSolimInput, ExpanderSide::RIGHT),
        ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::RIGHT)
    });
    solimInputModule.draw(widget::Widget::DrawArgs());

    expectConnected(solimInputModule, false);
}
