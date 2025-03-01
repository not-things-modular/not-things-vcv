#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "modules/poly-same-diff.hpp"

#include "../common/vcv-registration.hpp"

void initializePolySameDiffModule(PolySameDiffModule& polySameDiffModule) {
	polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A].channels = 0;
	polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B].channels = 0;

	polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels = 0;
	polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels = 0;
	polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels = 0;

	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.f);
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_MODE].setValue(0.f);

	polySameDiffModule.setOutputDuplicates(false);
}

void populateOutputs(PolySameDiffModule& polySameDiffModule) {
	setPortVoltages(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A], std::vector<float>(16, 6.9f));
	setPortVoltages(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB], std::vector<float>(16, 6.9f));
	setPortVoltages(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B], std::vector<float>(16, 6.9f));
}

void checkJsonOutputDuplicates(json_t* jsonData, bool expectedOutputDuplicates) {
	EXPECT_TRUE(json_is_object(jsonData));

	json_t* jsonPolySameDiffOutputDuplicates = json_object_get(jsonData, "ntPolySameDiffOutputDuplicates");
	EXPECT_TRUE(json_is_boolean(jsonPolySameDiffOutputDuplicates));

	EXPECT_EQ(json_boolean_value(jsonPolySameDiffOutputDuplicates), expectedOutputDuplicates);
}

TEST(PolySameDiffTest, WithNoInputsShouldHaveNoOutputs) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 0);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 0);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 0);
}

TEST(PolySameDiffTest, WithOnlyAInputShouldAssignToAOutput) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);
	polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B].channels = 0;

	polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A].channels = 11;
	for (int i = 0; i < 11; i++) {
		polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A].setVoltage(i + .42f, i);
	}

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 0);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 0);

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 11);
	for (int i = 0; i < 11; i++) {
		EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(i), i + .42f);
	}
}

TEST(PolySameDiffTest, WithOnlyAInputShouldAssignDifferentValuesToAOutput) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);
	polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B].channels = 0;

	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A], { .1f, 1.1, -1.f, .4f, 2.f, 1.4f, -1.2f, -1.5f });
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.4f);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 0);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 0);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 5);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(0), .1f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(1), 1.1f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(2), -1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(3), 2.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(4), -1.5f);
}

TEST(PolySameDiffTest, WithOnlyBInputShouldAssignToBOutput) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);
	polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A].channels = 0;

	polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B].channels = 11;
	for (int i = 0; i < 11; i++) {
		polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B].setVoltage(i + .42f, i);
	}

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 0);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 0);

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 11);
	for (int i = 0; i < 11; i++) {
		EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(i), i + .42f);
	}
}

TEST(PolySameDiffTest, WithOnlyBInputShouldAssignDifferentValuesToBOutput) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);
	polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A].channels = 0;

	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B], { .1f, 1.1, -1.f, .4f, 2.f, 1.4f, -1.2f, -1.5f });
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.4f);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 0);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 0);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 5);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(0), .1f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(1), 1.1f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(2), -1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(3), 2.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(4), -1.5f);
}

TEST(PolySameDiffTest, WithNoDeltaInVoltageModeAndNoDuplicateOutputsShouldMatchOnlySameVoltages) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);

	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A], { .44f, -2.f, -1.f, .33f, -2.0001f, -.56f, 0.f, .34f, 1.f, -1.5f, 1.0001f });
	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B], { 2.75f, -1.9999f, 2.5f, -1.5f, 2.55f, .35f, .0001f, 1.f });
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.f);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 2);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(0), 1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(1), -1.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 9);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(0), .44f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(1), -2.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(2), -1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(3), .33f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(4), -2.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(5), -.56f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(6), 0.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(7), .34f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(8), 1.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 6);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(0), 2.75f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(1), -1.9999f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(2), 2.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(3), 2.55f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(4), .35f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(5), .0001f);
}

TEST(PolySameDiffTest, WithNoDeltaInVoltageModeAndDuplicateOutputsShouldMatchOnlySameVoltagesAndKeepDuplicates) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);

	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A], { .44f, -2.f, -1.f, .33f, -2.0001f, -.56f, 0.f, .34f, 1.f, -1.5f, 1.0001f });
	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B], { 2.75f, -1.9999f, 2.5f, -1.5f, 2.55f, .35f, .0001f, 1.f });
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.f);
	polySameDiffModule.setOutputDuplicates(true);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 4);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(0), 1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(1), -1.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(2), -1.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(3), 1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 9);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(0), .44f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(1), -2.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(2), -1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(3), .33f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(4), -2.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(5), -.56f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(6), 0.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(7), .34f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(8), 1.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 6);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(0), 2.75f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(1), -1.9999f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(2), 2.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(3), 2.55f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(4), .35f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(5), .0001f);
}

