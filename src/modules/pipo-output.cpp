#include "modules/pipo-output.hpp"
#include "modules/pipo-input.hpp"
#include "components/ntport.hpp"
#include "components/lights.hpp"
#include "components/ntknob.hpp"

extern Model* modelPipoInput;
extern Model* modelPipoOutput;


PipoOutputModule::PipoOutputModule() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < 8; i++) {
		configOutput(OUT_OUTPUTS + i, string::f("Output %d", i + 1));

		ParamQuantity* pq = configParam(PARAM_OUTPUT_CHANNELS + i, 1.f, 16.f, 1.f, string::f("Output %d channels", i + 1));
		pq->snapEnabled = true;
		pq->smoothEnabled = false;
	}
}

void PipoOutputModule::process(const ProcessArgs& args) {
	// Only the Pipo Output module that has a Pipo Input module on its left will do processing
	if ((getLeftExpander().module != nullptr) && (getLeftExpander().module->getModel() == modelPipoInput)) {
		// Build up the list of output instances that are in the chain, light their LEDs and calculate the total number of input voltages needed
		resetProcessingData();

		// Loop through the outputs until there are none left
		while (moveToNextOutput()) {
			if (moveToNextInput()) {
				// If there is another input voltage available, assign it to the current output channel
				m_processingData.currentOutputModule->outputs[OutputId::OUT_OUTPUTS + m_processingData.outputIndex].setVoltage(
					m_processingData.currentInputModule->inputs[PipoInputModule::InputId::IN_INPUTS + m_processingData.inputIndex].getVoltage(m_processingData.inputChannelIndex),
					m_processingData.outputChannelIndex);
			} else {
				// There are no more inputs, so set the output channel to 0V instead
				m_processingData.currentOutputModule->outputs[OutputId::OUT_OUTPUTS + m_processingData.outputIndex].setVoltage(0.f, m_processingData.outputChannelIndex);
			}
		}
	}
}

void PipoOutputModule::draw(const widget::Widget::DrawArgs& args) {
	// Look to the left to see if there is a Pipo Input module there (with possibility other Pipo Output modules in between)
	// If not, we need to turn off our connected LED
	Expander* expander = &getLeftExpander();
	while ((expander->module != nullptr) && (expander->module->getModel() == modelPipoOutput)) {
		expander = &expander->module->getLeftExpander();
	}
	if ((expander->module != nullptr) && (expander->module->getModel() == modelPipoInput)) {
		lights[LightId::LIGHT_CONNECTED].setBrightness(1.f);
		lights[LightId::LIGHT_NOT_CONNECTED].setBrightness(0.f);
	} else {
		lights[LightId::LIGHT_CONNECTED].setBrightness(0.f);
		lights[LightId::LIGHT_NOT_CONNECTED].setBrightness(1.f);

		// Turn off all LEDs and set all output channels to 0V.
		// Do update the output channel counts so that the module doesn't have to be connected to have the output channels updated.
		for (int i = 0; i < 8; i++) {
			int channels = params[ParamId::PARAM_OUTPUT_CHANNELS + i].getValue();
			if (channels != outputs[OutputId::OUT_OUTPUTS + i].getChannels()) {
				outputs[OutputId::OUT_OUTPUTS + i].setChannels(channels);
			}
			outputs[OutputId::OUT_OUTPUTS + i].clearVoltages();
		}
		for (int i = 0; i < 8*3; i++) {
			lights[LightId::OUT_LIGHTS + i].setBrightness(0.f);
		}
	}
}

void PipoOutputModule::resetProcessingData() {
	m_processingData.exhaustedOutputs = false;
	m_processingData.exhaustedInputs = false;

	// Set this output module as current output module
	m_processingData.currentOutputModule = this;
	// Set the current output position before the first channel of the first output
	m_processingData.outputIndex = 0;
	m_processingData.outputChannelIndex = -1;
	m_processingData.outputChannelCount = params[ParamId::PARAM_OUTPUT_CHANNELS].getValue();

	// Update the channel count on the port if needed
	if (m_processingData.currentOutputModule->outputs[m_processingData.outputIndex].getChannels() != m_processingData.outputChannelCount) {
		m_processingData.currentOutputModule->outputs[m_processingData.outputIndex].setChannels(m_processingData.outputChannelCount);
	}

	// Set the left expander as current input module
	m_processingData.currentInputModule = getLeftExpander().module;
	// Set the current input position before the first channel of the first input
	m_processingData.inputIndex = 0;
	m_processingData.inputChannelIndex = -1;
	m_processingData.inputChannelCount = std::max(m_processingData.currentInputModule->inputs[PipoInputModule::InputId::IN_INPUTS].getChannels(), 1);
}

