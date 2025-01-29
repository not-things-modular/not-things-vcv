#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "modules/solim-input.hpp"
#include "modules/solim.hpp"
#include "modules/solim-random.hpp"
#include "modules/solim-input-octaver.hpp"
#include "modules/solim-output.hpp"
#include "modules/solim-output-octaver.hpp"
#include "core/solim-core.hpp"

#include "../common/vcv-registration.hpp"
#include "../common/save-data.hpp"

using RandomTriggerArray = std::array<RandomTrigger, 8>*;
struct MockSolimCore : SolimCore {
	MOCK_METHOD(SolimValueSet&, getActiveValues, (int), (override));
	MOCK_METHOD(SolimValueSet&, getInactiveValues, (int), (override));
	MOCK_METHOD(void, processAndActivateInactiveValues, (int, RandomTriggerArray), (override));
};

struct DummyWidget : ModuleWidget {
	DummyWidget(Module* module) {}
};

void initializeSolimModule(SolimModule& solimModule) {
	for (int i = 0; i < SolimModule::InputId::NUM_INPUTS; i++) {
		solimModule.inputs[i].channels = 0;
		for (int j = 0; j < 8; j++) {
			solimModule.inputs[i].setVoltage(j, 0.f);
		}
	}
	for (int i = 0; i < SolimModule::OutputId::NUM_OUTPUTS; i++) {
		solimModule.outputs[i].channels = 0;
		for (int j = 0; j < 8; j++) {
			solimModule.outputs[i].setVoltage(j, 0.f);
		}
	}
	for (int i = 0; i < SolimModule::LightId::NUM_LIGHTS; i++) {
		solimModule.lights[i].setBrightness(-99.f);
	}
}

std::array<RandomTrigger, 8> createRandomTriggers(RandomTrigger randomTrigger) {
	return { randomTrigger, randomTrigger, randomTrigger, randomTrigger, randomTrigger, randomTrigger, randomTrigger };
}

TEST(SolimTest, DrawShouldUpdateNoteDisplays) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	solimModule.m_lowerDisplay = new NoteDisplay();
	solimModule.m_upperDisplay = new NoteDisplay();
	initializeSolimModule(solimModule);

	SolimValueSet solimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(solimValueSet));

	solimValueSet.lowerLimit = -0.9999f;
	solimValueSet.upperLimit = 1.0001f;
	solimModule.draw(widget::Widget::DrawArgs());
	EXPECT_EQ(solimModule.m_lowerDisplay->scale, 4);
	EXPECT_EQ(solimModule.m_lowerDisplay->note, 0);
	EXPECT_EQ(solimModule.m_upperDisplay->scale, 6);
	EXPECT_EQ(solimModule.m_upperDisplay->note, 0);

	solimValueSet.lowerLimit = -1.00001f;
	solimValueSet.upperLimit = 1.0001f;
	solimModule.draw(widget::Widget::DrawArgs());
	EXPECT_EQ(solimModule.m_lowerDisplay->scale, 3);
	EXPECT_EQ(solimModule.m_lowerDisplay->note, 11);
	EXPECT_EQ(solimModule.m_upperDisplay->scale, 6);
	EXPECT_EQ(solimModule.m_upperDisplay->note, 0);

	solimValueSet.lowerLimit = -1.00001f;
	solimValueSet.upperLimit = -4.250f;
	solimModule.draw(widget::Widget::DrawArgs());
	EXPECT_EQ(solimModule.m_lowerDisplay->scale, 3);
	EXPECT_EQ(solimModule.m_lowerDisplay->note, 11);
	EXPECT_EQ(solimModule.m_upperDisplay->scale, 0);
	EXPECT_EQ(solimModule.m_upperDisplay->note, 9);

	delete solimModule.m_lowerDisplay;
	delete solimModule.m_upperDisplay;
}

TEST(SolimTest, ProcessWithoutExpandersOrExternalInputAndEightInputsShouldReadValuesIntoInactiveSet) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(3.24f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(4.62f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });
	}

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet.inputValueCount, 8);
	EXPECT_EQ(inactiveSolimValueSet.lowerLimit, 3.24f);
	EXPECT_EQ(inactiveSolimValueSet.upperLimit, 4.62f);
	EXPECT_EQ(inactiveSolimValueSet.sort, -1);
	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].addOctave, SolimValue::AddOctave::NONE);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].replaceOriginal, false);
	}
}

TEST(SolimTest, ProcessWithoutExpandersOrExternalInputAndFourInputsShouldReadValuesIntoInactiveSet) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(3.24f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(4.62f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	for (int i = 0; i < 4; i++) {
		// Not-connected inputs should be ignored while reading the input values, so leave an unconnected on in between each one.
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + 1 + (i * 2)], { (float) i + 1});
	}

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet.inputValueCount, 4);
	EXPECT_EQ(inactiveSolimValueSet.lowerLimit, 3.24f);
	EXPECT_EQ(inactiveSolimValueSet.upperLimit, 4.62f);
	EXPECT_EQ(inactiveSolimValueSet.sort, -1);
	for (int i = 0; i < 4; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].addOctave, SolimValue::AddOctave::NONE);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].replaceOriginal, false);
	}
}

TEST(SolimTest, ProcessWithoutExpandersOrExternalInputAndOnePolyphonicInputShouldReadValuesIntoInactiveSet) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(3.24f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(4.62f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + 3], { 1, 2, 3, 4});

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet.inputValueCount, 4);
	EXPECT_EQ(inactiveSolimValueSet.lowerLimit, 3.24f);
	EXPECT_EQ(inactiveSolimValueSet.upperLimit, 4.62f);
	EXPECT_EQ(inactiveSolimValueSet.sort, -1);
	for (int i = 0; i < 4; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].addOctave, SolimValue::AddOctave::NONE);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].replaceOriginal, false);
	}
}

TEST(SolimTest, ProcessWithoutExpandersOrExternalInputAndEightInputsShouldIgnorePolyphonicInputs) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(3.24f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(4.62f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	for (int i = 0; i < 8; i++) {
		solimModule.inputs[SolimModule::IN_INPUTS + i].channels = i + 1;
		solimModule.inputs[SolimModule::IN_INPUTS + i].setVoltage(i + 1);
	}

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet.inputValueCount, 8);
	EXPECT_EQ(inactiveSolimValueSet.lowerLimit, 3.24f);
	EXPECT_EQ(inactiveSolimValueSet.upperLimit, 4.62f);
	EXPECT_EQ(inactiveSolimValueSet.sort, -1);
	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].addOctave, SolimValue::AddOctave::NONE);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].replaceOriginal, false);
	}
}

TEST(SolimTest, ProcessWithoutExpandersOrExternalInputShouldLimitLimits) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(-5.5f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(5.5f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet.lowerLimit, -5.f);
	EXPECT_EQ(inactiveSolimValueSet.upperLimit, 5.f);
}

TEST(SolimTest, ProcessWithoutExpandersOrExternalInputShouldReadSortAsc) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	solimModule.params[SolimModule::PARAM_SORT].setValue(1.2f);

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet.sort, 1);
}

TEST(SolimTest, ProcessWithoutExpandersOrExternalInputShouldReadSortNone) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	solimModule.params[SolimModule::PARAM_SORT].setValue(0.f);

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet.sort, 0);
}

TEST(SolimTest, ProcessWithoutExpandersOrExternalInputShouldReadSortDesc) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet.sort, -1);
}

TEST(SolimTest, ProcessWithoutExpandersOrExternalInputShouldReadSortFromInput) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	// The param value should be ignored when an input is connected
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);
	setInputVoltages(solimModule.inputs[SolimModule::IN_SORT], { 1.2f });

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet.sort, 1);
}

TEST(SolimTest, ProcessWithoutExpandersOrExternalInputShouldReadLowerLimitFromInput) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	// The param value for lower limit should be ignored when an input is connected for it
	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(-1.2f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(3.2f);
	setInputVoltages(solimModule.inputs[SolimModule::IN_LOWER_LIMIT], { 1.2f });

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet.lowerLimit, 1.2f);
	EXPECT_EQ(inactiveSolimValueSet.upperLimit, 3.2f);
}

TEST(SolimTest, ProcessWithoutExpandersOrExternalInputShouldReadUpperLimitFromInput) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	// The param value for upper limit should be ignored when an input is connected for it
	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(-1.2f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(3.2f);
	setInputVoltages(solimModule.inputs[SolimModule::IN_UPPER_LIMIT], { 1.2f });

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet.lowerLimit, -1.2f);
	EXPECT_EQ(inactiveSolimValueSet.upperLimit, 1.2f);
}

TEST(SolimTest, ProcessWithInputOctaverExpanderOnRightSideShouldNotUseExpander) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	registerExpanderModule(solimModule, ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::RIGHT));

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1});

		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue((i % 3) - 1);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_SORT_POSITION + i].setValue((i % 2) - 1);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue((i % 2));
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].addOctave, SolimValue::AddOctave::NONE);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].replaceOriginal, false);
	}
}

TEST(SolimTest, ProcessWithInputOctaverExpanderOnLeftSideAndOutputOctaverExpanderInBetweenShouldNotUseExpander) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputOctaverModule solimInputOctaverModule;
	SolimOutputOctaverModule solimOutputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::LEFT),
		ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::LEFT)
	});

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });

		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue((i % 3) - 1);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_SORT_POSITION + i].setValue((i % 2) - 1);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue((i % 2));
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].addOctave, SolimValue::AddOctave::NONE);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].replaceOriginal, false);
	}
}

