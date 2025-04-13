#include "core/timeseq-json.hpp"
#include <sstream>

using namespace std;
using namespace timeseq;


JsonLoader::JsonLoader() {
	m_jsonScriptParser = new JsonScriptParser();
}

JsonLoader::~JsonLoader() {
	delete m_jsonScriptParser;
}

shared_ptr<json> JsonLoader::loadJson(istream& inputStream, vector<ValidationError> *validationErrors) {
	shared_ptr<json> json;

	try {
		json = make_shared<nlohmann::json>(json::parse(inputStream));
	} catch (const json::parse_error& error) {
		if (validationErrors != nullptr) {
			string location = "/";
			string message = error.what();
			validationErrors->emplace_back(location, message);
		}
	}

	return json;
}

shared_ptr<Script> JsonLoader::loadScript(istream& inputStream, vector<ValidationError> *validationErrors) {
	shared_ptr<Script> script;
	
	shared_ptr<json> json = loadJson(inputStream, validationErrors);
	if (json) {
		script = m_jsonScriptParser->parseScript(*json, validationErrors, vector<string>());
	}

	return script;
}