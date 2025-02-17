#pragma once

#include "core/timeseq-json.hpp"


namespace timeseq {

struct TimeSeqCore {
	TimeSeqCore();

	std::vector<timeseq::JsonValidationError> loadScript(std::string& scriptData);

	private:
		JsonLoader m_jsonLoader;
};

}