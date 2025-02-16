#pragma once

#include "core/timeseq-script.hpp"
#include <istream>
#include "nlohmann/json-schema.hpp"

using namespace nlohmann;


namespace timeseq {

struct JsonValidationError {
	JsonValidationError(const std::string& location, const std::string& message) : location(location), message(message) {}

	std::string location;
	std::string message;
};

struct JsonLoader {
	void setSchema(std::shared_ptr<json> schema);

	std::shared_ptr<json> loadJson(std::istream& inputStream, bool validate=true, std::vector<JsonValidationError> *validationErrors=nullptr);

	Script parseScript(const json& timelineJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptTimeline parseTimeline(const json& timelineJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptTimeScale parseTimeScale(const json& timeScaleJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptLane parseLane(const json& laneJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);

	private:
		json_schema::json_validator m_validator;
};


enum ValidationErrorCode {
	Script_TypeMissing = 1,
	Script_TypeUnsupported = 2,
	Script_VersionMissing = 3,
	Script_VersionUnsupported = 4,
	Script_TimelinesMissing = 5,

	Timeline_TimeScaleMissing = 100,
	Timeline_LanesMissing = 101,
	Timeline_LoopLockBoolean = 102,

	TimeScale_SampleRateNumber = 103,
	TimeScale_BpmNumber = 104,
	TimeScale_BpbNumber = 105,
	TimeScale_SamplerateOrBpmRequired = 106,
	TimeScale_SampleRateOnly = 107,
	TimeScale_BpbRequiresBpm = 108
};

}