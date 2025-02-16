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
	Script parseScript(const json& timelineJson, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
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

	void populateRef(ScriptRefObject &refObject, const json& refJson, bool allowRefs, std::vector<JsonValidationError> *validationErrors, std::vector<std::string> location);
};

struct JsonLoader {
	JsonLoader();
	~JsonLoader();

	void setSchema(std::shared_ptr<json> schema);

	std::shared_ptr<json> loadJson(std::istream& inputStream, bool validate=true, std::vector<JsonValidationError> *validationErrors=nullptr);

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
	Script_SegmentBlocksArray = 106,
	Script_SegmentBlocksItemRequired = 107,
	Script_SegmentsArray = 108,
	Script_SegmentsItemRequired = 109,

	Timeline_TimeScaleObject = 200,
	Timeline_LanesMissing = 201,
	Timeline_LoopLockBoolean = 202,

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

	SegmentEntity_SegmentObject = 400,
	SegmentEntity_SegmentBlockObject = 401,
	SegmentEntity_NoSegmentOrSegmentBlock = 402,
	SegmentEntity_EitherSegmentOrSegmentBlock = 403,

	Segment_RefOrInstance = 500,
	Segment_DurationNumber = 501,
	Segment_ActionsArray = 502,

	SegmentBlock_RefOrInstance = 600,
	SegmentBlock_SegmentsArray = 601,

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

	SetPolyphony_IndexNumber = 902,
	SetPolyphony_IndexRange = 902,
	SetPolyphony_ChannelsNumber = 903,
	SetPolyphony_ChannelsRange = 904,
};

}