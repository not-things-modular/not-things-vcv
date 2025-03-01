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

void initializePipoOutputModule(PipoOutputModule& pipoOutputModule, bool populateOutputsAndLeds) {
	pipoOutputModule.lights[PipoOutputModule::LIGHT_CONNECTED].setBrightness(4.20f);
	pipoOutputModule.lights[PipoOutputModule::LIGHT_NOT_CONNECTED].setBrightness(4.20f);

	// Set the output channels and voltages and the LED states
	if (populateOutputsAndLeds) {
		for (int i = 0; i < 8; i++) {
			pipoOutputModule.outputs[PipoOutputModule::OutputId::OUT_OUTPUTS + i].channels = i + 1;
			for (int j = 0; j < i + 1; j++) {
				pipoOutputModule.outputs[PipoOutputModule::OutputId::OUT_OUTPUTS + i].setVoltage(i + j, j);
			}
		}
		for (int i = 0; i < 8; i++) {
			pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3)].setBrightness((i % 3) == 0);
			pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 1].setBrightness((i % 3) == 1);
			pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 2].setBrightness((i % 3) == 2);
		}
	} else {
		// The test will take care of populating the outputs and LEDS, so just set them all to disconnected/off
		for (int i = 0; i < 8; i++) {
			pipoOutputModule.outputs[PipoOutputModule::OutputId::OUT_OUTPUTS + i].setVoltage(6.9f);
			pipoOutputModule.outputs[PipoOutputModule::OutputId::OUT_OUTPUTS + i].channels = 0;
		}
		for (int i = 0; i < 24; i++) {
			pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + i].setBrightness(0.f);
		}
	}
}

void verifyOutputsAndLeds(PipoOutputModule& pipoOutputModule, bool expectReset) {
	if (expectReset) {
		// The expectation is that all outputs are set to single-channel 0V and all LEDs are turned off
		for (int i = 0; i < 8; i++) {
			EXPECT_EQ(pipoOutputModule.outputs[PipoOutputModule::OutputId::OUT_OUTPUTS + i].channels, 1);
			EXPECT_EQ(pipoOutputModule.outputs[PipoOutputModule::OutputId::OUT_OUTPUTS + i].getVoltage(), 0.f);
		}
		for (int i = 0; i < 24; i++) {
			EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + i].getBrightness(), 0.f);
		}
	} else {
		// The outputs and LEDs should have been left the way that they were set in the initializePipoOutputModule method
		for (int i = 0; i < 8; i++) {
			EXPECT_EQ(pipoOutputModule.outputs[PipoOutputModule::OutputId::OUT_OUTPUTS + i].channels, i + 1);
			for (int j = 0; j < i + 1; j++) {
				EXPECT_EQ(pipoOutputModule.outputs[PipoOutputModule::OutputId::OUT_OUTPUTS + i].getVoltage(j), i + j);
			}
		}
		for (int i = 0; i < 8; i++) {
			EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3)].getBrightness(), (i % 3) == 0);
			EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 1].getBrightness(), (i % 3) == 1);
			EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 2].getBrightness(), (i % 3) == 2);
		}
	}
}

void expectConnected(PipoOutputModule& pipoOutputModule, bool connected) {
	if (connected) {
		EXPECT_NEAR(pipoOutputModule.lights[PipoOutputModule::LightId::LIGHT_CONNECTED].getBrightness(), 1.f, 0.0001f);
		EXPECT_NEAR(pipoOutputModule.lights[PipoOutputModule::LightId::LIGHT_NOT_CONNECTED].getBrightness(), 0.f, 0.0001f);
	} else {
		EXPECT_NEAR(pipoOutputModule.lights[PipoOutputModule::LightId::LIGHT_CONNECTED].getBrightness(), 0.f, 0.0001f);
		EXPECT_NEAR(pipoOutputModule.lights[PipoOutputModule::LightId::LIGHT_NOT_CONNECTED].getBrightness(), 1.f, 0.0001f);
	}
}

