#include "core/timeseq-script-parser-internal.hpp"

ScriptSegment JsonScriptParser::parseSegment(const json& segmentJson, bool allowRefs) {
	static const char* cSegmentProperties[] = { "duration", "actions", "disable-ui", "segment-block" };
	static const vector<string> vSegmentProperties(begin(cSegmentProperties), end(cSegmentProperties));
	ScriptSegment segment;

	verifyAllowedProperties(segmentJson, vSegmentProperties, true, m_context);

	populateRef(segment, segmentJson, allowRefs);
	if (segment.ref.length() > 0) {
		if (hasOneOf(segmentJson, cSegmentProperties)) {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Segment_RefOrInstance, "A ref segment can not be combined other non-ref segment properties.");
		}
	} else {
		static const char* segmentBlockProperty[] = { "segment-block" };
		static const char* nonSegmentBlockProperties[] = { "duration", "disable-ui" };
		if (!hasOneOf(segmentJson, segmentBlockProperty)) {
			json::const_iterator duration = segmentJson.find("duration");
			if ((duration != segmentJson.end()) && (duration->is_object())) {
				m_context.location.push_back("duration");
				segment.duration = parseDuration(*duration);
				m_context.location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Segment_DurationObject, "'duration' is required and must be an object.");
			}

			segment.disableUi = false;
			json::const_iterator disableUi = segmentJson.find("disable-ui");
			if (disableUi != segmentJson.end()) {
				if (disableUi->is_boolean()) {
					segment.disableUi = disableUi->get<bool>();
				} else {
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Segment_DisableUiBoolean, "'disable-ui' must be a boolean.");
				}
			}
		} else if (!hasOneOf(segmentJson, nonSegmentBlockProperties)) {
			json::const_iterator segmentBlock = segmentJson.find("segment-block");
			if (segmentBlock != segmentJson.end()) {
				if (segmentBlock->is_string()) {
					string segmentBlockRef = segmentBlock->get<string>();
					if (segmentBlockRef.length() > 0) {
						segment.segmentBlock.reset(new ScriptSegmentBlock());
						segment.segmentBlock->ref = segmentBlockRef;
					} else {
						ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Segment_SegmentBlockLength, "'segment-block' must be a non-empty string");
					}
				} else {
					ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Segment_SegmentBlockString, "'segment-block' must be a non-empty string");
				}
			}
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Segment_BlockOrSegment, "A segment must either be a single segment with a 'duration' and 'actions', or a segment block with a 'segment-block' reference and optional 'actions', but not both.");
		}

		json::const_iterator actions = segmentJson.find("actions");
		if (actions != segmentJson.end()) {
			if (actions->is_array()) {
				m_context.location.push_back("actions");

				int count = 0;
				vector<json> actionElements = (*actions);
				for (const json& action : actionElements) {
					m_context.location.push_back(to_string(count));
					if (action.is_object()) {
						segment.actions.push_back(parseAction(action, true));
					} else {
						ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Segment_ActionObject, "'actions' elements must be objects.");
					}
					m_context.location.pop_back();
					count++;
				}

				m_context.location.pop_back();
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Segment_ActionsArray, "'actions' must be an array.");
			}
		}
	}

	return segment;
}

ScriptSegmentBlock JsonScriptParser::parseSegmentBlock(const json& segmentBlockJson) {
	static const vector<string> segmentBlockProperties = { "repeat", "segments" };
	ScriptSegmentBlock segmentBlock;

	verifyAllowedProperties(segmentBlockJson, segmentBlockProperties, true, m_context); // Refs aren't really allowed, but that will be caught by the populateRef method.

	populateRef(segmentBlock, segmentBlockJson, false);

	json::const_iterator repeat = segmentBlockJson.find("repeat");
	if (repeat != segmentBlockJson.end()) {
		if ((repeat->is_number_unsigned()) && (repeat->get<int>() > 0)) {
			segmentBlock.repeat.reset(new int(repeat->get<int>()));
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::SegmentBlock_RepeatNumber, "'repeat' must be a positive number.");
		}
	}
	json::const_iterator segments = segmentBlockJson.find("segments");
	if ((segments != segmentBlockJson.end()) && (segments->is_array())) {
		m_context.location.push_back("segments");

		int count = 0;
		vector<json> segmentElements = (*segments);
		for (const json& segment : segmentElements) {
			m_context.location.push_back(to_string(count));
			if (segment.is_object()) {
				segmentBlock.segments.push_back(parseSegment(segment, true));
			} else {
				ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::SegmentBlock_SegmentObject, "'segments' elements must be Segment objects.");
			}
			m_context.location.pop_back();
			count++;
		}

		m_context.location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::SegmentBlock_SegmentsArray, "'segments' is required and must be an array.");
	}

	return segmentBlock;
}

