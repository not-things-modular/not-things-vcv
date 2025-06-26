#pragma once

#include "core/timeseq-script.hpp"
#include <istream>
#include "nlohmann/json.hpp"
#include "core/timeseq-validation.hpp"

using namespace nlohmann;


namespace timeseq {

struct JsonScriptParseContext;

struct JsonScriptParser {
	virtual ~JsonScriptParser();

	virtual std::shared_ptr<Script> parseScript(const json& timelineJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptTimeline parseTimeline(const json& timelineJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptTimeScale parseTimeScale(const json& timeScaleJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptLane parseLane(const json& laneJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSegment parseSegment(const json& segmentJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSegmentBlock parseSegmentBlock(const json& segmentBlockJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptDuration parseDuration(const json& durationJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptAction parseAction(const json& actionJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSetValue parseSetValue(const json& setValueJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSetVariable parseSetVariable(const json& setVariableJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSetPolyphony parseSetPolyphony(const json& setPolyphonyJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSetLabel parseSetLabel(const json& setLabelJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptAssert parseAssert(const json& assertJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptValue parseValue(const json& valueJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage);
	ScriptValue parseFullValue(const json& valueJson, bool allowRefs, bool fromShorthand, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptOutput parseOutput(const json& outputJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage);
	ScriptOutput parseFullOutput(const json& outputJson, bool allowRefs, bool fromShorthand, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptInput parseInput(const json& inputJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage);
	ScriptInput parseFullInput(const json& inputJson, bool allowRefs, bool fromShorthand, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptRand parseRand(const json& randJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptCalc parseCalc(const json& calcJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptInputTrigger parseInputTrigger(const json& inputTriggerJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptTuning parseTuning(const json& tuningJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptIf parseIf(const json& ifJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
	std::pair<ScriptValue, ScriptValue> parseIfValues(std::string ifOperator, const json& valuesJson, JsonScriptParseContext* context, std::vector<std::string> location);
	std::unique_ptr<std::pair<ScriptIf, ScriptIf>> parseIfIfs(std::string ifOperator, const json& ifsJson, JsonScriptParseContext* context, std::vector<std::string> location);

	void populateRef(ScriptRefObject &refObject, const json& refJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
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