TEST(PipoOutputTest, WithNoExpandersShouldDeactivateLed) {
	PipoOutputModule pipoOutputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	pipoOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(pipoOutputModule, false);
	verifyOutputsAndLeds(pipoOutputModule, true);
}

TEST(PipoOutputTest, WithPipoInputOnLeftShouldActivateLed) {
	PipoInputModule pipoInputModule;
	PipoOutputModule pipoOutputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	registerExpanderModule(pipoOutputModule, ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT));
	pipoOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(pipoOutputModule, true);
	verifyOutputsAndLeds(pipoOutputModule, false);
}

TEST(PipoOutputTest, WithPipoInputOnRightShouldDeactivateLed) {
	PipoInputModule pipoInputModule;
	PipoOutputModule pipoOutputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	registerExpanderModule(pipoOutputModule, ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::RIGHT));
	pipoOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(pipoOutputModule, false);
	verifyOutputsAndLeds(pipoOutputModule, true);
}

TEST(PipoOutputTest, WithUnknownModuleBeforePipoInputOnLeftShouldDectivateLed) {
	Module module;
	PipoInputModule pipoInputModule;
	PipoOutputModule pipoOutputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	registerExpanderModules(pipoOutputModule, {
		ExpanderData(module, createModel<Module, DummyWidget>("dummy"), ExpanderSide::LEFT),
		ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT)
	});
	pipoOutputModule.draw(widget::Widget::DrawArgs());

	expectConnected(pipoOutputModule, false);
	verifyOutputsAndLeds(pipoOutputModule, true);
}

TEST(PipoOutputTest, InChainOfOutputModulesWithPipoInputOnLeftShouldActivateLed) {
	PipoInputModule pipoInputModule;
	PipoOutputModule pipoOutputModule[3];
	initializePipoOutputModule(pipoOutputModule[0], true);

	registerExpanderModules(pipoOutputModule[0], {
		ExpanderData(pipoOutputModule[0], modelPipoOutput, ExpanderSide::LEFT),
		ExpanderData(pipoOutputModule[1], modelPipoOutput, ExpanderSide::LEFT),
		ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT)
	});
	pipoOutputModule[0].draw(widget::Widget::DrawArgs());

	expectConnected(pipoOutputModule[0], true);
	verifyOutputsAndLeds(pipoOutputModule[0], false);
}

TEST(PipoOutputTest, InChainOfOutputModulesWithNoPipoInputOnLeftShouldDeactivateLed) {
	PipoOutputModule pipoOutputModule[3];
	initializePipoOutputModule(pipoOutputModule[0], true);

	registerExpanderModules(pipoOutputModule[0], {
		ExpanderData(pipoOutputModule[0], modelPipoOutput, ExpanderSide::LEFT),
		ExpanderData(pipoOutputModule[1], modelPipoOutput, ExpanderSide::LEFT)
	});
	pipoOutputModule[0].draw(widget::Widget::DrawArgs());

	expectConnected(pipoOutputModule[0], false);
	verifyOutputsAndLeds(pipoOutputModule[0], true);
}

TEST(PipoOutputTest, InChainOfOutputModulesWithUnknownModuleBeforePipoInputOnLeftShouldDeactivateLed) {
	Module module;
	PipoInputModule pipoInputModule;
	PipoOutputModule pipoOutputModule[3];
	initializePipoOutputModule(pipoOutputModule[0], true);

	registerExpanderModules(pipoOutputModule[0], {
		ExpanderData(pipoOutputModule[0], modelPipoOutput, ExpanderSide::LEFT),
		ExpanderData(pipoOutputModule[1], modelPipoOutput, ExpanderSide::LEFT),
		ExpanderData(module, createModel<Module, DummyWidget>("dummy"), ExpanderSide::LEFT),
		ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT)
	});
	pipoOutputModule[0].draw(widget::Widget::DrawArgs());

	expectConnected(pipoOutputModule[0], false);
	verifyOutputsAndLeds(pipoOutputModule[0], true);
}

TEST(PipoOutputTest, WithNoInputModuleShouldNotProcess) {
	PipoOutputModule pipoOutputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	pipoOutputModule.process(Module::ProcessArgs());

	verifyOutputsAndLeds(pipoOutputModule, false);
}

