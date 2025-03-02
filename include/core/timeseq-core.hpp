#pragma once

#include "core/timeseq-json.hpp"
#include "core/timeseq-processor.hpp"


namespace timeseq {

struct PortReader {
	virtual float getInputPortVoltage(int index, int channel);
	virtual float getOutputPortVoltage(int index, int channel);
};

struct PortWriter {
	virtual void setOutputPortVoltage(int index, int channel, float voltage);
	virtual void setOutputPortChannels(int index, int channels);
};

struct TimeSeqCore {
	TimeSeqCore(PortReader* portReader, PortWriter* portWriter);
	~TimeSeqCore();

	std::vector<timeseq::ValidationError> loadScript(std::string& scriptData);

	private:
		JsonLoader* m_jsonLoader;
		ProcessorLoader* m_processorLoader;
};

}