TEST(SolimTest, ProcessWithInputOctaverExpanderOnLeftSideShouldUseExpander) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	registerExpanderModule(solimModule, ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::LEFT));

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });

		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue((i % 3) - 1);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_SORT_POSITION + i].setValue(i % 2);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue(i % 2);
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].addOctave, i % 3 == 0 ? SolimValue::AddOctave::LOWER : i % 3 == 1 ? SolimValue::AddOctave::NONE : SolimValue::AddOctave::HIGHER);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].sortRelative, i % 2 == 0 ? SolimValue::SortRelative::BEFORE : SolimValue::SortRelative::AFTER);
		EXPECT_EQ(inactiveSolimValueSet.inputValues[i].replaceOriginal, i % 2 == 1);
	}
}

TEST(SolimTest, ProcessWithInputOctaverExpanderOnLeftSideShouldUseExpanderWithMultipleColumns) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[3];
	SolimOutputModule solimOutputModule[3];
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[4];
	SolimValueSet inactiveSolimValueSet[4];
	for (int i = 0; i < 4; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(4, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::LEFT)
	});

	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT)
	});

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });
		for (int j = 0; j < 3; j++) {
			setInputVoltages(solimInputModule[j].inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });
		}

		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue((i % 3) - 1);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_SORT_POSITION + i].setValue(i % 2);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue(i % 2);
	}

	solimModule.process(Module::ProcessArgs());

	for (int column = 0; column < 4; column++) {
		for (int i = 0; i < 8; i++) {
			EXPECT_EQ(inactiveSolimValueSet[column].inputValues[i].addOctave, i % 3 == 0 ? SolimValue::AddOctave::LOWER : i % 3 == 1 ? SolimValue::AddOctave::NONE : SolimValue::AddOctave::HIGHER);
			EXPECT_EQ(inactiveSolimValueSet[column].inputValues[i].sortRelative, i % 2 == 0 ? SolimValue::SortRelative::BEFORE : SolimValue::SortRelative::AFTER);
			EXPECT_EQ(inactiveSolimValueSet[column].inputValues[i].replaceOriginal, i % 2 == 1);
		}
	}
}

TEST(SolimTest, ProcessWithInputOctaverExpanderOnLeftSideShouldUseExpanderWithMultipleColumnsAndPolyphonicInputs) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[3];
	SolimOutputModule solimOutputModule[3];
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[4];
	SolimValueSet inactiveSolimValueSet[4];
	for (int i = 0; i < 4; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(4, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::LEFT)
	});

	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT)
	});

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });
		for (int j = 0; j < 3; j++) {
			setInputVoltages(solimInputModule[j].inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });
		}

		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue((i % 3) - 1);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_SORT_POSITION + i].setValue(i % 2);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue(i % 2);
	}

	// On the fourth channel, the addOctave will always be set to HIGHER (instead of LOWER as was set on the param by the loop above)
	solimInputOctaverModule.inputs[SolimInputOctaverModule::ParamId::PARAM_ADD_OCTAVE + 3].channels = 1;
	solimInputOctaverModule.inputs[SolimInputOctaverModule::ParamId::PARAM_ADD_OCTAVE + 3].setVoltage(1.f, 0);

	// On the fifth channel, the sortPosition for will always be set to BEFORE (instead of AFTER as was set on the param by the loop above), except for the first colum, which will remain AFTER
	solimInputOctaverModule.inputs[SolimInputOctaverModule::ParamId::PARAM_SORT_POSITION + 4].channels = 2;
	solimInputOctaverModule.inputs[SolimInputOctaverModule::ParamId::PARAM_SORT_POSITION + 4].setVoltage(0.f, 0);
	solimInputOctaverModule.inputs[SolimInputOctaverModule::ParamId::PARAM_SORT_POSITION + 4].setVoltage(1.f, 1);

	// On the sixth channel, all replaceOriginal values will be set to false (instead of true as was set on the param by the loop above)
	solimInputOctaverModule.inputs[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 5].channels = 4;
	solimInputOctaverModule.inputs[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 5].setVoltage(0.f, 0);
	solimInputOctaverModule.inputs[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 5].setVoltage(0.f, 1);
	solimInputOctaverModule.inputs[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 5].setVoltage(0.f, 2);
	solimInputOctaverModule.inputs[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 5].setVoltage(0.f, 3);

	solimModule.process(Module::ProcessArgs());

	for (int column = 0; column < 4; column++) {
		for (int i = 0; i < 8; i++) {
			if (i == 3) {
				EXPECT_EQ(inactiveSolimValueSet[column].inputValues[i].addOctave, SolimValue::AddOctave::HIGHER);
			} else {
				EXPECT_EQ(inactiveSolimValueSet[column].inputValues[i].addOctave, i % 3 == 0 ? SolimValue::AddOctave::LOWER : i % 3 == 1 ? SolimValue::AddOctave::NONE : SolimValue::AddOctave::HIGHER);
			}
			if (i == 4) {
				if (column == 0) {
					EXPECT_EQ(inactiveSolimValueSet[column].inputValues[i].sortRelative, SolimValue::SortRelative::BEFORE);
				} else {
					EXPECT_EQ(inactiveSolimValueSet[column].inputValues[i].sortRelative, SolimValue::SortRelative::AFTER);
				}
			} else {
				EXPECT_EQ(inactiveSolimValueSet[column].inputValues[i].sortRelative, i % 2 == 0 ? SolimValue::SortRelative::BEFORE : SolimValue::SortRelative::AFTER);
			}
			if (i == 5) {
				EXPECT_EQ(inactiveSolimValueSet[column].inputValues[i].replaceOriginal, false);
			} else {
				EXPECT_EQ(inactiveSolimValueSet[column].inputValues[i].replaceOriginal, i % 2 == 1);
			}
		}
	}
}

TEST(SolimTest, ProcessSolimInputOctaverExpanderInputsShouldOverwriteParams) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	registerExpanderModule(solimModule, ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::LEFT));

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });

		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue(1);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_SORT_POSITION + i].setValue(1);
		solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue(1);
	}

	setInputVoltages(solimInputOctaverModule.inputs[SolimInputOctaverModule::InputId::IN_ADD_OCTAVE + 1], { -.5f });
	setInputVoltages(solimInputOctaverModule.inputs[SolimInputOctaverModule::InputId::IN_ADD_OCTAVE + 3], { 0.f });
	solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_ADD_OCTAVE + 5].setValue(0.f);
	setInputVoltages(solimInputOctaverModule.inputs[SolimInputOctaverModule::InputId::IN_ADD_OCTAVE + 5], { .5f });

	setInputVoltages(solimInputOctaverModule.inputs[SolimInputOctaverModule::InputId::IN_SORT_POSITION + 2], { 0.f });
	solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_SORT_POSITION + 4].setValue(0.f);
	setInputVoltages(solimInputOctaverModule.inputs[SolimInputOctaverModule::InputId::IN_SORT_POSITION + 4], { 1.5f });

	setInputVoltages(solimInputOctaverModule.inputs[SolimInputOctaverModule::InputId::IN_REPLACE_ORIGINAL + 5], { 0.f });
	solimInputOctaverModule.params[SolimInputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 7].setValue(0.f);
	setInputVoltages(solimInputOctaverModule.inputs[SolimInputOctaverModule::InputId::IN_REPLACE_ORIGINAL + 7], { 1.5f });

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);

		if (i == 1) {
			EXPECT_EQ(inactiveSolimValueSet.inputValues[i].addOctave, -1);
		} else if (i == 3) {
			EXPECT_EQ(inactiveSolimValueSet.inputValues[i].addOctave, 0);
		} else {
			EXPECT_EQ(inactiveSolimValueSet.inputValues[i].addOctave, 1);
		}

		if (i == 2) {
			EXPECT_EQ(inactiveSolimValueSet.inputValues[i].sortRelative, 0);
		} else {
			EXPECT_EQ(inactiveSolimValueSet.inputValues[i].sortRelative, 1);
		}

		if (i == 5) {
			EXPECT_EQ(inactiveSolimValueSet.inputValues[i].replaceOriginal, 0);
		} else {
			EXPECT_EQ(inactiveSolimValueSet.inputValues[i].replaceOriginal, 1);
		}
	}
}

TEST(SolimTest, ProcessWithOutputOctaverExpanderOnLeftSideShouldNotUseExpander) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimOutputOctaverModule solimOutputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	registerExpanderModule(solimModule, ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::LEFT));

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1});

		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue((i % 3) - 1);
		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue((i % 2));
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet.outputOctaves[i], SolimValue::AddOctave::NONE);
		EXPECT_EQ(inactiveSolimValueSet.outputReplaceOriginal[i], false);
	}
}

TEST(SolimTest, ProcessWithOutputOctaverExpanderOnRightSideAndInputOctaverExpanderInBetweenShouldNotUseExpander) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimOutputOctaverModule solimOutputOctaverModule;
	SolimInputOctaverModule solimInputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputOctaverModule, modelSolimInputOctaver, ExpanderSide::RIGHT),
		ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::RIGHT)
	});

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });

		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue((i % 3) - 1);
		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue((i % 2));
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet.outputOctaves[i], SolimValue::AddOctave::NONE);
		EXPECT_EQ(inactiveSolimValueSet.outputReplaceOriginal[i], false);
	}
}

