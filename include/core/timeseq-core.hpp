#pragma once

#include "core/timeseq-json.hpp"


namespace timeseq {

struct TimeSeqCore {
	TimeSeqCore();

	std::vector<timeseq::JsonValidationError> loadScript(std::string& script);

	private:
		JsonLoader m_jsonLoader;
};

}