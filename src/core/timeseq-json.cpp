#include "core/timeseq-json.hpp"

void timeseq::JsonLoader::setSchema(std::shared_ptr<nlohmann::json> schema) {
	m_validator.set_root_schema(*schema);
}

std::shared_ptr<nlohmann::json> timeseq::JsonLoader::loadJson(std::istream &inputStream, bool validate) {
	std::shared_ptr<nlohmann::json> json = std::make_shared<nlohmann::json>(nlohmann::json::parse(inputStream));

	if (validate) {
		m_validator.validate(*json);
	}

	return json;
}