TEST(PipoOutputTest, WithSingleInputModuleAndNoInputsOrOutputsConnectedShouldProcess) {
	PipoOutputModule pipoOutputModule;
	PipoInputModule pipoInputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	registerExpanderModule(pipoOutputModule, ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT));
	pipoOutputModule.process(Module::ProcessArgs());

	// All outputs should be set to a single-channel 0V (since the input ports are not connected), and the green LED should be on.
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[i].getChannels(), 1);
		EXPECT_EQ(pipoOutputModule.outputs[i].getVoltage(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3)].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 1].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 2].getBrightness(), 1.f);
	}
}

TEST(PipoOutputTest, ProcessShouldDimLEDOnDisconnectedOutputWithNoMatchingInputChannel) {
	PipoOutputModule pipoOutputModule;
	PipoInputModule pipoInputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	for (int i = 0; i < 8; i++) {
		pipoInputModule.inputs[i].channels = 1;
		pipoInputModule.inputs[i].setVoltage(i + 1);
	}

	// Make the first output receive all eight channels of the input module and connect it
	pipoOutputModule.params[0].setValue(8);
	pipoOutputModule.outputs[0].channels = 1;
	// The other seven outputs are not connected
	for (int i = 1; i < 8; i++) {
		pipoOutputModule.outputs[i].channels = 0;
	}

	registerExpanderModule(pipoOutputModule, ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT));
	pipoOutputModule.process(Module::ProcessArgs());

	// The first output should have received all eight input voltages, and have its green LED on.
	EXPECT_EQ(pipoOutputModule.outputs[0].getChannels(), 8);
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[0].getVoltage(i), i + 1);
	}
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 2].getBrightness(), 1.f);

	// All other outputs should have their LEDs dimmed, still be disconnected and have their voltage set to 0V.
	for (int i = 1; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[i].getChannels(), 0);
		EXPECT_EQ(pipoOutputModule.outputs[i].getVoltage(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3)].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 1].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 2].getBrightness(), 0.f);
	}
}

TEST(PipoOutputTest, ProcessShouldDimLEDOnDisconnectedOutputWithNoMatchingInputChannels) {
	PipoOutputModule pipoOutputModule;
	PipoInputModule pipoInputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	for (int i = 0; i < 8; i++) {
		pipoInputModule.inputs[i].channels = 1;
		pipoInputModule.inputs[i].setVoltage(i + 1);
	}

	// Make the first output receive all eight channels of the input module and connect it
	pipoOutputModule.params[0].setValue(8);
	pipoOutputModule.outputs[0].channels = 1;
	// The other seven outputs are not connected, and expect an increasing number of channels
	for (int i = 1; i < 8; i++) {
		pipoOutputModule.params[i].setValue(i);
		pipoOutputModule.outputs[i].channels = 0;
	}

	registerExpanderModule(pipoOutputModule, ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT));
	pipoOutputModule.process(Module::ProcessArgs());

	// The first output should have received all eight input voltages, and have its green LED on.
	EXPECT_EQ(pipoOutputModule.outputs[0].getChannels(), 8);
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[0].getVoltage(i), i + 1);
	}
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 2].getBrightness(), 1.f);

	// All other outputs should have their LEDs dimmed, still be disconnected and have their voltage set to 0V.
	for (int i = 1; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[i].getChannels(), 0);
		EXPECT_EQ(pipoOutputModule.outputs[i].getVoltage(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3)].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 1].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 2].getBrightness(), 0.f);
	}
}