TEST(PolySameDiffTest, WithDeltaInVoltageModeAndNoDuplicateOutputsShouldMatchCloseVoltages) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);

	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A], { .44f, -2.f, -1.f, .33f, -2.0001f, -.56f, 0.f, .34f, 1.f, -1.5f, 1.0001f });
	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B], { 2.75f, -1.9999f, 2.5f, -1.5f, 2.55f, .35f, .0001f, 1.f });
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.15f);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 5);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(0), .44f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(1), -2.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(2), 0.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(3), 1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(4), -1.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 2);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(0), -1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(1), -.56f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 2);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(0), 2.75f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(1), 2.5f);
}

TEST(PolySameDiffTest, WithDeltaInVoltageModeAndDuplicateOutputsShouldMatchCloseVoltagesAndOutputDuplicates) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);

	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A], { .44f, -2.f, -1.f, .33f, -2.0001f, -.56f, 0.f, .34f, 1.f, -1.5f, 1.0001f });
	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B], { 2.75f, -1.9999f, 2.5f, -1.5f, 2.55f, .35f, .0001f, 1.f });
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.15f);
	polySameDiffModule.setOutputDuplicates(true);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 14);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(0), .44f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(1), -2.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(2), .33f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(3), -2.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(4), 0.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(5), .34f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(6), 1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(7), -1.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(8), 1.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(9), -1.9999f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(10), -1.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(11), .35f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(12), .0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(13), 1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 2);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(0), -1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(1), -.56f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 3);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(0), 2.75f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(1), 2.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(2), 2.55f);
}

TEST(PolySameDiffTest, WithNoDeltaInNoteModeAndNoDuplicateOutputsShouldMatchOnlySameNotes) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);

	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A], { .44f, -2.f, -1.f, .33f, -2.0001f, -.56f, 0.f, .34f, 1.f, -1.5f, 1.0001f });
	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B], { 2.75f, -1.9999f, 2.5f, -1.5f, 2.55f, .35f, .0001f, 1.f });
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.f);
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_MODE].setValue(1.f);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 3);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(0), -2.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(1), -1.5);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(2), 1.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 4);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(0), .44f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(1), .33f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(2), -2.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(3), .34f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 3);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(0), 2.75f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(1), 2.55f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(2), .35f);
}

TEST(PolySameDiffTest, WithNoDeltaInNoteModeAndDuplicateOutputsShouldMatchOnlySameNotesAndKeepDuplicates) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);

	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A], { .44f, -2.f, -1.f, .33f, -2.0001f, -.56f, 0.f, .34f, 1.f, -1.5f, 1.0001f });
	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B], { 2.75f, -1.9999f, 2.5f, -1.5f, 2.55f, .35f, .0001f, 1.f });
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.f);
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_MODE].setValue(1.f);
	polySameDiffModule.setOutputDuplicates(true);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 11);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(0), -2.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(1), -1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(2), 0.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(3), 1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(4), -1.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(5), 1.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(6), -1.9999f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(7), 2.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(8), -1.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(9), .0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(10), 1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 5);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(0), .44f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(1), .33f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(2), -2.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(3), -.56f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].getVoltage(4), .34f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 3);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(0), 2.75f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(1), 2.55f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(2), .35f);
}

TEST(PolySameDiffTest, WithDeltaInNoteModeAndNoDuplicateOutputsShouldMatchCloseNotes) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);

	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A], { .44f, -2.f, -1.f, .33f, -2.0001f, -.56f, 0.f, .34f, 1.f, -1.5f, 1.0001f });
	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B], { 2.75f, -1.9999f, 2.5f, -1.5f, 2.55f, .35f, .0001f, 1.f });
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.15f);
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_MODE].setValue(1.f);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 2);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(0), .44f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(1), -2.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 0);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 1);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(0), 2.75f);
}

TEST(PolySameDiffTest, WithDeltaInNoteModeAndDuplicateOutputsShouldMatchCloseNotesAndOutputDuplicates) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);

	// One less voltage in each of the inputs in this test compared to the other tests, since we'll go over the output count limit of the AB match otherwise
	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A], { .44f, -2.f, -1.f, .33f, -2.0001f, -.56f, 0.f, .34f, -1.5f, 1.0001f });
	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B], { 2.75f, -1.9999f, 2.5f, 2.55f, .35f, .0001f, 1.f });
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.15f);
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_MODE].setValue(1.f);
	polySameDiffModule.setOutputDuplicates(true);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 16);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(0), .44f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(1), -2.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(2), -1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(3), .33f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(4), -2.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(5), -.56f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(6), 0.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(7), .34f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(8), -1.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(9), 1.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(10), -1.9999f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(11), 2.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(12), 2.55f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(13), .35f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(14), .0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(15), 1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 0);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 1);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(0), 2.75f);
}

