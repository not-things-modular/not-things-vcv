#pragma once

#include "core/timeseq-script.hpp"
#include <istream>
#include "nlohmann/json-schema.hpp"
#include "core/timeseq-validation.hpp"

using namespace nlohmann;


namespace timeseq {

struct JsonScriptParser {
	std::shared_ptr<Script> parseScript(const json& timelineJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptTimeline parseTimeline(const json& timelineJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptTimeScale parseTimeScale(const json& timeScaleJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptLane parseLane(const json& laneJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSegmentEntity parseSegmentEntity(const json& segmentEntityJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSegment parseSegment(const json& segmentJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSegmentBlock parseSegmentBlock(const json& segmentBlockJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptDuration parseDuration(const json& durationJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptAction parseAction(const json& actionJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSetValue parseSetValue(const json& setValueJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSetPolyphony parseSetPolyphony(const json& setPolyphonyJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptValue parseValue(const json& valueJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptOutput parseOutput(const json& outputJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptInput parseInput(const json& inputJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptRand parseRand(const json& randJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptCalc parseCalc(const json& calcJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptInputTrigger parseInputTrigger(const json& inputTriggerJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);

	void populateRef(ScriptRefObject &refObject, const json& refJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
};

struct JsonLoader {
	JsonLoader();
	~JsonLoader();

	void setSchema(std::shared_ptr<json> schema);

	std::shared_ptr<json> loadJson(std::istream& inputStream, bool validate=true, std::vector<ValidationError> *validationErrors=nullptr);
	std::shared_ptr<Script> loadScript(std::istream& inputStream, std::vector<ValidationError> *validationErrors);

	private:
		json_schema::json_validator *m_validator;
		JsonScriptParser *m_jsonScriptParser;
};

}