#pragma once

#include <string>
#include <vector>

namespace timeseq {

enum ValidationErrorCode {
	Ref_String = 1,
	Ref_Length = 2,
	Ref_NotAllowed = 3,
	Ref_NotFound = 4,
	Id_String = 5,
	Id_Length = 6,
	Id_NotAllowed= 7,

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

	Lane_AutoStartBoolean = 300,
	Lane_LoopBoolean = 301,
	Lane_RepeatNumber = 302,
	Lane_StartTriggerString = 303,
	Lane_StartTriggerLength = 304,
	Lane_StopTriggerString = 305,
	Lane_StopTriggerLength = 306,
	Lane_SegmentsMissing = 307,
	Lane_SegmentObject = 308,

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


std::string createValidationErrorMessage(ValidationErrorCode code, ...);
std::string createValidationErrorLocation(std::vector<std::string> location);
#define ADD_VALIDATION_ERROR(validationErrors, location, code, message...) \
	if (validationErrors != nullptr) { \
		string errorLocation = createValidationErrorLocation(location); \
		string errorMessage = createValidationErrorMessage(code, message, ""); \
		validationErrors->emplace_back(errorLocation, errorMessage); \
	}


	struct ValidationError {
	ValidationError(const std::string& location, const std::string& message) : location(location), message(message) {}

	std::string location;
	std::string message;
};

}