TEST(PipoOutputTest, ProcessShouldShowRedLEDOnConnectedOutputWithNoMatchingInputChannel) {
	PipoOutputModule pipoOutputModule;
	PipoInputModule pipoInputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	for (int i = 0; i < 8; i++) {
		pipoInputModule.inputs[i].channels = 1;
		pipoInputModule.inputs[i].setVoltage(i + 1);
	}

	// Make the first output receive all eight channels of the input module and connect it
	pipoOutputModule.params[0].setValue(8);
	pipoOutputModule.outputs[0].channels = 1;
	// The other seven outputs are not connected
	for (int i = 1; i < 8; i++) {
		pipoOutputModule.outputs[i].channels = 1;
	}

	registerExpanderModule(pipoOutputModule, ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT));
	pipoOutputModule.process(Module::ProcessArgs());

	// The first output should have received all eight input voltages, and have its green LED on.
	EXPECT_EQ(pipoOutputModule.outputs[0].getChannels(), 8);
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[0].getVoltage(i), i + 1);
	}
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 2].getBrightness(), 1.f);

	// All other outputs should have their LEDs dimmed, still be disconnected and have their voltage set to 0V.
	for (int i = 1; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[i].getChannels(), 1);
		EXPECT_EQ(pipoOutputModule.outputs[i].getVoltage(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3)].getBrightness(), 1.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 1].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 2].getBrightness(), 0.f);
	}
}

TEST(PipoOutputTest, ProcessShouldShowRedLEDOnConnectedOutputWithNoMatchingInputChannels) {
	PipoOutputModule pipoOutputModule;
	PipoInputModule pipoInputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	for (int i = 0; i < 8; i++) {
		pipoInputModule.inputs[i].channels = 1;
		pipoInputModule.inputs[i].setVoltage(i + 1);
	}

	// Make the first output receive all eight channels of the input module and connect it
	pipoOutputModule.params[0].setValue(8);
	pipoOutputModule.outputs[0].channels = 1;
	// The other seven outputs are not connected, and expect an increasing number of channels
	for (int i = 1; i < 8; i++) {
		pipoOutputModule.params[i].setValue(i);
		pipoOutputModule.outputs[i].channels = 1;
	}

	registerExpanderModule(pipoOutputModule, ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT));
	pipoOutputModule.process(Module::ProcessArgs());

	// The first output should have received all eight input voltages, and have its green LED on.
	EXPECT_EQ(pipoOutputModule.outputs[0].getChannels(), 8);
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[0].getVoltage(i), i + 1);
	}
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 2].getBrightness(), 1.f);

	// All other outputs should have their LEDs dimmed, still be disconnected and have their voltage set to 0V.
	for (int i = 1; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[i].getChannels(), i);
		EXPECT_EQ(pipoOutputModule.outputs[i].getVoltage(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3)].getBrightness(), 1.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 1].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 2].getBrightness(), 0.f);
	}
}

TEST(PipoOutputTest, ProcessShouldShowOrangeLEDOnDisconnectedOutputWithIncompleteInputChannels) {
	PipoOutputModule pipoOutputModule;
	PipoInputModule pipoInputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	for (int i = 0; i < 8; i++) {
		pipoInputModule.inputs[i].channels = 1;
		pipoInputModule.inputs[i].setVoltage(i + 1);
	}

	// Make the first output receive all seven out of eight channels of the input module and connect it
	pipoOutputModule.params[0].setValue(7);
	pipoOutputModule.outputs[0].channels = 1;
	// The other seven outputs are not connected
	for (int i = 1; i < 8; i++) {
		pipoOutputModule.outputs[i].channels = 0;
	}
	// The second output requests two channels (while there is only one remaining)
	pipoOutputModule.params[1].setValue(2);

	registerExpanderModule(pipoOutputModule, ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT));
	pipoOutputModule.process(Module::ProcessArgs());

	// The first output should have received seven input voltages, and have its green LED on.
	EXPECT_EQ(pipoOutputModule.outputs[0].getChannels(), 7);
	for (int i = 0; i < 7; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[0].getVoltage(i), i + 1);
	}
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 2].getBrightness(), 1.f);

	// The second output should have the orange LED on, but still be disconnected
	EXPECT_EQ(pipoOutputModule.outputs[1].getChannels(), 0);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3 + 1].getBrightness(), 1.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3 + 2].getBrightness(), 0.f);

	// All other outputs should have their LEDs dimmed, still be disconnected and have their voltage set to 0V.
	for (int i = 2; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[i].getChannels(), 0);
		EXPECT_EQ(pipoOutputModule.outputs[i].getVoltage(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3)].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 1].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 2].getBrightness(), 0.f);
	}
}

