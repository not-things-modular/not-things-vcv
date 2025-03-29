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
	Script_GlobalActionsArray = 129,
	Script_GlobalActionsObject = 130,
	Script_GlobalActionsItemRequired = 131,
	Script_GlobalActionTiming = 132,

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
	Lane_RestartTriggerString = 305,
	Lane_RestartTriggerLength = 306,
	Lane_StopTriggerString = 307,
	Lane_StopTriggerLength = 308,
	Lane_SegmentsMissing = 309,
	Lane_SegmentObject = 310,
	Lane_DisableUiBoolean = 311,

	If_EqArray = 400,
	If_NeArray = 401,
	If_LtArray = 402,
	If_LteArray = 403,
	If_GtArray = 404,
	If_GteArray = 405,
	If_AndArray = 406,
	If_OrArray = 407,
	If_NoOperation = 408,
	If_MultpleOperations = 409,
	If_TwoValues = 410,
	If_ToleranceNumber = 411,

	Segment_RefOrInstance = 500,
	Segment_DurationObject = 501,
	Segment_ActionsArray = 502,
	Segment_ActionObject = 503,
	Segment_SegmentBlockString = 504,
	Segment_SegmentBlockLength = 505,
	Segment_BlockOrSegment = 506,
	Segment_DisableUiBoolean = 507,

	SegmentBlock_RefOrInstance = 600,
	SegmentBlock_SegmentsArray = 601,
	SegmentBlock_SegmentObject = 602,
	SegmentBlock_RepeatNumber = 603,

	Duration_SamplesNumber = 700,
	Duration_MillisNumber = 701,
	Duration_BarsNumber = 702,
	Duration_BeatsNumber = 703,
	Duration_HzNumber = 704,
	Duration_NoSamplesOrMillisOrBars = 705,
	Duration_EitherSamplesOrMillisOrBars = 706,
	Duration_BarsRequiresBeats = 707,
	Duration_BeatsButNoBmp = 708,
	Duration_BarsButNoBpb = 709,
	Duration_BeatsNotZero = 710,

	Action_RefOrInstance = 800,
	Action_TimingEnum = 801,
	Action_SetValueObject = 802,
	Action_SetVariableObject = 803,
	Action_SetPolyphonyObject = 804,
	Action_TriggerString = 805,
	Action_TriggerLength = 806,
	Action_StartValueObject = 807,
	Action_EndValueObject = 808,
	Action_EaseFactorFloat = 809,
	Action_EaseFactorRange = 810,
	Action_EaseAlgorithmEnum = 810,
	Action_OutputObject = 812,
	Action_VariableString = 813,
	Action_VariableLength = 814,
	Action_IfObject = 815,
	Action_NonGlideProperties = 816,
	Action_MissingGlideValues = 817,
	Action_MissingGlideActions = 818,
	Action_TooManyGlideActions = 819,
	Action_OnlyGlideProperties = 820,
	Action_MissingNonGlideProperties = 821,

	SetValue_OutputObject = 900,
	SetValue_ValueObject = 901,

	SetVariable_NameString = 1000,
	SetVariable_ValueObject = 1001,

	SetPolyphony_IndexNumber = 1100,
	SetPolyphony_IndexRange = 1101,
	SetPolyphony_ChannelsNumber = 1102,
	SetPolyphony_ChannelsRange = 1103,

	Value_RefOrInstance = 1200,
	Value_NoActualValue = 1202,
	Value_MultipleValues = 1203,
	Value_VoltageFloat = 1204,
	Value_VoltageRange = 1205,
	Value_VariableString = 1206,
	Value_VariableNonEmpty = 1207,
	Value_NoteString = 1208,
	Value_NoteFormat = 1209,
	Value_InputObject = 1210,
	Value_OutputObject = 1211,
	Value_RandObject = 1212,
	Value_CalcArray = 1213,
	Value_CalcObject = 1214,
	Value_QuantizeBool = 1215,

	Output_RefOrInstance = 1300,
	Output_IndexNumber = 1301,
	Output_IndexRange = 1302,
	Output_ChannelNumber = 1303,
	Output_ChannelRange = 1304,

	Input_RefOrInstance = 1400,
	Input_IndexNumber = 1401,
	Input_IndexRange = 1402,
	Input_ChannelNumber = 1403,
	Input_ChannelRange = 1404,

	Rand_LowerObject = 1500,
	Rand_UpperObject = 1501,

	Calc_NoOperation = 1600,
	Calc_MultpleOperations = 1601,
	Calc_AddObject = 1602,
	Calc_SubObject = 1603,
	Calc_DivObject = 1604,
	Calc_MultObject = 1605,

	InputTrigger_IdString = 1700,
	InputTrigger_IdLength = 1701,
	InputTrigger_InputObject = 1702
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