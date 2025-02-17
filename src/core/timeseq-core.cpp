#include "core/timeseq-core.hpp"
#include "core/timeseq-script.hpp"
#include <rack.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

extern rack::Plugin* pluginInstance;


timeseq::TimeSeqCore::TimeSeqCore() {
	std::string schemaFile = rack::asset::plugin(pluginInstance, "res/timeseq.schema.json");
	std::ifstream schemaStream(schemaFile);
	std::shared_ptr<json> schemaJson = m_jsonLoader.loadJson(schemaStream, false);

	m_jsonLoader.setSchema(schemaJson);
}

std::vector<timeseq::JsonValidationError> timeseq::TimeSeqCore::loadScript(std::string& scriptData) {
	std::istringstream scriptStream(scriptData);
	std::vector<timeseq::JsonValidationError> errors;
	
	// std::shared_ptr<json> json = m_jsonLoader.loadJson(scriptStream, true, &errors);
	std::shared_ptr<Script> script = m_jsonLoader.loadScript(scriptStream, &errors);

	return errors;
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