TEST(SolimTest, ProcessWithOutputOctaverExpanderOnRightSideShouldUseExpander) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimOutputOctaverModule solimOutputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	registerExpanderModule(solimModule, ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::RIGHT));

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });

		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue((i % 3) - 1);
		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue(i % 2);
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet.outputOctaves[i], i % 3 == 0 ? SolimValue::AddOctave::LOWER : i % 3 == 1 ? SolimValue::AddOctave::NONE : SolimValue::AddOctave::HIGHER);
		EXPECT_EQ(inactiveSolimValueSet.outputReplaceOriginal[i], i % 2 == 1);
	}
	EXPECT_FALSE(inactiveSolimValueSet.resort);
}

TEST(SolimTest, ProcessWithOutputOctaverAndExpanderOnRightSideAndMultipleIOsShouldUseExpander) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[3];
	SolimOutputModule solimOutputModule[3];
	SolimOutputOctaverModule solimOutputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[4];
	SolimValueSet inactiveSolimValueSet[4];
	for (int i = 0; i < 4; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(4, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT)
	});

	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::RIGHT)
	});

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });

		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue((i % 3) - 1);
		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue(i % 2);
	}

	solimModule.process(Module::ProcessArgs());

	for (int column = 0; column < 4; column++) {
		for (int i = 0; i < 8; i++) {
			EXPECT_EQ(inactiveSolimValueSet[column].outputOctaves[i], i % 3 == 0 ? SolimValue::AddOctave::LOWER : i % 3 == 1 ? SolimValue::AddOctave::NONE : SolimValue::AddOctave::HIGHER);
			EXPECT_EQ(inactiveSolimValueSet[column].outputReplaceOriginal[i], i % 2 == 1);
		}
		EXPECT_FALSE(inactiveSolimValueSet[column].resort);
	}
}

TEST(SolimTest, ProcessWithOutputOctaverAndExpanderOnRightSideAndMultipleIOsShouldUseExpanderWithPolyphonicInputs) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[3];
	SolimOutputModule solimOutputModule[3];
	SolimOutputOctaverModule solimOutputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[4];
	SolimValueSet inactiveSolimValueSet[4];
	for (int i = 0; i < 4; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(4, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT)
	});

	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::RIGHT)
	});

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });

		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue((i % 3) - 1);
		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue(i % 2);
	}

	// Add octaves on the third input should be AddOctave::LOWER for all columns (while in the loop above it would have been set to HIGHER)
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_ADD_OCTAVE + 2].channels = 1;
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_ADD_OCTAVE + 2].setVoltage(-1.f, 0);

	// Replace original on the fourth input should be on for all columns
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 3].channels = 1;
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 3].setVoltage(1.f, 0);

	// Replace original on the sixth input should be off for the first column, and on for all others
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 5].channels = 2;
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 5].setVoltage(0.f, 0);
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 5].setVoltage(1.f, 1);

	// The replace original on the sevent input should be off for all columns
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 6].channels = 4;
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 6].setVoltage(1.f, 0);
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 6].setVoltage(1.f, 1);
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 6].setVoltage(1.f, 2);
	solimOutputOctaverModule.inputs[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 6].setVoltage(1.f, 3);

	solimModule.process(Module::ProcessArgs());

	for (int column = 0; column < 4; column++) {
		for (int i = 0; i < 8; i++) {
			if (i == 2) {
				EXPECT_EQ(inactiveSolimValueSet[column].outputOctaves[i], SolimValue::AddOctave::LOWER);
			} else {
				EXPECT_EQ(inactiveSolimValueSet[column].outputOctaves[i], i % 3 == 0 ? SolimValue::AddOctave::LOWER : i % 3 == 1 ? SolimValue::AddOctave::NONE : SolimValue::AddOctave::HIGHER);
			}
			if (i == 3) {
				EXPECT_EQ(inactiveSolimValueSet[column].outputReplaceOriginal[i], true);
			} else if (i == 5) {
				if (column == 0) {
					EXPECT_EQ(inactiveSolimValueSet[column].outputReplaceOriginal[i], false);
				} else {
					EXPECT_EQ(inactiveSolimValueSet[column].outputReplaceOriginal[i], true);
				}
			} else if (i == 6) {
				EXPECT_EQ(inactiveSolimValueSet[column].outputReplaceOriginal[i], true);
			} else {
				EXPECT_EQ(inactiveSolimValueSet[column].outputReplaceOriginal[i], i % 2 == 1) << "Failed on column " << column << " and i " << i;
			}
		}
		EXPECT_FALSE(inactiveSolimValueSet[column].resort);
	}
}

TEST(SolimTest, ProcessWithOutputOctaverExpanderOnRightSideShouldUseExpanderWithResort) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimOutputOctaverModule solimOutputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	registerExpanderModule(solimModule, ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::RIGHT));

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });

		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue((i % 3) - 1);
		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue(i % 2);
	}
	solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_RESORT].setValue(1.f);

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet.outputOctaves[i], i % 3 == 0 ? SolimValue::AddOctave::LOWER : i % 3 == 1 ? SolimValue::AddOctave::NONE : SolimValue::AddOctave::HIGHER);
		EXPECT_EQ(inactiveSolimValueSet.outputReplaceOriginal[i], i % 2 == 1);
	}
	EXPECT_TRUE(inactiveSolimValueSet.resort);
}

TEST(SolimTest, ProcessSolimOutputOctaverExpanderInputsShouldOverwriteParams) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimOutputOctaverModule solimOutputOctaverModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	registerExpanderModule(solimModule, ExpanderData(solimOutputOctaverModule, modelSolimOutputOctaver, ExpanderSide::RIGHT));

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + 1 });

		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_ADD_OCTAVE + i].setValue(1);
		solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + i].setValue(1);
	}

	setInputVoltages(solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputId::IN_ADD_OCTAVE + 1], { -.5f });
	setInputVoltages(solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputId::IN_ADD_OCTAVE + 3], { 0.f });
	solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_ADD_OCTAVE + 5].setValue(0.f);
	setInputVoltages(solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputId::IN_ADD_OCTAVE + 5], { .5f });

	setInputVoltages(solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputId::IN_REPLACE_ORIGINAL + 5], { 0.f });
	solimOutputOctaverModule.params[SolimOutputOctaverModule::ParamId::PARAM_REPLACE_ORIGINAL + 7].setValue(0.f);
	setInputVoltages(solimOutputOctaverModule.inputs[SolimOutputOctaverModule::InputId::IN_REPLACE_ORIGINAL + 7], { 1.5f });

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet.inputValues[i].value, i + 1, 0.00001f);

		if (i == 1) {
			EXPECT_EQ(inactiveSolimValueSet.outputOctaves[i], -1);
		} else if (i == 3) {
			EXPECT_EQ(inactiveSolimValueSet.outputOctaves[i], 0);
		} else {
			EXPECT_EQ(inactiveSolimValueSet.outputOctaves[i], 1);
		}

		if (i == 5) {
			EXPECT_EQ(inactiveSolimValueSet.outputReplaceOriginal[i], 0);
		} else {
			EXPECT_EQ(inactiveSolimValueSet.outputReplaceOriginal[i], 1);
		}
	}
}

TEST(SolimTest, ProcessShouldUseRandomOnLeftSide) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimRandomModule solimRandomModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);

	registerExpanderModule(solimModule, ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT));

	solimModule.process(Module::ProcessArgs());
}

TEST(SolimTest, ProcessShouldUseRandomOnRightSide) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimRandomModule solimRandomModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);

	registerExpanderModule(solimModule, ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::RIGHT));

	solimModule.process(Module::ProcessArgs());
}

TEST(SolimTest, ProcessShouldUseRandomOnLeftSideIfThereAreRandomsOnBothSides) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimRandomModule solimRandomModule[2];
	std::array<RandomTrigger, 8> randomTriggers;
	initializeSolimModule(solimModule);

	// The left random module will have a reset trigger queued up, so expect that to be passed in.
	randomTriggers = createRandomTriggers(RandomTrigger::NONE);
	randomTriggers[0] = RandomTrigger::RESET;

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(2).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	{
		testing::InSequence seq;
		// The first call will be the first time that there is a random involved, so it will do nothing.
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The second one we can actually verify
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(randomTriggers))).Times(1);
	}

	registerExpanderModule(solimModule, ExpanderData(solimRandomModule[0], modelSolimRandom, ExpanderSide::LEFT));
	registerExpanderModule(solimModule, ExpanderData(solimRandomModule[1], modelSolimRandom, ExpanderSide::RIGHT));

	// Upon first attaching a random module, no actual triggers are done
	solimModule.process(Module::ProcessArgs());

	// Now set up the triggers, and verify which one is called in the second process.
	solimRandomModule[0].m_resetCounters[0] = 1;
	solimRandomModule[1].m_oneCounters[0] = 1;
	solimModule.process(Module::ProcessArgs());
}

TEST(SolimTest, ProcessWithRandomShouldReceiveMoveTrigger) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimRandomModule solimRandomModule;
	std::array<RandomTrigger, 8> randomTriggers;
	initializeSolimModule(solimModule);

	randomTriggers = createRandomTriggers(RandomTrigger::NONE);
	randomTriggers[0] = RandomTrigger::MOVE;

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(2).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	{
		testing::InSequence seq;
		// The first call will be the first time that there is a random involved, so it will do nothing.
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The second one we can actually verify
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(randomTriggers))).Times(1);
	}

	registerExpanderModule(solimModule, ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT));

	// Upon first attaching a random module, no actual triggers are done
	solimModule.process(Module::ProcessArgs());

	// Now set up the triggers, and verify which one is called in the second process.
	solimRandomModule.m_moveCounters[0] = 1;
	solimModule.process(Module::ProcessArgs());
}