bool PipoOutputModule::moveToNextOutput() {
	if (!m_processingData.exhaustedOutputs) {
		// Move to the next channel on the current output port
		m_processingData.outputChannelIndex++;
		if (m_processingData.outputChannelIndex >= m_processingData.outputChannelCount) {
			// First set the LED of the current output port
			if (!m_processingData.exhaustedInputs) {
				// There are more inputs available, so the channels of the output port have all been filled
				m_processingData.currentOutputModule->lights[LightId::OUT_LIGHTS + (m_processingData.outputIndex * 3)].setBrightness(0.f);
				m_processingData.currentOutputModule->lights[LightId::OUT_LIGHTS + (m_processingData.outputIndex * 3) + 1].setBrightness(0.f);
				m_processingData.currentOutputModule->lights[LightId::OUT_LIGHTS + (m_processingData.outputIndex * 3) + 2].setBrightness(1.f);
			} else if (m_processingData.hasOutputChannelsAssigned) {
				// There are some output channels that have been assigned, but not all
				m_processingData.currentOutputModule->lights[LightId::OUT_LIGHTS + (m_processingData.outputIndex * 3)].setBrightness(0.f);
				m_processingData.currentOutputModule->lights[LightId::OUT_LIGHTS + (m_processingData.outputIndex * 3) + 1].setBrightness(1.f);
				m_processingData.currentOutputModule->lights[LightId::OUT_LIGHTS + (m_processingData.outputIndex * 3) + 2].setBrightness(0.f);
			} else {
				// No output channels were used
				m_processingData.currentOutputModule->lights[LightId::OUT_LIGHTS + (m_processingData.outputIndex * 3)].setBrightness(m_processingData.currentOutputModule->outputs[m_processingData.outputIndex].isConnected()); // Only show the LED if the output port is connected
				m_processingData.currentOutputModule->lights[LightId::OUT_LIGHTS + (m_processingData.outputIndex * 3) + 1].setBrightness(0.f);
				m_processingData.currentOutputModule->lights[LightId::OUT_LIGHTS + (m_processingData.outputIndex * 3) + 2].setBrightness(0.f);
			}
			m_processingData.hasOutputChannelsAssigned = false;

			// If we moved beyond the number of channels that are needed on the current output port, move to the next output port
			m_processingData.outputIndex++;
			m_processingData.outputChannelIndex = 0;
			if (m_processingData.outputIndex > 7) {
				// If we moved past the last output port on the current output module, move to the next output module
				if ((m_processingData.currentOutputModule->getRightExpander().module != nullptr) && (m_processingData.currentOutputModule->getRightExpander().module->getModel() == modelPipoOutput)) {
					// Update our position to the next output module
					m_processingData.currentOutputModule = m_processingData.currentOutputModule->getRightExpander().module;
					m_processingData.outputIndex = 0;
				} else {
					// There are no further output modules anymore
					m_processingData.exhaustedOutputs = true;
				}
			}

			// If we didn't move beyond the very last output port, update the channel count.
			if (!m_processingData.exhaustedOutputs) {
				m_processingData.outputChannelCount = m_processingData.currentOutputModule->params[ParamId::PARAM_OUTPUT_CHANNELS + m_processingData.outputIndex].getValue();
				if (m_processingData.currentOutputModule->outputs[m_processingData.outputIndex].getChannels() != m_processingData.outputChannelCount) {
					m_processingData.currentOutputModule->outputs[m_processingData.outputIndex].setChannels(m_processingData.outputChannelCount);
				}
			}
		}
	}

	return !m_processingData.exhaustedOutputs;
}

bool PipoOutputModule::moveToNextInput() {
	if (!m_processingData.exhaustedInputs) {
		// Move to the next channel on the current input port
		m_processingData.inputChannelIndex++;
		if (m_processingData.inputChannelIndex >= m_processingData.inputChannelCount) {
			// If we moved beyond the number of channels that are available on the current input port, move to the next input port
			m_processingData.inputIndex++;
			m_processingData.inputChannelIndex = 0;
			if (m_processingData.inputIndex > 7) {
				// If we moved past the last input port on the current input module, move to the next input module
				if ((m_processingData.currentInputModule->getLeftExpander().module != nullptr) && (m_processingData.currentInputModule->getLeftExpander().module->getModel() == modelPipoInput)) {
					// Update our position to the next input module
					m_processingData.currentInputModule = m_processingData.currentInputModule->getLeftExpander().module;
					m_processingData.inputIndex = 0;
				} else {
					// There are no further input modules anymore
					m_processingData.exhaustedInputs = true;
				}
			}

			// If we didn't move beyond the very last input port, update the channel count to that of this port.
			if (!m_processingData.exhaustedInputs) {
				m_processingData.inputChannelCount = std::max(m_processingData.currentInputModule->inputs[PipoInputModule::InputId::IN_INPUTS + m_processingData.inputIndex].getChannels(), 1);
			}
		}
	}

	// If we haven't exhausted the inputs yet, indicate that at least one of the channels on the currently selected output port has a value assigned.
	m_processingData.hasOutputChannelsAssigned |= !m_processingData.exhaustedInputs;

	return !m_processingData.exhaustedInputs;
}


PipoOutputWidget::PipoOutputWidget(PipoOutputModule* module): NTModuleWidget(dynamic_cast<NTModule*>(module), "pipo-output") {
	float xOut = 25;
	float y = 41.5;
	float yDelta = 40;
	for (int i = 0; i < 8; i++) {
		addOutput(createOutputCentered<NTPort>(Vec(xOut, y), module, PipoOutputModule::OUT_OUTPUTS + i));
		addChild(createLightCentered<TinyLight<RedOrangeGreenLight>>(Vec(xOut + 12.5, y + 12.5), module, PipoOutputModule::OUT_LIGHTS + (i * 3)));
		addParam(createParamCentered<NTKnobDark16>(Vec(xOut - 15.5f, y + 15.5f), module, PipoOutputModule::PARAM_OUTPUT_CHANNELS + i));
		y += yDelta;
	}

	addChild(createLightCentered<TinyLight<GreenRedLight>>(Vec(5.f, 20.f), module, PipoOutputModule::LIGHT_CONNECTED));
}


Model* modelPipoOutput = createModel<PipoOutputModule, PipoOutputWidget>("pipo-output");