TEST(PolySameDiffTest, WithDeltaInNoteModeAndDuplicateOutputsShouldMatchCloseNotesAndOutputDuplicatesUpToLimit) {
	PolySameDiffModule polySameDiffModule;

	initializePolySameDiffModule(polySameDiffModule);
	populateOutputs(polySameDiffModule);

	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_A], { .44f, -2.f, -1.f, .33f, -2.0001f, -.56f, 0.f, .34f, 1.f, -1.5f, 1.0001f });
	setPortVoltages(polySameDiffModule.inputs[PolySameDiffModule::InputId::IN_B], { 2.75f, -1.9999f, 2.5f, -1.5f, 2.55f, .35f, .0001f, 1.f });
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_DELTA].setValue(0.15f);
	polySameDiffModule.params[PolySameDiffModule::ParamId::PARAM_MODE].setValue(1.f);
	polySameDiffModule.setOutputDuplicates(true);

	polySameDiffModule.process(Module::ProcessArgs());

	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].channels, 16); // There are two more values from the B matches, but 16 is the limit
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(0), .44f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(1), -2.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(2), -1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(3), .33f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(4), -2.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(5), -.56f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(6), 0.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(7), .34f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(8), 1.f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(9), -1.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(10), 1.0001f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(11), -1.9999f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(12), 2.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(13), -1.5f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(14), 2.55f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_AB].getVoltage(15), .35f);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_A].channels, 0);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].channels, 1);
	EXPECT_EQ(polySameDiffModule.outputs[PolySameDiffModule::OutputId::OUT_B].getVoltage(0), 2.75f);
}

TEST(PolySameDiffTest, ShouldDefaultToNoDuplicateOutput) {
	PolySameDiffModule polySameDiffModule;

	EXPECT_EQ(polySameDiffModule.getOutputDuplicates(), false);

	json_t* jsonData = polySameDiffModule.dataToJson();
	checkJsonOutputDuplicates(jsonData, false);

	delete jsonData;
}

TEST(PolySameDiffTest, ShouldSwitchToDuplicateOutput) {
	PolySameDiffModule polySameDiffModule;

	json_t *jsonData = json_object();
	json_object_set_new(jsonData, "ntPolySameDiffOutputDuplicates", json_boolean(true));
	polySameDiffModule.dataFromJson(jsonData);
	
	EXPECT_TRUE(polySameDiffModule.getOutputDuplicates());

	delete jsonData;
}

TEST(PolySameDiffTest, ShouldSwitchToNoDuplicateOutput) {
	PolySameDiffModule polySameDiffModule;
	polySameDiffModule.setOutputDuplicates(true);

	EXPECT_TRUE(polySameDiffModule.getOutputDuplicates());
	json_t *jsonData = json_object();
	json_object_set_new(jsonData, "ntPolySameDiffOutputDuplicates", json_boolean(false));
	polySameDiffModule.dataFromJson(jsonData);
	
	EXPECT_FALSE(polySameDiffModule.getOutputDuplicates());

	delete jsonData;
}


TEST(PolySameDiffTest, ShouldSwitchToDefaultNoDuplicateOutputOnInvalidConfigData) {
	PolySameDiffModule polySameDiffModule;
	polySameDiffModule.setOutputDuplicates(true);

	EXPECT_TRUE(polySameDiffModule.getOutputDuplicates());
	json_t *jsonData = json_object();
	json_object_set_new(jsonData, "ntPolySameDiffOutputDuplicates", json_integer(1));
	polySameDiffModule.dataFromJson(jsonData);
	
	EXPECT_FALSE(polySameDiffModule.getOutputDuplicates());

	delete jsonData;
}
TEST(PolySameDiffTest, ShouldPersistOutputDuplicates) {
	PolySameDiffModule polySameDiffModule;
	polySameDiffModule.setOutputDuplicates(true);

	EXPECT_TRUE(polySameDiffModule.getOutputDuplicates());
	json_t* jsonData = polySameDiffModule.dataToJson();
	checkJsonOutputDuplicates(jsonData, true);

	delete jsonData;
}

TEST(PolySameDiffTest, ShouldPersistOutputNoDuplicates) {
	PolySameDiffModule polySameDiffModule;
	polySameDiffModule.setOutputDuplicates(true);
	polySameDiffModule.setOutputDuplicates(false);

	EXPECT_FALSE(polySameDiffModule.getOutputDuplicates());
	json_t* jsonData = polySameDiffModule.dataToJson();
	checkJsonOutputDuplicates(jsonData, false);

	delete jsonData;
}