ScriptDuration JsonScriptParser::parseDuration(const json& durationJson) {
	static const vector<string> durationProperties = { "samples", "millis", "bars", "beats", "hz" };
	ScriptDuration duration;
	int durationCount = 0;

	verifyAllowedProperties(durationJson, durationProperties, false, m_context);

	json::const_iterator samples = durationJson.find("samples");
	if (samples != durationJson.end()) {
		durationCount++;
		if ((samples->is_number_unsigned()) && (samples->get<uint64_t>() > 0)) {
			duration.samples.reset(new uint64_t(samples->get<uint64_t>()));
		} else if (samples->is_object()) {
			ScriptValue *scriptValue = new ScriptValue(parseValue(*samples, true, "samples", ValidationErrorCode::Duration_SamplesNumberOrValue, "'samples' must be an object."));
			duration.samplesValue.reset(scriptValue);
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_SamplesNumberOrValue, "'samples' must be a positive integer number or a value object.");
		}
	}

	json::const_iterator millis = durationJson.find("millis");
	if (millis != durationJson.end()) {
		durationCount++;
		if ((millis->is_number()) && (millis->get<float>() > 0)) {
			duration.millis.reset(new float(millis->get<float>()));
		} else if (millis->is_object()) {
			ScriptValue *scriptValue = new ScriptValue(parseValue(*millis, true, "millis", ValidationErrorCode::Duration_MillisNumberOrValue, "'millis' must be an object."));
			duration.millisValue.reset(scriptValue);
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_MillisNumberOrValue, "'millis' must be a positive decimal number or a value object.");
		}
	}

	json::const_iterator bars = durationJson.find("bars");
	if (bars != durationJson.end()) {
		if ((bars->is_number_unsigned()) && (bars->get<uint64_t>() > 0)) {
			duration.bars.reset(new uint64_t(bars->get<uint64_t>()));
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_BarsNumber, "'bars' must be a positive integer number or a value object.");
		}
	}

	json::const_iterator beats = durationJson.find("beats");
	if (beats != durationJson.end()) {
		durationCount++;
		if ((beats->is_number()) && (beats->get<float>() >= 0)) {
			duration.beats.reset(new float(beats->get<float>()));
		} else if (beats->is_object()) {
			ScriptValue *scriptValue = new ScriptValue(parseValue(*beats, true, "beats", ValidationErrorCode::Duration_BeatsNumberOrValue, "'beats' must be an object."));
			duration.beatsValue.reset(scriptValue);
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_BeatsNumberOrValue, "'beats' must be a positive decimal number or a value object.");
		}
	}

	json::const_iterator hz = durationJson.find("hz");
	if (hz != durationJson.end()) {
		durationCount++;
		if ((hz->is_number()) && (hz->get<float>() > 0)) {
			duration.hz.reset(new float(hz->get<float>()));
		} else if (hz->is_object()) {
			ScriptValue *scriptValue = new ScriptValue(parseValue(*hz, true, "hz", ValidationErrorCode::Duration_HzNumberOrValue, "'hz' must be an object."));
			duration.hzValue.reset(scriptValue);
		} else {
			ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_HzNumberOrValue, "'hz' must be a positive decimal number or a value object.");
		}
	}

	if (durationCount == 0) {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_NoSamplesOrMillisOrBeatsOrHz, "either 'samples', 'millis', 'beats' or 'hz' must be used.");
	} else if (durationCount > 1) {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_EitherSamplesOrMillisOrBeatsOrHz, "only one of 'samples', 'millis', 'beats' or 'hz' can be used at a time.");
	} else if (duration.bars && duration.beatsValue) {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_BarsRequiresBeats, "'bars' can only be used with a constant 'beats', not with a value-based 'beats'.");
	} else if (duration.bars && !duration.beats) {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_BarsRequiresBeats, "'bars' can not be used without 'beats'.");
	} else if ((!duration.bars) && (duration.beats) && (*duration.beats.get() == 0)) {
		ADD_VALIDATION_ERROR(&m_context.validationErrors, m_context.location, ValidationErrorCode::Duration_BeatsNotZero, "'beats' can not be 0 unless 'bars' is also used.");
	}

	return duration;
}
