#pragma once

#include <string>
#include <vector>

namespace timeseq {

enum ValidationErrorCode {
	Ref_String = 1,
	Ref_Length = 2,
	Ref_NotAllowed = 3,
	Ref_NotFound = 4,
	Ref_CircularFound = 5,
	Id_String = 6,
	Id_Length = 7,
	Id_NotAllowed= 8,

	Script_TypeMissing = 101,
	Script_TypeUnsupported = 102,
	Script_VersionMissing = 103,
	Script_VersionUnsupported = 104,
	Script_TimelinesMissing = 105,
	Script_TimelineObject = 106,
	Script_SegmentBlocksArray = 107,
	Script_SegmentBlockObject = 108,
	Script_SegmentsArray = 109,
	Script_SegmentObject = 110,
	Script_InputsArray = 111,
	Script_InputObject = 112,
	Script_OutputsArray = 113,
	Script_OutputObject = 114,
	Script_CalcsArray = 115,
	Script_CalcObject = 116,
	Script_ValuesArray = 117,
	Script_ValueObject = 118,
	Script_ActionsArray = 119,
	Script_ActionObject = 120,
	Script_InputTriggersArray = 121,
	Script_InputTriggerObject = 122,
	Script_GlobalActionsArray = 123,
	Script_GlobalActionsObject = 124,
	Script_GlobalActionTiming = 125,
	Script_ComponentPoolObject = 126,

	Timeline_TimeScaleObject = 200,
	Timeline_LanesMissing = 201,
	Timeline_LaneObject = 202,
	Timeline_LoopLockBoolean = 203,

	TimeScale_SampleRateNumber = 303,
	TimeScale_BpmNumber = 304,
	TimeScale_BpbNumber = 305,
	TimeScale_BpbRequiresBpm = 306,
	TimeScale_Empty = 307,

	Lane_AutoStartBoolean = 400,
	Lane_LoopBoolean = 401,
	Lane_RepeatNumber = 402,
	Lane_StartTriggerString = 403,
	Lane_StartTriggerLength = 404,
	Lane_RestartTriggerString = 405,
	Lane_RestartTriggerLength = 406,
	Lane_StopTriggerString = 407,
	Lane_StopTriggerLength = 408,
	Lane_SegmentsMissing = 409,
	Lane_SegmentObject = 410,
	Lane_DisableUiBoolean = 411,

	If_EqArray = 500,
	If_NeArray = 501,
	If_LtArray = 502,
	If_LteArray = 503,
	If_GtArray = 504,
	If_GteArray = 505,
	If_AndArray = 506,
	If_OrArray = 507,
	If_NoOperation = 508,
	If_MultpleOperations = 509,
	If_TwoValues = 510,
	If_ToleranceNumber = 511,

	Segment_RefOrInstance = 600,
	Segment_DurationObject = 601,
	Segment_ActionsArray = 602,
	Segment_ActionObject = 603,
	Segment_SegmentBlockString = 604,
	Segment_SegmentBlockLength = 605,
	Segment_BlockOrSegment = 606,
	Segment_DisableUiBoolean = 607,

	SegmentBlock_RefOrInstance = 700,
	SegmentBlock_SegmentsArray = 701,
	SegmentBlock_SegmentObject = 702,
	SegmentBlock_RepeatNumber = 703,

	Duration_SamplesNumber = 800,
	Duration_MillisNumber = 801,
	Duration_BarsNumber = 802,
	Duration_BeatsNumber = 803,
	Duration_HzNumber = 804,
	Duration_NoSamplesOrMillisOrBeatsOrHz = 805,
	Duration_EitherSamplesOrMillisOrBeatsOrHz = 806,
	Duration_BarsRequiresBeats = 807,
	Duration_BeatsButNoBmp = 808,
	Duration_BarsButNoBpb = 809,
	Duration_BeatsNotZero = 810,

	Action_RefOrInstance = 900,
	Action_TimingEnum = 901,
	Action_SetValueObject = 902,
	Action_SetVariableObject = 903,
	Action_SetPolyphonyObject = 904,
	Action_AssertObject = 905,
	Action_TriggerString = 906,
	Action_TriggerLength = 907,
	Action_StartValueObject = 908,
	Action_EndValueObject = 909,
	Action_EaseFactorFloat = 910,
	Action_EaseFactorRange = 911,
	Action_EaseAlgorithmEnum = 912,
	Action_OutputObject = 913,
	Action_VariableString = 914,
	Action_VariableLength = 915,
	Action_IfObject = 916,
	Action_NonGlideProperties = 917,
	Action_MissingGlideValues = 918,
	Action_MissingGlideActions = 919,
	Action_TooManyGlideActions = 920,
	Action_OnlyGlideProperties = 921,
	Action_MissingNonGlideProperties = 922,
	Action_TooManyNonGlideProperties = 923,

	SetValue_OutputObject = 1000,
	SetValue_ValueObject = 1001,

	SetVariable_NameString = 1100,
	SetVariable_ValueObject = 1101,

	SetPolyphony_IndexNumber = 1200,
	SetPolyphony_IndexRange = 1201,
	SetPolyphony_ChannelsNumber = 1202,
	SetPolyphony_ChannelsRange = 1203,

	Assert_NameString = 1301,
	Assert_NameLength = 1302,
	Assert_ExpectObject = 1303,
	Assert_StopOnFailBool = 1304,

	Value_RefOrInstance = 1400,
	Value_NoActualValue = 1402,
	Value_MultipleValues = 1403,
	Value_VoltageFloat = 1404,
	Value_VoltageRange = 1405,
	Value_VariableString = 1406,
	Value_VariableNonEmpty = 1407,
	Value_NoteString = 1408,
	Value_NoteFormat = 1409,
	Value_InputObject = 1410,
	Value_OutputObject = 1411,
	Value_RandObject = 1412,
	Value_CalcArray = 1413,
	Value_CalcObject = 1414,
	Value_QuantizeBool = 1415,

	Output_RefOrInstance = 1500,
	Output_IndexNumber = 1501,
	Output_IndexRange = 1502,
	Output_ChannelNumber = 1503,
	Output_ChannelRange = 1504,

	Input_RefOrInstance = 1600,
	Input_IndexNumber = 1601,
	Input_IndexRange = 1602,
	Input_ChannelNumber = 1603,
	Input_ChannelRange = 1604,

	Rand_LowerObject = 1700,
	Rand_UpperObject = 1701,

	Calc_NoOperation = 1800,
	Calc_MultpleOperations = 1801,
	Calc_AddObject = 1802,
	Calc_SubObject = 1803,
	Calc_DivObject = 1804,
	Calc_MultObject = 1805,

	InputTrigger_IdString = 1900,
	InputTrigger_IdLength = 1901,
	InputTrigger_InputObject = 1902
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