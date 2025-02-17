#include "core/timeseq-json.hpp"
#include <sstream>

using namespace std;
using namespace timeseq;


class ValidationErrorHandler : public json_schema::error_handler {
	public:
		ValidationErrorHandler(vector<JsonValidationError> *validationErrors) {
			m_validationErrors = validationErrors;
		}

		void error(const json::json_pointer &ptr, const json &instance, const string &message) override
		{
			if (m_validationErrors != nullptr) {
				string location = ptr.to_string();
				m_validationErrors->emplace_back(location, message);
			}
		}

	private:
		vector<JsonValidationError> *m_validationErrors;
};


JsonLoader::JsonLoader() {
	m_validator = new json_schema::json_validator();
	m_jsonScriptParser = new JsonScriptParser();
}

JsonLoader::~JsonLoader() {
	delete m_validator;
	delete m_jsonScriptParser;
}

void JsonLoader::setSchema(shared_ptr<json> schema) {
	m_validator->set_root_schema(*schema);
}

shared_ptr<json> JsonLoader::loadJson(istream& inputStream, bool validate, vector<JsonValidationError> *validationErrors) {
	shared_ptr<json> json;

	try {
		json = make_shared<nlohmann::json>(json::parse(inputStream));

		if (validate) {
			ValidationErrorHandler errorHandler(validationErrors);
			m_validator->validate(*json, errorHandler);
		}
	} catch (const json::parse_error& error) {
		if (validationErrors != nullptr) {
			string location = "/";
			string message = error.what();
			validationErrors->emplace_back(location, message);
		}
	}

	return json;
}

shared_ptr<Script> JsonLoader::loadScript(istream& inputStream, vector<JsonValidationError> *validationErrors) {
	shared_ptr<Script> script;
	
	shared_ptr<json> json = loadJson(inputStream, true, validationErrors);
	if (json) {
		script = m_jsonScriptParser->parseScript(*json, validationErrors, vector<string>());
	}

	return script;
}