TEST(PipoOutputTest, ProcessShouldShowOrangeLEDOnConnectedOutputWithIncompleteInputChannels) {
	PipoOutputModule pipoOutputModule;
	PipoInputModule pipoInputModule;
	initializePipoOutputModule(pipoOutputModule, true);

	for (int i = 0; i < 8; i++) {
		pipoInputModule.inputs[i].channels = 1;
		pipoInputModule.inputs[i].setVoltage(i + 1);
	}

	// Make the first output receive all seven out of eight channels of the input module and connect it
	pipoOutputModule.params[0].setValue(7);
	pipoOutputModule.outputs[0].channels = 1;
	// The other seven outputs are not connected
	for (int i = 2; i < 8; i++) {
		pipoOutputModule.outputs[i].channels = 0;
	}
	// The second output requests two channels (while there is only one remaining) and is connected
	pipoOutputModule.outputs[1].channels = 1;
	pipoOutputModule.params[1].setValue(2);

	registerExpanderModule(pipoOutputModule, ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT));
	pipoOutputModule.process(Module::ProcessArgs());

	// The first output should have received seven input voltages, and have its green LED on.
	EXPECT_EQ(pipoOutputModule.outputs[0].getChannels(), 7);
	for (int i = 0; i < 7; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[0].getVoltage(i), i + 1);
	}
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 2].getBrightness(), 1.f);

	// The second output should have the orange LED on, have two channels, received one channel and have the other default to 0V
	EXPECT_EQ(pipoOutputModule.outputs[1].getChannels(), 2);
	EXPECT_EQ(pipoOutputModule.outputs[1].getVoltage(0), 8.f);
	EXPECT_EQ(pipoOutputModule.outputs[1].getVoltage(1), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3 + 1].getBrightness(), 1.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3 + 2].getBrightness(), 0.f);

	// All other outputs should have their LEDs dimmed, still be disconnected and have their voltage set to 0V.
	for (int i = 2; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[i].getChannels(), 0);
		EXPECT_EQ(pipoOutputModule.outputs[i].getVoltage(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3)].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 1].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 2].getBrightness(), 0.f);
	}
}

TEST(PipoOutputTest, ShouldDistributeChannelsFromOnePolyphonicInputOverMultipleConnectedOutputs) {
	PipoOutputModule pipoOutputModule;
	PipoInputModule pipoInputModule;
	initializePipoOutputModule(pipoOutputModule, false);

	registerExpanderModule(pipoOutputModule, ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT));

	// The first input has a 6.9f voltage
	pipoInputModule.inputs[0].channels = 1;
	pipoInputModule.inputs[0].setVoltage(6.9f);

	// The second input has 16 polyphonic channels
	pipoInputModule.inputs[1].channels = 16;
	for (int i = 0; i < 16; i++) {
		pipoInputModule.inputs[1].setVoltage(i - 8, i);
	}

	// The other inputs are disconnected, and as such will default to a monophonic 0V input
	for (int i = 2; i < 8; i++) {
		pipoInputModule.inputs[i].channels = 0;
	}

	// The first output expects 1 channel and is connected
	pipoOutputModule.params[0].setValue(1);
	pipoOutputModule.outputs[0].channels = 1;

	// The second output expects 8 channels and is connected
	pipoOutputModule.params[1].setValue(8);
	pipoOutputModule.outputs[1].channels = 1;

	// The third output expects 3 channels and is connected
	pipoOutputModule.params[2].setValue(3);
	pipoOutputModule.outputs[2].channels = 1;

	// The fourth output expects 7 channels and is connected
	pipoOutputModule.params[3].setValue(7);
	pipoOutputModule.outputs[3].channels = 1;
	
	// The other outputs expect 1 channel and are connected
	for (int i = 4; i < 8; i++) {
		pipoOutputModule.params[i].setValue(1);
		pipoOutputModule.outputs[i].channels = 1;
	}

	pipoOutputModule.process(Module::ProcessArgs());

	EXPECT_EQ(pipoOutputModule.outputs[0].channels, 1);
	EXPECT_EQ(pipoOutputModule.outputs[0].getVoltage(), 6.9f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 2].getBrightness(), 1.f);

	EXPECT_EQ(pipoOutputModule.outputs[1].channels, 8);
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[1].getVoltage(i), -8 + i);
	}
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3 + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3 + 2].getBrightness(), 1.f);

	EXPECT_EQ(pipoOutputModule.outputs[2].channels, 3);
	for (int i = 0; i < 3; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[2].getVoltage(i), i);
	}
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 6].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 6 + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 6 + 2].getBrightness(), 1.f);

	EXPECT_EQ(pipoOutputModule.outputs[3].channels, 7);
	// The first four channels are from the second input with 16 channels
	for (int i = 0; i < 4; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[3].getVoltage(i), i + 3);
	}
	// The last two channels are from the disconnected inputs
	EXPECT_EQ(pipoOutputModule.outputs[3].getVoltage(5), 0);
	EXPECT_EQ(pipoOutputModule.outputs[3].getVoltage(6), 0);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 9].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 9 + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 9 + 2].getBrightness(), 1.f);

	// The remaining output ports should be monophonic with 0V and a green LED
	for (int i = 4; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[i].channels, 1);
		EXPECT_EQ(pipoOutputModule.outputs[i].getVoltage(0), 0);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3)].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 1].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 2].getBrightness(), 1.f);
		}
}