TEST(SolimTest, ProcessWithRandomShouldReceiveOneTrigger) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimRandomModule solimRandomModule;
	std::array<RandomTrigger, 8> randomTriggers;
	initializeSolimModule(solimModule);

	randomTriggers = createRandomTriggers(RandomTrigger::NONE);
	randomTriggers[0] = RandomTrigger::ONE;

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(2).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	{
		testing::InSequence seq;
		// The first call will be the first time that there is a random involved, so it will do nothing.
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The second one we can actually verify
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(randomTriggers))).Times(1);
	}

	registerExpanderModule(solimModule, ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT));

	// Upon first attaching a random module, no actual triggers are done
	solimModule.process(Module::ProcessArgs());

	// Now set up the triggers, and verify which one is called in the second process.
	solimRandomModule.m_oneCounters[0] = 1;
	solimModule.process(Module::ProcessArgs());
}

TEST(SolimTest, ProcessWithRandomShouldReceiveAllTrigger) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimRandomModule solimRandomModule;
	std::array<RandomTrigger, 8> randomTriggers;
	initializeSolimModule(solimModule);

	randomTriggers = createRandomTriggers(RandomTrigger::NONE);
	randomTriggers[0] = RandomTrigger::ALL;

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(2).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	{
		testing::InSequence seq;
		// The first call will be the first time that there is a random involved, so it will do nothing.
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The second one we can actually verify
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(randomTriggers))).Times(1);
	}

	registerExpanderModule(solimModule, ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT));

	// Upon first attaching a random module, no actual triggers are done
	solimModule.process(Module::ProcessArgs());

	// Now set up the triggers, and verify which one is called in the second process.
	solimRandomModule.m_allCounters[0] = 1;
	solimModule.process(Module::ProcessArgs());
}

TEST(SolimTest, ProcessWithRandomShouldReceiveResetTrigger) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimRandomModule solimRandomModule;
	std::array<RandomTrigger, 8> randomTriggers;
	initializeSolimModule(solimModule);

	randomTriggers = createRandomTriggers(RandomTrigger::NONE);
	randomTriggers[0] = RandomTrigger::RESET;

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(2).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	{
		testing::InSequence seq;
		// The first call will be the first time that there is a random involved, so it will do nothing.
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The second one we can actually verify
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(randomTriggers))).Times(1);
	}

	registerExpanderModule(solimModule, ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT));

	// Upon first attaching a random module, no actual triggers are done
	solimModule.process(Module::ProcessArgs());

	// Now set up the triggers, and verify which one is called in the second process.
	solimRandomModule.m_resetCounters[0] = 1;
	solimModule.process(Module::ProcessArgs());
}

TEST(SolimTest, ProcessWithRandomWithResetAllOneAndMoveTriggersShouldUseResetTrigger) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimRandomModule solimRandomModule;
	std::array<RandomTrigger, 8> randomTriggers;
	initializeSolimModule(solimModule);

	randomTriggers = createRandomTriggers(RandomTrigger::NONE);
	randomTriggers[0] = RandomTrigger::RESET;

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(3).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	{
		testing::InSequence seq;
		// The first call will be the first time that there is a random involved, so it will do nothing.
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The second one we can actually verify
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(randomTriggers))).Times(1);
		// The final call should not contain any additional triggers
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
	}

	registerExpanderModule(solimModule, ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT));

	// Upon first attaching a random module, no actual triggers are done
	solimModule.process(Module::ProcessArgs());

	// Now set up the triggers, and verify which one is called in the second process.
	solimRandomModule.m_resetCounters[0] = 1;
	solimRandomModule.m_allCounters[0] = 1;
	solimRandomModule.m_oneCounters[0] = 1;
	solimRandomModule.m_moveCounters[0] = 1;
	solimModule.process(Module::ProcessArgs());

	// Process again, and no triggers should be re-detected
	solimModule.process(Module::ProcessArgs());
}

TEST(SolimTest, ProcessWithRandomWithAllOneAndMoveTriggersShouldUseAllTrigger) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimRandomModule solimRandomModule;
	std::array<RandomTrigger, 8> randomTriggers;
	initializeSolimModule(solimModule);

	randomTriggers = createRandomTriggers(RandomTrigger::NONE);
	randomTriggers[0] = RandomTrigger::ALL;

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(3).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	{
		testing::InSequence seq;
		// The first call will be the first time that there is a random involved, so it will do nothing.
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The second one we can actually verify
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(randomTriggers))).Times(1);
		// The final call should not contain any additional triggers
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
	}

	registerExpanderModule(solimModule, ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT));

	// Upon first attaching a random module, no actual triggers are done
	solimModule.process(Module::ProcessArgs());

	// Now set up the triggers, and verify which one is called in the second process.
	solimRandomModule.m_allCounters[0] = 1;
	solimRandomModule.m_oneCounters[0] = 1;
	solimRandomModule.m_moveCounters[0] = 1;
	solimModule.process(Module::ProcessArgs());

	// Process again, and no triggers should be re-detected
	solimModule.process(Module::ProcessArgs());
}

TEST(SolimTest, ProcessWithRandomWithOneAndMoveTriggersShouldUseOneTrigger) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimRandomModule solimRandomModule;
	std::array<RandomTrigger, 8> randomTriggers;
	initializeSolimModule(solimModule);

	randomTriggers = createRandomTriggers(RandomTrigger::NONE);
	randomTriggers[0] = RandomTrigger::ONE;

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(3).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	{
		testing::InSequence seq;
		// The first call will be the first time that there is a random involved, so it will do nothing.
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The second one we can actually verify
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(randomTriggers))).Times(1);
		// The final call should not contain any additional triggers
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
	}

	registerExpanderModule(solimModule, ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT));

	// Upon first attaching a random module, no actual triggers are done
	solimModule.process(Module::ProcessArgs());

	// Now set up the triggers, and verify which one is called in the second process.
	solimRandomModule.m_oneCounters[0] = 1;
	solimRandomModule.m_moveCounters[0] = 1;
	solimModule.process(Module::ProcessArgs());

	// Process again, and no triggers should be re-detected
	solimModule.process(Module::ProcessArgs());
}

TEST(SolimTest, ProcessShouldUsePolyphonicInputs) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[5];
	SolimOutputModule solimOutputModule[5];
	SolimRandomModule solimRandomModule;
	std::array<RandomTrigger, 8> randomTriggers;
	initializeSolimModule(solimModule);

	randomTriggers = createRandomTriggers(RandomTrigger::NONE);
	randomTriggers[0] = RandomTrigger::ONE;
	randomTriggers[1] = RandomTrigger::MOVE;
	randomTriggers[2] = RandomTrigger::ALL;
	randomTriggers[4] = RandomTrigger::RESET;

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	for (int i = 0; i < 6; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).Times(3).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	}
	{
		testing::InSequence seq;
		// The first call will be the first time that there is a random involved, so it will do nothing.
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(6, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The second one we can actually verify
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(6, testing::Pointee(randomTriggers))).Times(1);
		// The final call should not contain any additional triggers
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(6, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
	}

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[3], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[4], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT)
	});

	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[3], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[4], modelSolimOutput, ExpanderSide::RIGHT)
	});

	// Upon first attaching a random module, no actual triggers are done
	solimModule.process(Module::ProcessArgs());

	// Now set up the triggers, and verify which one is called in the second process.
	solimRandomModule.m_oneCounters[0] = 1;
	solimRandomModule.m_moveCounters[1] = 1;
	solimRandomModule.m_allCounters[2] = 1;
	solimRandomModule.m_resetCounters[4] = 1;
	solimModule.process(Module::ProcessArgs());

	// Process again, and no triggers should be re-detected
	solimModule.process(Module::ProcessArgs());
}

TEST(SolimTest, ProcessShouldDetectRandomAddingAndRemoving) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimRandomModule solimRandomModule;
	std::array<RandomTrigger, 8> randomTriggers;
	initializeSolimModule(solimModule);

	randomTriggers = createRandomTriggers(RandomTrigger::NONE);
	randomTriggers[0] = RandomTrigger::ONE;

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(10).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	{
		testing::InSequence seq;
		// The first call will be without a random module attached
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);
		// The second call will have a random module, but no triggers should be fired yet
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The thrid call will not detect a new trigger
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The third call will fire a trigger
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(randomTriggers))).Times(1);
		// The fourth call will not detect a new trigger
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The fifth call will detect a new trigger
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(randomTriggers))).Times(1);
		// The sixth call will detect that there is no random anymore
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);
		// The seventh call will re-detect the random module, but not fire any triggers
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The eight call will not detect a new trigger
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(createRandomTriggers(RandomTrigger::NONE)))).Times(1);
		// The eight call will fire a trigger again
		EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, testing::Pointee(randomTriggers))).Times(1);
	}

	solimRandomModule.m_oneCounters[0] = 69;

	solimModule.process(Module::ProcessArgs()); // No random module present
	registerExpanderModule(solimModule, ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT));
	solimModule.process(Module::ProcessArgs()); // Detection of random, no trigger
	solimModule.process(Module::ProcessArgs()); // Random still there, but no new triggers
	solimRandomModule.m_oneCounters[0]++;
	solimModule.process(Module::ProcessArgs()); // New trigger
	solimModule.process(Module::ProcessArgs()); // Random still there, but no new triggers
	solimRandomModule.m_oneCounters[0]++;
	solimModule.process(Module::ProcessArgs()); // New trigger
	solimModule.leftExpander.module = nullptr;
	solimRandomModule.m_oneCounters[0]++;
	solimModule.process(Module::ProcessArgs()); // No random anymore
	solimRandomModule.m_oneCounters[0]++;
	registerExpanderModule(solimModule, ExpanderData(solimRandomModule, modelSolimRandom, ExpanderSide::LEFT));
	solimModule.process(Module::ProcessArgs()); // Random present again, but don't fire a trigger
	solimModule.process(Module::ProcessArgs()); // Random still there, but no new trigger
	solimRandomModule.m_oneCounters[0]++;
	solimModule.process(Module::ProcessArgs()); // New trigger
}

