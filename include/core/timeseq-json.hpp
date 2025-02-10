#pragma once

#include "nlohmann/json-schema.hpp"
#include <istream>


namespace timeseq {

struct JsonLoader {
	void setSchema(std::shared_ptr<nlohmann::json> schema);
	std::shared_ptr<nlohmann::json> loadJson(std::istream &inputStream, bool validate=true);

	private:
		nlohmann::json_schema::json_validator m_validator;
};

}