TEST(PipoOutputTest, ShouldDistributeChannelsFromOnePolyphonicInputOverMultipleConnectedAndDisconnectedOutputs) {
	PipoOutputModule pipoOutputModule;
	PipoInputModule pipoInputModule;
	initializePipoOutputModule(pipoOutputModule, false);

	registerExpanderModule(pipoOutputModule, ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT));

	// The first input has a 6.9f voltage
	pipoInputModule.inputs[0].channels = 1;
	pipoInputModule.inputs[0].setVoltage(6.9f);

	// The second input has 16 polyphonic channels
	pipoInputModule.inputs[1].channels = 16;
	for (int i = 0; i < 16; i++) {
		pipoInputModule.inputs[1].setVoltage(i - 8, i);
	}

	// The other inputs are disconnected, and as such will default to a monophonic 0V input
	for (int i = 2; i < 8; i++) {
		pipoInputModule.inputs[i].channels = 0;
	}

	// The first output expects 1 channel and is connected
	pipoOutputModule.params[0].setValue(1);
	pipoOutputModule.outputs[0].channels = 1;

	// The second output expects 8 channels and is not connected
	pipoOutputModule.params[1].setValue(8);
	pipoOutputModule.outputs[1].channels = 0;

	// The third output expects 3 channels and is connected
	pipoOutputModule.params[2].setValue(3);
	pipoOutputModule.outputs[2].channels = 1;

	// The fourth output expects 7 channels and is not connected
	pipoOutputModule.params[3].setValue(7);
	pipoOutputModule.outputs[3].channels = 0;
	
	// The other outputs expect 1 channel and are connected
	for (int i = 4; i < 8; i++) {
		pipoOutputModule.params[i].setValue(1);
		pipoOutputModule.outputs[i].channels = 1;
	}

	pipoOutputModule.process(Module::ProcessArgs());

	EXPECT_EQ(pipoOutputModule.outputs[0].channels, 1);
	EXPECT_EQ(pipoOutputModule.outputs[0].getVoltage(), 6.9f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 2].getBrightness(), 1.f);

	EXPECT_EQ(pipoOutputModule.outputs[1].channels, 0);
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[1].getVoltage(i), -8 + i);
	}
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3 + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 3 + 2].getBrightness(), 1.f);

	EXPECT_EQ(pipoOutputModule.outputs[2].channels, 3);
	for (int i = 0; i < 3; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[2].getVoltage(i), i);
	}
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 6].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 6 + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 6 + 2].getBrightness(), 1.f);

	EXPECT_EQ(pipoOutputModule.outputs[3].channels, 0);
	// The first four channels are from the second input with 16 channels
	for (int i = 0; i < 4; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[3].getVoltage(i), i + 3);
	}
	// The last two channels are from the disconnected inputs
	EXPECT_EQ(pipoOutputModule.outputs[3].getVoltage(5), 0);
	EXPECT_EQ(pipoOutputModule.outputs[3].getVoltage(6), 0);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 9].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 9 + 1].getBrightness(), 0.f);
	EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + 9 + 2].getBrightness(), 1.f);

	// The remaining output ports should be monophonic with 0V and a green LED
	for (int i = 4; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[i].channels, 1);
		EXPECT_EQ(pipoOutputModule.outputs[i].getVoltage(0), 0);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3)].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 1].getBrightness(), 0.f);
		EXPECT_EQ(pipoOutputModule.lights[PipoOutputModule::LightId::OUT_LIGHTS + (i * 3) + 2].getBrightness(), 1.f);
		}
}

