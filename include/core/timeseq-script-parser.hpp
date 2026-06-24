#pragma once

#include "core/timeseq-script.hpp"
#include "core/timeseq-validation.hpp"
#include "nlohmann/json.hpp"
#include <istream>
#include <vector>
#include <string>
#include <memory>

namespace timeseq {

struct JsonScriptParseContext {
	int version;
	std::vector<std::string> location;
	std::vector<ValidationError> validationErrors;
};

struct JsonScriptParser {
	std::shared_ptr<Script> parseScript(const nlohmann::json& scriptJson);

	const std::vector<ValidationError> getValidationErrors();

	private:
		ScriptTimeline parseTimeline(const nlohmann::json& timelineJson);
		ScriptTimeScale parseTimeScale(const nlohmann::json& timeScaleJson);
		ScriptLane parseLane(const nlohmann::json& laneJson);
		ScriptSegment parseSegment(const nlohmann::json& segmentJson, bool allowRefs);
		ScriptSegmentBlock parseSegmentBlock(const nlohmann::json& segmentBlockJson);
		ScriptDuration parseDuration(const nlohmann::json& durationJson);
		ScriptAction parseAction(const nlohmann::json& actionJson, bool allowRefs);
		ScriptSetValue parseSetValue(const nlohmann::json& setValueJson);
		ScriptSetVariable parseSetVariable(const nlohmann::json& setVariableJson);
		ScriptSetPolyphony parseSetPolyphony(const nlohmann::json& setPolyphonyJson);
		ScriptSetLabel parseSetLabel(const nlohmann::json& setLabelJson);
		ScriptAssert parseAssert(const nlohmann::json& assertJson);
		ScriptMoveSequence parseMoveSequence(const nlohmann::json& moveSequenceJson);
		ScriptAddToSequence parseAddToSequence(const nlohmann::json& addToSequenceJson);
		ScriptRemoveFromSequence parseRemoveFromSequence(const nlohmann::json& removeFromJson);
		ScriptValue parseValue(const nlohmann::json& valueJson, bool allowRefs, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage);
		ScriptValue parseFullValue(const nlohmann::json& valueJson, bool allowRefs, bool fromShorthand);
		ScriptOutput parseOutput(const nlohmann::json& outputJson, bool allowRefs, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage);
		ScriptOutput parseFullOutput(const nlohmann::json& outputJson, bool allowRefs, bool fromShorthand);
		ScriptInput parseInput(const nlohmann::json& inputJson, bool allowRefs, std::string subLocation, ValidationErrorCode validationErrorCode, std::string validationErrorMessage);
		ScriptInput parseFullInput(const nlohmann::json& inputJson, bool allowRefs, bool fromShorthand);
		ScriptRand parseRand(const nlohmann::json& randJson);
		ScriptCalc parseCalc(const nlohmann::json& calcJson, bool allowRefs);
		ScriptInputTrigger parseInputTrigger(const nlohmann::json& inputTriggerJson);
		ScriptTuning parseTuning(const nlohmann::json& tuningJson, bool allowRefs);
		ScriptIf parseIf(const nlohmann::json& ifJson, bool allowRefs);
		std::pair<ScriptValue, ScriptValue> parseIfValues(std::string ifOperator, const nlohmann::json& valuesJson);
		std::unique_ptr<std::vector<ScriptIf>> parseIfIfs(std::string ifOperator, const nlohmann::json& ifsJson);

		ScriptSequenceValue parseSequenceValue(const nlohmann::json& sequenceJson, bool allowRefs);
		ScriptSequence parseSequence(const nlohmann::json& sequenceJson);

		void populateRef(ScriptRefObject &refObject, const nlohmann::json& refJson, bool allowRefs);

		std::unique_ptr<JsonScriptParseContext> m_context;
};

struct JsonLoader {
	virtual ~JsonLoader();

	virtual std::shared_ptr<nlohmann::json> loadJson(std::istream& inputStream, std::vector<ValidationError>* validationErrors=nullptr);
	virtual std::shared_ptr<Script> loadScript(std::istream& inputStream, std::vector<ValidationError>* validationErrors);
};

}