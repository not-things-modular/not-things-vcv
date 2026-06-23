#pragma once

#include "core/timeseq-script.hpp"
#include <istream>
#include "nlohmann/json.hpp"
#include "core/timeseq-validation.hpp"

namespace timeseq {

struct JsonScriptParseContext;

struct JsonScriptParser {
	virtual ~JsonScriptParser();

	virtual std::shared_ptr<Script> parseScript(const nlohmann::json& timelineJson, std::vector<ValidationError> *validationErrors, std::vector<std::string> location);
	ScriptTimeline parseTimeline(const nlohmann::json& timelineJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptTimeScale parseTimeScale(const nlohmann::json& timeScaleJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptLane parseLane(const nlohmann::json& laneJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSegment parseSegment(const nlohmann::json& segmentJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSegmentBlock parseSegmentBlock(const nlohmann::json& segmentBlockJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptDuration parseDuration(const nlohmann::json& durationJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptAction parseAction(const nlohmann::json& actionJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSetValue parseSetValue(const nlohmann::json& setValueJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSetVariable parseSetVariable(const nlohmann::json& setVariableJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSetPolyphony parseSetPolyphony(const nlohmann::json& setPolyphonyJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSetLabel parseSetLabel(const nlohmann::json& setLabelJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptAssert parseAssert(const nlohmann::json& assertJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptMoveSequence parseMoveSequence(const nlohmann::json& moveSequenceJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptAddToSequence parseAddToSequence(const nlohmann::json& addToSequenceJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptRemoveFromSequence parseRemoveFromSequence(const nlohmann::json& removeFromJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptValue parseValue(const nlohmann::json& valueJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage);
	ScriptValue parseFullValue(const nlohmann::json& valueJson, bool allowRefs, bool fromShorthand, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptOutput parseOutput(const nlohmann::json& outputJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage);
	ScriptOutput parseFullOutput(const nlohmann::json& outputJson, bool allowRefs, bool fromShorthand, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptInput parseInput(const nlohmann::json& inputJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage);
	ScriptInput parseFullInput(const nlohmann::json& inputJson, bool allowRefs, bool fromShorthand, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptRand parseRand(const nlohmann::json& randJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptCalc parseCalc(const nlohmann::json& calcJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptInputTrigger parseInputTrigger(const nlohmann::json& inputTriggerJson, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptTuning parseTuning(const nlohmann::json& tuningJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptIf parseIf(const nlohmann::json& ifJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
	std::pair<ScriptValue, ScriptValue> parseIfValues(std::string ifOperator, const nlohmann::json& valuesJson, JsonScriptParseContext* context, std::vector<std::string> location);
	std::unique_ptr<std::vector<ScriptIf>> parseIfIfs(std::string ifOperator, const nlohmann::json& ifsJson, JsonScriptParseContext* context, std::vector<std::string> location);

	ScriptSequenceValue parseSequenceValue(const nlohmann::json& sequenceJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
	ScriptSequence parseSequence(const nlohmann::json& sequenceJson, JsonScriptParseContext* context, std::vector<std::string> location);

	void populateRef(ScriptRefObject &refObject, const nlohmann::json& refJson, bool allowRefs, JsonScriptParseContext* context, std::vector<std::string> location);
};

struct JsonLoader {
	virtual ~JsonLoader();

	virtual std::shared_ptr<nlohmann::json> loadJson(std::istream& inputStream, std::vector<ValidationError>* validationErrors=nullptr);
	virtual std::shared_ptr<Script> loadScript(std::istream& inputStream, std::vector<ValidationError>* validationErrors);

	private:
		JsonScriptParser m_jsonScriptParser;
};

}