TEST(SolimTest, ProcessWithOneInputAndOutputExpanderShouldProcessExpanders) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimOutputModule solimOutputModule;
	SolimInputModule solimInputModule;
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[2];
	SolimValueSet inactiveSolimValueSet[2];
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[0]));
	EXPECT_CALL(*solimCore, getActiveValues(1)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[1]));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[0]));
	EXPECT_CALL(*solimCore, getInactiveValues(1)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[1]));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(2, nullptr)).Times(1);

	registerExpanderModule(solimModule, ExpanderData(solimInputModule, modelSolimInput, ExpanderSide::LEFT));
	registerExpanderModule(solimModule, ExpanderData(solimOutputModule, modelSolimOutput, ExpanderSide::RIGHT));

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(3.24f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(4.62f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + .1f });
	}
	for (int i = 0; i < 4; i++) {
		setInputVoltages(solimInputModule.inputs[SolimInputModule::IN_INPUTS + i], { (float) i + .2f });
	}

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(inactiveSolimValueSet[0].inputValueCount, 8);
	EXPECT_EQ(inactiveSolimValueSet[0].lowerLimit, 3.24f);
	EXPECT_EQ(inactiveSolimValueSet[0].upperLimit, 4.62f);
	EXPECT_EQ(inactiveSolimValueSet[0].sort, -1);
	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet[0].inputValues[i].value, i + .1f, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet[0].inputValues[i].addOctave, SolimValue::AddOctave::NONE);
		EXPECT_EQ(inactiveSolimValueSet[0].inputValues[i].replaceOriginal, false);
	}

	EXPECT_EQ(inactiveSolimValueSet[1].inputValueCount, 4);
	EXPECT_EQ(inactiveSolimValueSet[1].lowerLimit, 3.24f);
	EXPECT_EQ(inactiveSolimValueSet[1].upperLimit, 4.62f);
	EXPECT_EQ(inactiveSolimValueSet[1].sort, -1);
	for (int i = 0; i < 4; i++) {
		EXPECT_NEAR(inactiveSolimValueSet[1].inputValues[i].value, i + .2f, 0.00001f);
		EXPECT_EQ(inactiveSolimValueSet[1].inputValues[i].addOctave, SolimValue::AddOctave::NONE);
		EXPECT_EQ(inactiveSolimValueSet[1].inputValues[i].replaceOriginal, false);
	}
}

TEST(SolimTest, ProcessWithEightInputAndOutputExpanderShouldProcessExpanders) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[7];
	SolimOutputModule solimOutputModule[7];
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[8];
	SolimValueSet inactiveSolimValueSet[8];
	for (int i = 0; i < 8; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(8, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[3], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[4], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[5], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[6], modelSolimInput, ExpanderSide::LEFT)
	});
	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[3], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[4], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[5], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[6], modelSolimOutput, ExpanderSide::RIGHT)
	});

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(3.24f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(4.62f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i + .1f });
	}
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < i + 1; j++) {
			setInputVoltages(solimInputModule[i].inputs[SolimInputModule::IN_INPUTS + j], { (float) i + (.1f * (j + 1)) });
		}
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(inactiveSolimValueSet[i].lowerLimit, 3.24f);
		EXPECT_EQ(inactiveSolimValueSet[i].upperLimit, 4.62f);
		EXPECT_EQ(inactiveSolimValueSet[i].sort, -1);
		for (int j = 0; j < inactiveSolimValueSet[i].inputValueCount; j++) {
			EXPECT_EQ(inactiveSolimValueSet[i].inputValues[j].addOctave, SolimValue::AddOctave::NONE);
			EXPECT_EQ(inactiveSolimValueSet[i].inputValues[j].replaceOriginal, false);
		}
	}

	EXPECT_EQ(inactiveSolimValueSet[0].inputValueCount, 8);
	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet[0].inputValues[i].value, i + .1f, 0.00001f);
	}

	EXPECT_EQ(inactiveSolimValueSet[1].inputValueCount, 1);
	EXPECT_NEAR(inactiveSolimValueSet[1].inputValues[0].value, .1f, 0.00001f);

	EXPECT_EQ(inactiveSolimValueSet[2].inputValueCount, 2);
	EXPECT_NEAR(inactiveSolimValueSet[2].inputValues[0].value, 1.1f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[2].inputValues[1].value, 1.2f, 0.00001f);

	EXPECT_EQ(inactiveSolimValueSet[3].inputValueCount, 3);
	EXPECT_NEAR(inactiveSolimValueSet[3].inputValues[0].value, 2.1f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[3].inputValues[1].value, 2.2f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[3].inputValues[2].value, 2.3f, 0.00001f);

	EXPECT_EQ(inactiveSolimValueSet[4].inputValueCount, 4);
	EXPECT_NEAR(inactiveSolimValueSet[4].inputValues[0].value, 3.1f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[4].inputValues[1].value, 3.2f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[4].inputValues[2].value, 3.3f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[4].inputValues[3].value, 3.4f, 0.00001f);

	EXPECT_EQ(inactiveSolimValueSet[5].inputValueCount, 5);
	EXPECT_NEAR(inactiveSolimValueSet[5].inputValues[0].value, 4.1f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[5].inputValues[1].value, 4.2f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[5].inputValues[2].value, 4.3f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[5].inputValues[3].value, 4.4f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[5].inputValues[4].value, 4.5f, 0.00001f);

	EXPECT_EQ(inactiveSolimValueSet[6].inputValueCount, 6);
	EXPECT_NEAR(inactiveSolimValueSet[6].inputValues[0].value, 5.1f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[6].inputValues[1].value, 5.2f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[6].inputValues[2].value, 5.3f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[6].inputValues[3].value, 5.4f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[6].inputValues[4].value, 5.5f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[6].inputValues[5].value, 5.6f, 0.00001f);

	EXPECT_EQ(inactiveSolimValueSet[7].inputValueCount, 7);
	EXPECT_NEAR(inactiveSolimValueSet[7].inputValues[0].value, 6.1f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[7].inputValues[1].value, 6.2f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[7].inputValues[2].value, 6.3f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[7].inputValues[3].value, 6.4f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[7].inputValues[4].value, 6.5f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[7].inputValues[5].value, 6.6f, 0.00001f);
	EXPECT_NEAR(inactiveSolimValueSet[7].inputValues[6].value, 6.7f, 0.00001f);
}

TEST(SolimTest, ProcessWithMoreInputThenOutputExpandersShouldUseLowestCount) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[6];
	SolimOutputModule solimOutputModule[3];
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[4];
	SolimValueSet inactiveSolimValueSet[4];
	for (int i = 0; i < 4; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(4, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[3], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[4], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[5], modelSolimInput, ExpanderSide::LEFT),
	});
	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
	});

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(3.24f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(4.62f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i });
	}
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 4; j++) {
			setInputVoltages(solimInputModule[i].inputs[SolimInputModule::IN_INPUTS + j], { (float) i + j });
		}
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 4; i++) {
		EXPECT_EQ(inactiveSolimValueSet[i].lowerLimit, 3.24f);
		EXPECT_EQ(inactiveSolimValueSet[i].upperLimit, 4.62f);
		EXPECT_EQ(inactiveSolimValueSet[i].sort, -1);
		for (int j = 0; j < inactiveSolimValueSet[i].inputValueCount; j++) {
			EXPECT_EQ(inactiveSolimValueSet[i].inputValues[j].addOctave, SolimValue::AddOctave::NONE);
			EXPECT_EQ(inactiveSolimValueSet[i].inputValues[j].replaceOriginal, false);
		}
	}

	EXPECT_EQ(inactiveSolimValueSet[0].inputValueCount, 8);
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(inactiveSolimValueSet[0].inputValues[i].value, i);
	}

	for (int i = 0; i < 3; i++) {
		EXPECT_EQ(inactiveSolimValueSet[i + 1].inputValueCount, 4);
		for (int j = 0; j < 3; j++) {
			EXPECT_EQ(inactiveSolimValueSet[i + 1].inputValues[j].value, i + j);
		}
	}
}

TEST(SolimTest, ProcessWithMoreOutputThenInputExpandersShouldUseLowestCount) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[3];
	SolimOutputModule solimOutputModule[6];
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[4];
	SolimValueSet inactiveSolimValueSet[4];
	for (int i = 0; i < 4; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(4, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
	});
	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[3], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[4], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[5], modelSolimOutput, ExpanderSide::RIGHT),
	});

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(3.24f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(4.62f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i });
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			setInputVoltages(solimInputModule[i].inputs[SolimInputModule::IN_INPUTS + j], { (float) i + j });
		}
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 4; i++) {
		EXPECT_EQ(inactiveSolimValueSet[i].lowerLimit, 3.24f);
		EXPECT_EQ(inactiveSolimValueSet[i].upperLimit, 4.62f);
		EXPECT_EQ(inactiveSolimValueSet[i].sort, -1);
		for (int j = 0; j < inactiveSolimValueSet[i].inputValueCount; j++) {
			EXPECT_EQ(inactiveSolimValueSet[i].inputValues[j].addOctave, SolimValue::AddOctave::NONE);
			EXPECT_EQ(inactiveSolimValueSet[i].inputValues[j].replaceOriginal, false);
		}
	}

	EXPECT_EQ(inactiveSolimValueSet[0].inputValueCount, 8);
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(inactiveSolimValueSet[0].inputValues[i].value, i);
	}

	for (int i = 0; i < 3; i++) {
		EXPECT_EQ(inactiveSolimValueSet[i + 1].inputValueCount, 4);
		for (int j = 0; j < 3; j++) {
			EXPECT_EQ(inactiveSolimValueSet[i + 1].inputValues[j].value, i + j);
		}
	}
}

