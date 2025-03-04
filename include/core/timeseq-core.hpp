#pragma once

#include "core/timeseq-json.hpp"
#include "core/timeseq-processor.hpp"


namespace timeseq {

struct PortReader {
	virtual float getInputPortVoltage(int index, int channel) = 0;
	virtual float getOutputPortVoltage(int index, int channel) = 0;
};

struct PortWriter {
	virtual void setOutputPortVoltage(int index, int channel, float voltage) = 0;
	virtual void setOutputPortChannels(int index, int channels) = 0;
};

struct SampleRateReader {
	virtual float getSampleRate() = 0;
};

struct TimeSeqCore {
	TimeSeqCore(PortReader* portReader, SampleRateReader* sampleRateReader, PortWriter* portWriter);
	~TimeSeqCore();

	std::vector<timeseq::ValidationError> loadScript(std::string& scriptData);

	private:
		JsonLoader* m_jsonLoader;
		ProcessorLoader* m_processorLoader;
};

}