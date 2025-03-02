#include "core/timeseq-core.hpp"
#include "core/timeseq-script.hpp"
#include <rack.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

extern rack::Plugin* pluginInstance;


timeseq::TimeSeqCore::TimeSeqCore(PortReader* portReader, PortWriter* portWriter) {
	m_jsonLoader = new JsonLoader();
	std::string schemaFile = rack::asset::plugin(pluginInstance, "res/timeseq.schema.json");
	std::ifstream schemaStream(schemaFile);
	std::shared_ptr<json> schemaJson = m_jsonLoader->loadJson(schemaStream, false);

	m_jsonLoader->setSchema(schemaJson);

	m_processorLoader = new ProcessorLoader(portReader, portWriter);
}

timeseq::TimeSeqCore::~TimeSeqCore() {
}

std::vector<timeseq::ValidationError> timeseq::TimeSeqCore::loadScript(std::string& scriptData) {
	std::istringstream scriptStream(scriptData);
	std::vector<timeseq::ValidationError> validationErrors;
	
	std::shared_ptr<Script> script = m_jsonLoader->loadScript(scriptStream, &validationErrors);
	std::shared_ptr<Processor> processor = m_processorLoader->loadScript(script, &validationErrors);

	return validationErrors;
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
// 		timeseq::JsonLoader jsonLoader;

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