TEST(SolimTest, ProcessWithEightInputAndOutputExpanderShouldUseNonPolyphonicSolimInputs) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[7];
	SolimOutputModule solimOutputModule[7];
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[8];
	SolimValueSet inactiveSolimValueSet[8];
	for (int i = 0; i < 8; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(8, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[3], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[4], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[5], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[6], modelSolimInput, ExpanderSide::LEFT)
	});
	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[3], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[4], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[5], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[6], modelSolimOutput, ExpanderSide::RIGHT)
	});

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(3.24f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(4.62f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	setInputVoltages(solimModule.inputs[SolimModule::IN_LOWER_LIMIT], { -1.24f });
	setInputVoltages(solimModule.inputs[SolimModule::IN_UPPER_LIMIT], { 1.62f });
	setInputVoltages(solimModule.inputs[SolimModule::IN_SORT], { 1.2f });

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i });
	}
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < i + 1; j++) {
			setInputVoltages(solimInputModule[i].inputs[SolimInputModule::IN_INPUTS + j], { (float) i + j });
		}
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(inactiveSolimValueSet[i].lowerLimit, -1.24f);
		EXPECT_EQ(inactiveSolimValueSet[i].upperLimit, 1.62f);
		EXPECT_EQ(inactiveSolimValueSet[i].sort, 1);
	}
}

TEST(SolimTest, ProcessWithEightInputAndOutputExpanderShouldUsePolyphonicSolimInputs) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[7];
	SolimOutputModule solimOutputModule[7];
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[8];
	SolimValueSet inactiveSolimValueSet[8];
	for (int i = 0; i < 8; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(8, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[3], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[4], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[5], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[6], modelSolimInput, ExpanderSide::LEFT)
	});
	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[3], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[4], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[5], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[6], modelSolimOutput, ExpanderSide::RIGHT)
	});

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(3.24f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(4.62f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	setInputVoltages(solimModule.inputs[SolimModule::IN_LOWER_LIMIT], { -1.1f, -1.2f, -1.3f, -1.4f, -1.5f, -1.6f, -1.7f, -1.8f });
	setInputVoltages(solimModule.inputs[SolimModule::IN_UPPER_LIMIT], { 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f });
	setInputVoltages(solimModule.inputs[SolimModule::IN_SORT], { -1.1f, 0.f, 1.1f, -1.2f, 0, .5f, -.5f, 0 });

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i });
	}
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < i + 1; j++) {
			setInputVoltages(solimInputModule[i].inputs[SolimInputModule::IN_INPUTS + j], { (float) i + j });
		}
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_NEAR(inactiveSolimValueSet[i].lowerLimit, -1.f - (.1f * (i + 1)), 0.0001f);
		EXPECT_NEAR(inactiveSolimValueSet[i].upperLimit, 1.f + (.1f * (i + 1)), 0.0001f);
		EXPECT_EQ(inactiveSolimValueSet[i].sort, (i % 3) - 1);
	}
}

TEST(SolimTest, ProcessWithEightInputAndOutputExpanderShouldUsePolyphonicSolimInputsWithFallbackToLastChannel) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[7];
	SolimOutputModule solimOutputModule[7];
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[8];
	SolimValueSet inactiveSolimValueSet[8];
	for (int i = 0; i < 8; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(8, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[3], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[4], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[5], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[6], modelSolimInput, ExpanderSide::LEFT)
	});
	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[3], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[4], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[5], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[6], modelSolimOutput, ExpanderSide::RIGHT)
	});

	solimModule.params[SolimModule::PARAM_LOWER_LIMIT].setValue(3.24f);
	solimModule.params[SolimModule::PARAM_UPPER_LIMIT].setValue(4.62f);
	solimModule.params[SolimModule::PARAM_SORT].setValue(-1.2f);

	// Only provision 4 channels on each input, causing the module to use the last value for the remaining inputs.
	setInputVoltages(solimModule.inputs[SolimModule::IN_LOWER_LIMIT], { -1.1f, -1.2f, -1.3f, -1.4f });
	setInputVoltages(solimModule.inputs[SolimModule::IN_UPPER_LIMIT], { 1.1f, 1.2f, 1.3f, 1.4f });
	setInputVoltages(solimModule.inputs[SolimModule::IN_SORT], { -1.1f, 0.f, 1.1f, -1.2f });

	for (int i = 0; i < 8; i++) {
		setInputVoltages(solimModule.inputs[SolimModule::IN_INPUTS + i], { (float) i });
	}
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < i + 1; j++) {
			setInputVoltages(solimInputModule[i].inputs[SolimInputModule::IN_INPUTS + j], { (float) i + j });
		}
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 4; i++) {
		EXPECT_NEAR(inactiveSolimValueSet[i].lowerLimit, -1.f - (.1f * (i + 1)), 0.0001f);
		EXPECT_NEAR(inactiveSolimValueSet[i].upperLimit, 1.f + (.1f * (i + 1)), 0.0001f);
		EXPECT_EQ(inactiveSolimValueSet[i].sort, (i % 3) - 1);
	}

	for (int i = 4; i < 8; i++) {
		EXPECT_EQ(inactiveSolimValueSet[i].lowerLimit, -1.4f);
		EXPECT_EQ(inactiveSolimValueSet[i].upperLimit, 1.4f);
		EXPECT_EQ(inactiveSolimValueSet[i].sort, -1);
	}
}

TEST(SolimTest, ProcessWithoutExpandersAndEightResultsShouldWriteResults) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	activeSolimValueSet.resultValueCount = 8;
	for (int i = 0; i < 8; i++) {
		activeSolimValueSet.resultValues[i] = 1.1f * i;
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(solimModule.outputs[SolimModule::OutputId::OUT_OUTPUTS + i].getVoltage(), 1.1f * i);
		EXPECT_EQ(solimModule.lights[SolimModule::LightId::OUT_LIGHTS + i].getBrightness(), 1.f);
	}
}

TEST(SolimTest, ProcessWithoutExpandersAndFourResultsShouldWriteResults) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet;
	SolimValueSet inactiveSolimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).Times(1).WillRepeatedly(testing::ReturnRef(activeSolimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(1);

	// Change the value of all of the outputs, so we can verify that the unused outputs are set to 0
	for (int i = 0; i < 8; i++) {
		solimModule.outputs[SolimModule::OutputId::OUT_OUTPUTS + i].setVoltage(6.9f);
		solimModule.lights[SolimModule::LightId::OUT_LIGHTS + i].setBrightness(4.2f);
	}

	activeSolimValueSet.resultValueCount = 4;
	for (int i = 0; i < 4; i++) {
		activeSolimValueSet.resultValues[i] = 1.1f * i;
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 0; i < 4; i++) {
		EXPECT_EQ(solimModule.outputs[SolimModule::OutputId::OUT_OUTPUTS + i].getVoltage(), 1.1f * i);
		EXPECT_EQ(solimModule.lights[SolimModule::LightId::OUT_LIGHTS + i].getBrightness(), 1.f);
	}

	for (int i = 4; i < 8; i++) {
		EXPECT_EQ(solimModule.outputs[SolimModule::OutputId::OUT_OUTPUTS + i].getVoltage(), 0.f);
		EXPECT_EQ(solimModule.lights[SolimModule::LightId::OUT_LIGHTS + i].getBrightness(), 0.f);
	}
}

TEST(SolimTest, ProcessWithEightInputAndOutputExpandersShouldWriteResults) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[7];
	SolimOutputModule solimOutputModule[7];
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[8];
	SolimValueSet inactiveSolimValueSet[8];
	for (int i = 0; i < 8; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(8, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[3], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[4], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[5], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[6], modelSolimInput, ExpanderSide::LEFT)
	});
	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[3], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[4], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[5], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[6], modelSolimOutput, ExpanderSide::RIGHT)
	});

	// Change the value of all of the outputs, so we can verify that the unused outputs are set to 0
	for (int i = 0; i < 8; i++) {
		solimModule.outputs[SolimModule::OutputId::OUT_OUTPUTS + i].setVoltage(6.9f);
		solimModule.lights[SolimModule::LightId::OUT_LIGHTS + i].setBrightness(4.2f);
	}
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 8; j++) {
			solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].setVoltage(6.9f);
			solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].setBrightness(4.2f);
		}
	}

	// Set the result data
	for (int i = 0; i < 8; i++) {
		activeSolimValueSet[i].resultValueCount = i + 1;
		for (int j = 0; j < i + 1; j++) {
			activeSolimValueSet[i].resultValues[j] = (i + 1) + (j * 0.1f);
		}
	}

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(solimModule.outputs[SolimModule::OutputId::OUT_OUTPUTS].getVoltage(), 1.f);
	EXPECT_EQ(solimModule.lights[SolimModule::LightId::OUT_LIGHTS].getBrightness(), 1.f);
	for (int i = 1; i < 8; i++) {
		EXPECT_EQ(solimModule.outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + i].getVoltage(), 0.f);
		EXPECT_EQ(solimModule.lights[SolimOutputModule::LightId::OUT_LIGHTS + i].getBrightness(), 0.f);
	}

	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 8; j++) {
			if (j < i + 2) { // (i + 2) because the first iteration was on the output of the solim module itself, so the output modules started being populated in the second loop
				EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].getVoltage(), (i + 2) + (j * 0.1f));
				EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].getBrightness(), 1.f);
			} else {
				EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].getVoltage(), 0.f);
				EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].getBrightness(), 0.f);
			}
		}
	}
}

