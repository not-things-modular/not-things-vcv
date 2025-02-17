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

struct JsonScriptParser {
	std::shared_ptr<Script> parseScript(const json& timelineJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptTimeline parseTimeline(const json& timelineJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptTimeScale parseTimeScale(const json& timeScaleJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptLane parseLane(const json& laneJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSegmentEntity parseSegmentEntity(const json& segmentEntityJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSegment parseSegment(const json& segmentJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSegmentBlock parseSegmentBlock(const json& segmentBlockJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptDuration parseDuration(const json& durationJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptAction parseAction(const json& actionJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSetValue parseSetValue(const json& setValueJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptSetPolyphony parseSetPolyphony(const json& setPolyphonyJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptValue parseValue(const json& valueJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptOutput parseOutput(const json& outputJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptInput parseInput(const json& inputJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptRand parseRand(const json& randJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptCalc parseCalc(const json& calcJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
	ScriptInputTrigger parseInputTrigger(const json& inputTriggerJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);

	void populateRef(ScriptRefObject &refObject, const json& refJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
};

struct JsonLoader {
	JsonLoader();
	~JsonLoader();

	void setSchema(std::shared_ptr<json> schema);

	std::shared_ptr<json> loadJson(std::istream& inputStream, bool validate=true, std::vector<JsonValidationError> *validationErrors=nullptr);
	std::shared_ptr<Script> loadScript(std::istream& inputStream, std::vector<JsonValidationError> *validationErrors);

	private:
		json_schema::json_validator *m_validator;
		JsonScriptParser *m_jsonScriptParser;
};


enum ValidationErrorCode {
	Ref_String = 1,
	Ref_Length = 2,
	Ref_NotAllowed = 3,
	Id_String = 4,
	Id_Length = 5,
	Id_NotAllowed= 6,

	Script_TypeMissing = 101,
	Script_TypeUnsupported = 102,
	Script_VersionMissing = 103,
	Script_VersionUnsupported = 104,
	Script_TimelinesMissing = 105,
	Script_TimelineObject = 106,
	Script_SegmentBlocksArray = 107,
	Script_SegmentBlockObject = 108,
	Script_SegmentBlocksItemRequired = 109,
	Script_SegmentsArray = 110,
	Script_SegmentObject = 111,
	Script_SegmentsItemRequired = 112,
	Script_InputsArray = 113,
	Script_InputObject = 114,
	Script_InputsItemRequired = 115,
	Script_OutputsArray = 116,
	Script_OutputObject = 117,
	Script_OutputsItemRequired = 118,
	Script_CalcsArray = 119,
	Script_CalcObject = 120,
	Script_CalcsItemRequired = 121,
	Script_ValuesArray = 122,
	Script_ValueObject = 123,
	Script_ValuesItemRequired = 124,
	Script_ActionsArray = 125,
	Script_ActionObject = 126,
	Script_ActionsItemRequired = 127,
	Script_InputTriggersArray = 126,
	Script_InputTriggerObject = 127,
	Script_InputTriggersItemRequired = 128,

	Timeline_TimeScaleObject = 200,
	Timeline_LanesMissing = 201,
	Timeline_LaneObject = 202,
	Timeline_LoopLockBoolean = 203,

	TimeScale_SampleRateNumber = 303,
	TimeScale_BpmNumber = 304,
	TimeScale_BpbNumber = 305,
	TimeScale_SampleRateOnly = 306,
	TimeScale_BpbRequiresBpm = 307,

	Lane_LoopBoolean = 300,
	Lane_RepeatNumber = 301,
	Lane_StartTriggerString = 302,
	Lane_StartTriggerLength = 303,
	Lane_StopTriggerString = 304,
	Lane_StopTriggerLength = 305,
	Lane_SegmentsMissing = 305,
	Lane_SegmentObject = 306,

	SegmentEntity_SegmentObject = 400,
	SegmentEntity_SegmentBlockObject = 401,
	SegmentEntity_NoSegmentOrSegmentBlock = 402,
	SegmentEntity_EitherSegmentOrSegmentBlock = 403,

	Segment_RefOrInstance = 500,
	Segment_DurationNumber = 501,
	Segment_ActionsArray = 502,
	Segment_ActionObject = 503,

	SegmentBlock_RefOrInstance = 600,
	SegmentBlock_SegmentsArray = 601,
	SegmentBlock_SegmentObject = 602,

	Duration_SamplesNumber = 700,
	Duration_MillisNumber = 701,
	Duration_BarsNumber = 702,
	Duration_BeatsNumber = 703,
	Duration_NoSamplesOrMillisOrBars = 704,
	Duration_EitherSamplesOrMillisOrBars = 705,
	Duration_BarsRequiresBeats = 706,

	Action_RefOrInstance = 800,
	Action_TimingEnum = 801,
	Action_SetValueObject = 802,
	Action_SetPolyphonyObject = 803,
	Action_TriggerString = 804,
	Action_TriggerLength = 805,
	Action_StartValueObject = 806,
	Action_EndValueObject = 807,
	Action_NonGlideProperties = 808,
	Action_OnlyGlideProperties = 808,

	SetValue_OutputObject = 900,
	SetValue_ValueObject = 901,

	SetPolyphony_IndexNumber = 1000,
	SetPolyphony_IndexRange = 1001,
	SetPolyphony_ChannelsNumber = 1002,
	SetPolyphony_ChannelsRange = 1003,

	Value_RefOrInstance = 1100,
	Value_VoltageFloat = 1101,
	Value_VoltageRange = 1102,
	Value_NoteString = 1103,
	Value_NoteFormat = 1104,
	Value_InputObject = 1105,
	Value_OutputObject = 1106,
	Value_RandObject = 1107,
	Value_CalcArray = 1108,
	Value_CalcObject = 1109,

	Output_RefOrInstance = 1200,
	Output_IndexNumber = 1201,
	Output_IndexRange = 1202,
	Output_ChannelNumber = 1203,
	Output_ChannelRange = 1204,

	Input_RefOrInstance = 1300,
	Input_IndexNumber = 1301,
	Input_IndexRange = 1302,
	Input_ChannelNumber = 1303,
	Input_ChannelRange = 1304,

	Rand_LowerObject = 1400,
	Rand_UpperObject = 1401,

	Calc_NoOperation = 1500,
	Calc_MultpleOperations = 1501,
	Calc_AddObject = 1502,
	Calc_SubObject = 1503,
	Calc_DivObject = 1504,
	Calc_MultObject = 1505,

	InputTrigger_IdString = 1600,
	InputTrigger_IdLength = 1601,
	InputTrigger_InputObject = 1062
};

}