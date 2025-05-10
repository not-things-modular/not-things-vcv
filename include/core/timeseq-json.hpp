#pragma once

#include "core/timeseq-script.hpp"
#include <istream>
#include "nlohmann/json.hpp"
#include "core/timeseq-validation.hpp"

using namespace nlohmann;


namespace timeseq {

struct JsonScriptParser {
	virtual ~JsonScriptParser();

	virtual std::shared_ptr<Script> parseScript(const json& timelineJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptTimeline parseTimeline(const json& timelineJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptTimeScale parseTimeScale(const json& timeScaleJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptLane parseLane(const json& laneJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSegment parseSegment(const json& segmentJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSegmentBlock parseSegmentBlock(const json& segmentBlockJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptDuration parseDuration(const json& durationJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptAction parseAction(const json& actionJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSetValue parseSetValue(const json& setValueJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSetVariable parseSetVariable(const json& setVariableJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSetPolyphony parseSetPolyphony(const json& setPolyphonyJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptAssert parseAssert(const json& assertJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptValue parseValue(const json& valueJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptOutput parseOutput(const json& outputJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptInput parseInput(const json& inputJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptRand parseRand(const json& randJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptCalc parseCalc(const json& calcJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptInputTrigger parseInputTrigger(const json& inputTriggerJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);

	ScriptIf parseIf(const json& ifJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	std::pair<ScriptValue, ScriptValue> parseIfValues(std::string ifOperator, const json& valuesJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	std::unique_ptr<std::pair<ScriptIf, ScriptIf>> parseIfIfs(std::string ifOperator, const json& ifsJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);

	void populateRef(ScriptRefObject &refObject, const json& refJson, bool allowRefs, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
};

struct JsonLoader {
	JsonLoader();
	virtual ~JsonLoader();

	virtual std::shared_ptr<json> loadJson(std::istream& inputStream, std::vector<ValidationError>* validationErrors=nullptr);
	virtual std::shared_ptr<Script> loadScript(std::istream& inputStream, std::vector<ValidationError>* validationErrors);

	private:
		JsonScriptParser *m_jsonScriptParser;
};

}