TEST(SolimTest, ProcessWithExcessOutputExpandersShouldDimLightsOnWriteResults) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[2];
	SolimOutputModule solimOutputModule[6];
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[3];
	SolimValueSet inactiveSolimValueSet[3];
	for (int i = 0; i < 3; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(3, nullptr)).Times(1);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
	});
	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[3], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[4], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[5], modelSolimOutput, ExpanderSide::RIGHT)
	});

	// Change the value and lights of all of the outputs, so we can verify that the unused outputs are set to 0
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 8; j++) {
			solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].setVoltage(6.9f);
			solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].setBrightness(4.2f);
		}
	}

	// Set the result data
	for (int i = 0; i < 3; i++) {
		activeSolimValueSet[i].resultValueCount = 8;
		for (int j = 0; j < 8; j++) {
			activeSolimValueSet[i].resultValues[j] = 3.14f;
		}
	}

	solimModule.process(Module::ProcessArgs());

	for (int i = 3; i < 6; i++) {
		for (int j = 0; j < 8; j++) {
			EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].getVoltage(), 0.f);
			EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].getBrightness(), 0.f);
		}
	}
}

TEST(SolimTest, ProcessWithSomePolyphonicOutputExpandersShouldCreatePolyphonicOutput) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[7];
	SolimOutputModule solimOutputModule[7];
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[8];
	SolimValueSet inactiveSolimValueSet[8];
	for (int i = 0; i < 8; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(8, nullptr)).Times(1);

	solimOutputModule[2].setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);
	solimOutputModule[5].setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[3], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[4], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[5], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[6], modelSolimInput, ExpanderSide::LEFT)
	});
	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[3], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[4], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[5], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[6], modelSolimOutput, ExpanderSide::RIGHT)
	});

	// Change the value of all of the outputs, so we can verify that the unused outputs are set to 0
	solimModule.outputs[SolimModule::OutputId::OUT_OUTPUTS].channels = 12;
	for (int i = 0; i < 8; i++) {
		solimModule.outputs[SolimModule::OutputId::OUT_OUTPUTS + i].setVoltage(6.9f);
		solimModule.lights[SolimModule::LightId::OUT_LIGHTS + i].setBrightness(4.2f);
	}
	for (int i = 0; i < 7; i++) {
		solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS].channels = 12;
		for (int j = 0; j < 8; j++) {
			solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].setVoltage(6.9f);
			solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].setBrightness(4.2f);
		}
	}

	// Set the result data
	for (int i = 0; i < 8; i++) {
		activeSolimValueSet[i].resultValueCount = i + 1;
		for (int j = 0; j < i + 1; j++) {
			activeSolimValueSet[i].resultValues[j] = (i + 1) + (j * 0.1f);
		}
	}

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(solimModule.outputs[SolimModule::OutputId::OUT_OUTPUTS].getVoltage(), 1.f);
	EXPECT_EQ(solimModule.lights[SolimModule::LightId::OUT_LIGHTS].getBrightness(), 1.f);
	EXPECT_EQ(solimModule.lights[SolimModule::LightId::OUT_POLYPHONIC_LIGHT].getBrightness(), 0.f);
	EXPECT_EQ(solimModule.outputs[SolimOutputModule::OutputId::OUT_OUTPUTS].getChannels(), 1);
	for (int i = 1; i < 8; i++) {
		EXPECT_EQ(solimModule.outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + i].getVoltage(), 0.f);
		EXPECT_EQ(solimModule.lights[SolimOutputModule::LightId::OUT_LIGHTS + i].getBrightness(), 0.f);
	}

	for (int i = 0; i < 7; i++) {
		if (i != 2 && i != 5) {
			// Check the monophonic outputs
			EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_POLYPHONIC_LIGHT].getBrightness(), 0.f);
			EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS].getChannels(), 1);
			for (int j = 0; j < 8; j++) {
				if (j < i + 2) { // (i + 2) because the first iteration was on the output of the solim module itself, so the output modules started being populated in the second loop
					EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].getVoltage(), (i + 2) + (j * 0.1f));
					EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].getBrightness(), 1.f);
				} else {
					EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].getVoltage(), 0.f);
					EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].getBrightness(), 0.f);
				}
			}
		} else {
			// Check the polyphonic outputs
			EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_POLYPHONIC_LIGHT].getBrightness(), 1.f);
			EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS].getChannels(), i + 2); // There should be multiple channels instead of multiple active outputs
			// Check the values that are assigned to the different channels on the first output
			for (int j = 0; j < i + 2; j++) {
				EXPECT_EQ(solimOutputModule[i].outputs[0].getVoltage(j), (i + 2) + (j * 0.1f));
			}
			// The other 7 outputs should be disabled
			for (int j = 1; j < 8; j++) {
				EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].getVoltage(), 0.f);
				EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].getBrightness(), 0.f);
			}
		}
	}
}

TEST(SolimTest, ProcessWithSomePolyphonicOutputExpandersAndPolyphonicMainModuleShouldCreatePolyphonicOutput) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	SolimInputModule solimInputModule[7];
	SolimOutputModule solimOutputModule[7];
	initializeSolimModule(solimModule);

	SolimValueSet activeSolimValueSet[8];
	SolimValueSet inactiveSolimValueSet[8];
	for (int i = 0; i < 8; i++) {
		EXPECT_CALL(*solimCore, getActiveValues(i)).Times(1).WillRepeatedly(testing::ReturnRef(activeSolimValueSet[i]));
		EXPECT_CALL(*solimCore, getInactiveValues(i)).WillRepeatedly(testing::ReturnRef(inactiveSolimValueSet[i]));
	}
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(8, nullptr)).Times(1);

	solimModule.setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);
	solimOutputModule[2].setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);
	solimOutputModule[5].setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);

	registerExpanderModules(solimModule, {
		ExpanderData(solimInputModule[0], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[1], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[2], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[3], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[4], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[5], modelSolimInput, ExpanderSide::LEFT),
		ExpanderData(solimInputModule[6], modelSolimInput, ExpanderSide::LEFT)
	});
	registerExpanderModules(solimModule, {
		ExpanderData(solimOutputModule[0], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[1], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[2], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[3], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[4], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[5], modelSolimOutput, ExpanderSide::RIGHT),
		ExpanderData(solimOutputModule[6], modelSolimOutput, ExpanderSide::RIGHT)
	});

	// Change the value of all of the outputs, so we can verify that the unused outputs are set to 0
	for (int i = 0; i < 8; i++) {
		solimModule.outputs[SolimModule::OutputId::OUT_OUTPUTS + i].channels = 12;
		solimModule.outputs[SolimModule::OutputId::OUT_OUTPUTS + i].setVoltage(6.9f);
		solimModule.lights[SolimModule::LightId::OUT_LIGHTS + i].setBrightness(4.2f);
	}
	for (int i = 0; i < 7; i++) {
		solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS].channels = 12;
		for (int j = 0; j < 8; j++) {
			solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].setVoltage(6.9f);
			solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].setBrightness(4.2f);
		}
	}

	// Set the result data
	activeSolimValueSet[0].resultValueCount = 4;
	activeSolimValueSet[0].resultValues = { 3.2f, 2.2f, 1.2f, 0.2f };
	for (int i = 1; i < 8; i++) {
		activeSolimValueSet[i].resultValueCount = i + 1;
		for (int j = 0; j < i + 1; j++) {
			activeSolimValueSet[i].resultValues[j] = (i + 1) + (j * 0.1f);
		}
	}

	solimModule.process(Module::ProcessArgs());

	EXPECT_EQ(solimModule.lights[SolimModule::LightId::OUT_LIGHTS].getBrightness(), 0.f);
	EXPECT_EQ(solimModule.lights[SolimModule::LightId::OUT_POLYPHONIC_LIGHT].getBrightness(), 1.f);
	EXPECT_EQ(solimModule.outputs[SolimOutputModule::OutputId::OUT_OUTPUTS].getChannels(), 4);
	for (int i = 0; i < 4; i++) {
		EXPECT_NEAR(solimModule.outputs[SolimOutputModule::OutputId::OUT_OUTPUTS].getVoltage(i), 3.2f - i, 0.00001f);
	}
	for (int i = 1; i < 8; i++) {
		EXPECT_EQ(solimModule.outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + i].getVoltage(), 0.f);
		EXPECT_EQ(solimModule.lights[SolimOutputModule::LightId::OUT_LIGHTS + i].getBrightness(), 0.f);
	}

	for (int i = 0; i < 7; i++) {
		if (i != 2 && i != 5) {
			// Check the monophonic outputs
			EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_POLYPHONIC_LIGHT].getBrightness(), 0.f);
			EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS].getChannels(), 1);
			for (int j = 0; j < 8; j++) {
				if (j < i + 2) { // (i + 2) because the first iteration was on the output of the solim module itself, so the output modules started being populated in the second loop
					EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].getVoltage(), (i + 2) + (j * 0.1f));
					EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].getBrightness(), 1.f);
				} else {
					EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].getVoltage(), 0.f);
					EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].getBrightness(), 0.f);
				}
			}
		} else {
			// Check the polyphonic outputs
			EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_POLYPHONIC_LIGHT].getBrightness(), 1.f);
			EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS].getChannels(), i + 2); // There should be multiple channels instead of multiple active outputs
			// Check the values that are assigned to the different channels on the first output
			for (int j = 0; j < i + 2; j++) {
				EXPECT_EQ(solimOutputModule[i].outputs[0].getVoltage(j), (i + 2) + (j * 0.1f));
			}
			// The other 7 outputs should be disabled
			for (int j = 1; j < 8; j++) {
				EXPECT_EQ(solimOutputModule[i].outputs[SolimOutputModule::OutputId::OUT_OUTPUTS + j].getVoltage(), 0.f);
				EXPECT_EQ(solimOutputModule[i].lights[SolimOutputModule::LightId::OUT_LIGHTS + j].getBrightness(), 0.f);
			}
		}
	}
}

