#include "core/timeseq-core.hpp"
#include "core/timeseq-script.hpp"
#include <rack.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

extern rack::Plugin* pluginInstance;

using namespace timeseq;


TimeSeqCore::TimeSeqCore(PortReader* portReader, SampleRateReader* sampleRateReader, PortWriter* portWriter) {
	m_jsonLoader = new JsonLoader();
	std::string schemaFile = rack::asset::plugin(pluginInstance, "res/timeseq.schema.json");
	std::ifstream schemaStream(schemaFile);
	std::shared_ptr<json> schemaJson = m_jsonLoader->loadJson(schemaStream, false);

	m_jsonLoader->setSchema(schemaJson);

	m_processorLoader = new ProcessorLoader(portReader, sampleRateReader, portWriter, this);
}

TimeSeqCore::~TimeSeqCore() {
}

std::vector<ValidationError> TimeSeqCore::loadScript(std::string& scriptData) {
	std::istringstream scriptStream(scriptData);
	std::vector<ValidationError> validationErrors;
	
	std::shared_ptr<Script> script = m_jsonLoader->loadScript(scriptStream, &validationErrors);
	std::shared_ptr<Processor> processor = m_processorLoader->loadScript(script, &validationErrors);

	if (validationErrors.size() == 0) {
		m_script = script;
		m_processor = processor;
		
		m_processor->reset();
		m_status = Status::IDLE;
	}

	return validationErrors;
}

TimeSeqCore::Status TimeSeqCore::getStatus() {
	return m_status;
}

bool TimeSeqCore::canProcess() {
	return (bool) m_processor;
}

void TimeSeqCore::start() {
	if (m_processor) {
		m_status = Status::RUNNING;
	} else {
		m_status = Status::EMPTY;
	}
}

void TimeSeqCore::pause() {
	if (m_processor) {
		m_status = Status::PAUSED;
	} else {
		m_status = Status::EMPTY;
	}
}

void TimeSeqCore::reset() {
	if (m_processor) {
		m_processor->reset();
	}
}

void TimeSeqCore::process() {
	if (m_processor) {
		m_processor->process();
	}
}

float TimeSeqCore::getVariable(std::string name) {
	std::unordered_map<std::string, float>::iterator it = m_variables.find(name);
	if (it != m_variables.end()) {
		return it->second;
	} else {
		return 0.f;
	}
}

void TimeSeqCore::setVariable(std::string name, float value) {
	m_variables[name] = value;
}


// void testJsonValidation() {
// 	// nlohmann::json schema;
// 	// nlohmann::json_schema::json_validator validator;

// 	// class custom_error_handler : public nlohmann::json_schema::basic_error_handler
//     // {
//     //     void error(const json::json_pointer &pointer, const json &instance, const std::string &message) override
//     //     {
//     //         nlohmann::json_schema::basic_error_handler::error(pointer, instance, message);
//     //         std::cerr << "ERROR: '" << pointer << "' - '" << instance << "': " << message << "\n";
//     //     }
//     // };

// 	std::string x = rack::asset::plugin(pluginInstance, "res/timeseq.schema.json");
// 	std::ifstream f1(x);
// 	x = rack::asset::plugin(pluginInstance, "test/resources/sequencer-test.json");
// 	std::ifstream f2(x);
// 	try {
// 		JsonLoader jsonLoader;

// 		std::shared_ptr<json> json = jsonLoader.loadJson(f1, false);
// 		jsonLoader.setSchema(json);
// 		jsonLoader.loadJson(f2);
// 		// custom_error_handler  err;
// 		// schema = json::parse(f);
// 		// validator.set_root_schema(schema);
// 		// validator.validate(schema);
// 	} catch (const std::exception &e) {
// 		auto z = e.what();
// 		std::cerr << "Validation of schema failed, here is why: " << e.what() << "\n";
// 	}
// }