TEST(PipoOutputTest, ShouldUseInputsFromMultipleInputModules) {
	PipoOutputModule pipoOutputModule;
	PipoInputModule pipoInputModule[3];
	initializePipoOutputModule(pipoOutputModule, false);

	registerExpanderModules(pipoOutputModule, {
		ExpanderData(pipoInputModule[0], modelPipoInput, ExpanderSide::LEFT),
		ExpanderData(pipoInputModule[1], modelPipoInput, ExpanderSide::LEFT),
		ExpanderData(pipoInputModule[2], modelPipoInput, ExpanderSide::LEFT)
	});

	float inputVoltage = 0.f;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 8; j++) {
			pipoInputModule[i].inputs[j].channels = j + 1;
			for (int k = 0; k < j + 1; k++) {
				pipoInputModule[i].inputs[j].setVoltage(inputVoltage, k);
				inputVoltage += .01f;
			}
		}
	}

	for (int i = 0; i < 8; i++) {
		pipoOutputModule.params[i].setValue(16);
		pipoOutputModule.outputs[i].channels = 1;
	}

	pipoOutputModule.process(Module::ProcessArgs());

	float outputVoltage = 0.f;
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(pipoOutputModule.outputs[i].getChannels(), 16);
		for (int j = 0; j < 16; j++) {
			EXPECT_NEAR(pipoOutputModule.outputs[i].getVoltage(j), (outputVoltage < inputVoltage - 0.00001f) ? outputVoltage : 0.f, 0.00001f);
			outputVoltage += .01f;
		}
	}
}

TEST(PipoOutputTest, ShouldUseOutputsOfMultipleOutputModules) {
	PipoOutputModule pipoOutputModule[3];
	PipoInputModule pipoInputModule;
	initializePipoOutputModule(pipoOutputModule[0], false);

	registerExpanderModule(pipoOutputModule[0], ExpanderData(pipoInputModule, modelPipoInput, ExpanderSide::LEFT));
	registerExpanderModules(pipoOutputModule[0], {
		ExpanderData(pipoOutputModule[1], modelPipoOutput, ExpanderSide::RIGHT),
		ExpanderData(pipoOutputModule[2], modelPipoOutput, ExpanderSide::RIGHT)
	});

	float inputVoltage = 0.f;
	for (int i = 0; i < 8; i++) {
		pipoInputModule.inputs[i].channels = 16;
		for (int j = 0; j < 16; j++) {
			pipoInputModule.inputs[i].setVoltage(inputVoltage, j);
			inputVoltage += .01f;
		}
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 8; j++) {
			pipoOutputModule[i].params[j].setValue(j + 1);
			pipoOutputModule[i].outputs[j].channels = 1;
		}
	}

	pipoOutputModule[0].process(Module::ProcessArgs());

	float outputVoltage = 0.f;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 8; j++) {
			EXPECT_EQ(pipoOutputModule[i].outputs[j].getChannels(), j + 1);
			for (int k = 0; k < j + 1; k++) {
				EXPECT_NEAR(pipoOutputModule[i].outputs[j].getVoltage(k), outputVoltage, 0.00001f);
				outputVoltage += .01f;
			}
		}
	}
}