TEST(SolimTest, WithAudioProcessRateShouldProcessEveryTime) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet solimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(solimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).WillRepeatedly(testing::ReturnRef(solimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(80); // Every process call below should trigger a process on the core

	solimModule.setProcessRate(SolimModule::ProcessRate::AUDIO);
	ASSERT_EQ(solimModule.getProcessRate(), SolimModule::ProcessRate::AUDIO);
	Module::SampleRateChangeEvent sampleRateChangeEvent;
	sampleRateChangeEvent.sampleRate = 48000;
	solimModule.onSampleRateChange(sampleRateChangeEvent);

	for (int i = 0; i < 80; i++) {
		solimModule.process(Module::ProcessArgs());
	}
}

TEST(SolimTest, WithDividedProcessRateShouldNotProcessEveryTime) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet solimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(solimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).WillRepeatedly(testing::ReturnRef(solimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(10); // At sample rate 48K, only one in eight calls should result in a core process

	solimModule.setProcessRate(SolimModule::ProcessRate::DIVIDED);
	ASSERT_EQ(solimModule.getProcessRate(), SolimModule::ProcessRate::DIVIDED);
	Module::SampleRateChangeEvent sampleRateChangeEvent;
	sampleRateChangeEvent.sampleRate = 48000;
	solimModule.onSampleRateChange(sampleRateChangeEvent);

	for (int i = 0; i < 80; i++) {
		solimModule.process(Module::ProcessArgs());
	}
}

TEST(SolimTest, WithDividedProcessRateShouldHandleSampleRateChange) {
	MockSolimCore* solimCore = new MockSolimCore();
	SolimModule solimModule(solimCore);
	initializeSolimModule(solimModule);

	SolimValueSet solimValueSet;
	EXPECT_CALL(*solimCore, getActiveValues(0)).WillRepeatedly(testing::ReturnRef(solimValueSet));
	EXPECT_CALL(*solimCore, getInactiveValues(0)).WillRepeatedly(testing::ReturnRef(solimValueSet));
	EXPECT_CALL(*solimCore, processAndActivateInactiveValues(1, nullptr)).Times(30); // 10 invocations from the 48K sampleRate and 20 more invocations from the 24K one

	solimModule.setProcessRate(SolimModule::ProcessRate::DIVIDED);
	ASSERT_EQ(solimModule.getProcessRate(), SolimModule::ProcessRate::DIVIDED);

	// First run 80 times at sample rate 48000
	Module::SampleRateChangeEvent sampleRateChangeEvent;
	sampleRateChangeEvent.sampleRate = 48000;
	solimModule.onSampleRateChange(sampleRateChangeEvent);
	for (int i = 0; i < 80; i++) {
		solimModule.process(Module::ProcessArgs());
	}

	// Now run 80 more times at sample rate 24000
	sampleRateChangeEvent.sampleRate = 24000;
	solimModule.onSampleRateChange(sampleRateChangeEvent);
	for (int i = 0; i < 80; i++) {
		solimModule.process(Module::ProcessArgs());
	}
}

TEST(SolimTest, ShouldDefaultToMonophonicOutputMode) {
	SolimModule solimModule;

	EXPECT_EQ(solimModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);
	json_t* jsonData = solimModule.dataToJson();
	checkJsonSolimOutputMode(solimModule.dataToJson(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);

	delete jsonData;
}

TEST(SolimTest, ShouldSwitchToPolyphonicModeFromJsonData) {
	SolimModule solimModule;

	json_t *jsonData = json_object();
	json_object_set_new(jsonData, "ntSolimOutputMode", json_integer(static_cast<SolimOutputMode>(SolimOutputMode::OUTPUT_MODE_POLYPHONIC)));
	solimModule.dataFromJson(jsonData);

	EXPECT_EQ(solimModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_POLYPHONIC);

	delete jsonData;
}

TEST(SolimTest, ShouldSwitchToMonophonicModeFromJsonData) {
	SolimModule solimModule;

	json_t *jsonData = json_object();
	json_object_set_new(jsonData, "ntSolimOutputMode", json_integer(static_cast<SolimOutputMode>(SolimOutputMode::OUTPUT_MODE_MONOPHONIC)));
	solimModule.dataFromJson(jsonData);
	EXPECT_EQ(solimModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);

	solimModule.setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);

	solimModule.dataFromJson(jsonData);
	EXPECT_EQ(solimModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);

	delete jsonData;
}

TEST(SolimTest, ShouldPersistPolyphonicOutputMode) {
	SolimModule solimModule;

	solimModule.setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);
	EXPECT_EQ(solimModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_POLYPHONIC);
	json_t* jsonData = solimModule.dataToJson();
	checkJsonSolimOutputMode(solimModule.dataToJson(), SolimOutputMode::OUTPUT_MODE_POLYPHONIC);

	delete jsonData;
}

TEST(SolimTest, ShouldPersistMonophonicOutputMode) {
	SolimModule solimModule;

	solimModule.setOutputMode(SolimOutputMode::OUTPUT_MODE_POLYPHONIC);
	solimModule.setOutputMode(SolimOutputMode::OUTPUT_MODE_MONOPHONIC);
	EXPECT_EQ(solimModule.getOutputMode(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);
	json_t* jsonData = solimModule.dataToJson();
	checkJsonSolimOutputMode(solimModule.dataToJson(), SolimOutputMode::OUTPUT_MODE_MONOPHONIC);

	delete jsonData;
}

void checkJsonSolimProcessRate(json_t* jsonData, SolimModule::ProcessRate expectedProcessRate) {
	EXPECT_TRUE(json_is_object(jsonData));

	json_t* jsonSolimProcessRate = json_object_get(jsonData, "ntSolimProcessRate");
	EXPECT_TRUE(json_is_integer(jsonSolimProcessRate));

	EXPECT_EQ(json_integer_value(jsonSolimProcessRate), static_cast<int>(expectedProcessRate));
}

TEST(SolimTest, ShouldDefaultToAudioProcessRate) {
	SolimModule solimModule;

	EXPECT_EQ(solimModule.getProcessRate(), SolimModule::ProcessRate::AUDIO);
	json_t* jsonData = solimModule.dataToJson();
	checkJsonSolimProcessRate(solimModule.dataToJson(), SolimModule::ProcessRate::AUDIO);

	delete jsonData;
}

TEST(SolimTest, ShouldSwitchToDividedModeFromJsonData) {
	SolimModule solimModule;

	json_t *jsonData = json_object();
	json_object_set_new(jsonData, "ntSolimProcessRate", json_integer(static_cast<SolimModule::ProcessRate>(SolimModule::ProcessRate::DIVIDED)));
	solimModule.dataFromJson(jsonData);

	EXPECT_EQ(solimModule.getProcessRate(), SolimModule::ProcessRate::DIVIDED);

	delete jsonData;
}

TEST(SolimTest, ShouldSwitchToAudioModeFromJsonData) {
	SolimModule solimModule;

	json_t *jsonData = json_object();
	json_object_set_new(jsonData, "ntSolimProcessRate", json_integer(static_cast<SolimModule::ProcessRate>(SolimModule::ProcessRate::AUDIO)));
	solimModule.dataFromJson(jsonData);
	EXPECT_EQ(solimModule.getProcessRate(), SolimModule::ProcessRate::AUDIO);

	solimModule.setProcessRate(SolimModule::ProcessRate::DIVIDED);

	solimModule.dataFromJson(jsonData);
	EXPECT_EQ(solimModule.getProcessRate(), SolimModule::ProcessRate::AUDIO);

	delete jsonData;
}

TEST(SolimTest, ShouldPersistDividedProcessRate) {
	SolimModule solimModule;

	solimModule.setProcessRate(SolimModule::ProcessRate::DIVIDED);
	EXPECT_EQ(solimModule.getProcessRate(), SolimModule::ProcessRate::DIVIDED);
	json_t* jsonData = solimModule.dataToJson();
	checkJsonSolimProcessRate(solimModule.dataToJson(), SolimModule::ProcessRate::DIVIDED);

	delete jsonData;
}

TEST(SolimTest, ShouldPersistAudioProcessRate) {
	SolimModule solimModule;

	solimModule.setProcessRate(SolimModule::ProcessRate::DIVIDED);
	solimModule.setProcessRate(SolimModule::ProcessRate::AUDIO);
	EXPECT_EQ(solimModule.getProcessRate(), SolimModule::ProcessRate::AUDIO);
	json_t* jsonData = solimModule.dataToJson();
	checkJsonSolimProcessRate(solimModule.dataToJson(), SolimModule::ProcessRate::AUDIO);

	delete jsonData;
}
