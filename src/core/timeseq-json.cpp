#include "core/timeseq-json.hpp"
#include <sstream>
#include <stdarg.h>

using namespace std;


#define ADD_VALIDATION_ERROR(validationErrors, location, code, message...) \
	if (validationErrors != nullptr) { \
		string errorLocation = createValidationErrorLocation(location); \
		string errorMessage = createValidationErrorMessage(code, message, ""); \
		validationErrors->emplace_back(errorLocation, errorMessage); \
	}

class ValidationErrorHandler : public json_schema::error_handler {
	public:
		ValidationErrorHandler(vector<timeseq::JsonValidationError> *validationErrors) {
			m_validationErrors = validationErrors;
		}

		void error(const json::json_pointer &ptr, const json &instance, const string &message) override
		{
			if (m_validationErrors != nullptr) {
				string location = ptr.to_string();
				m_validationErrors->emplace_back(location, message);
			}
		}

	private:
		vector<timeseq::JsonValidationError> *m_validationErrors;
};

string createValidationErrorMessage(timeseq::ValidationErrorCode code, ...) {
	ostringstream errorMessage;

	va_list args;
	va_start(args, code);
	char* message = va_arg(args, char*);
	while (message[0] != 0) {
		errorMessage << message;
		message = va_arg(args, char*);
	}
	va_end(args);

	errorMessage << " [" << code << "]";

	return errorMessage.str();
}

string createValidationErrorLocation(vector<string> location) {
	ostringstream errorLocation;
	for (const string& entry : location) {
		if (errorLocation.tellp() != 0) {
			errorLocation << "/";
		}
		errorLocation << entry;
	}

	if (errorLocation.tellp() == 0) {
		errorLocation << "/";
	}

	return errorLocation.str();
}


void timeseq::JsonLoader::setSchema(shared_ptr<json> schema) {
	m_validator.set_root_schema(*schema);
}

shared_ptr<json> timeseq::JsonLoader::loadJson(istream& inputStream, bool validate, vector<JsonValidationError> *validationErrors) {
	shared_ptr<json> json;

	try {
		json = make_shared<nlohmann::json>(json::parse(inputStream));

		// if (validate) {
		// 	ValidationErrorHandler errorHandler(validationErrors);
		// 	m_validator.validate(*json, errorHandler);
		// }

		if (validationErrors != nullptr) {
			parseScript(*json, validationErrors, vector<string>());
		}
	} catch (const json::parse_error& error) {
		if (validationErrors != nullptr) {
			string location = "/";
			string message = error.what();
			validationErrors->emplace_back(location, message);
		}
	}

	return json;
}

timeseq::Script timeseq::JsonLoader::parseScript(const json& scriptJson, vector<JsonValidationError> *validationErrors, vector<string> location) {
	Script script;

	json::const_iterator type = scriptJson.find("type");
	if ((type == scriptJson.end()) || (!type->is_string())) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TypeMissing, "type is a required and must be a string.");
	}
	else if ((*type) != "not-things_timeseq_script") {
		std::string typeValue = (*type);
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TypeUnsupported, "type '", typeValue.c_str(), "' is not supported.");
	}
	
	json::const_iterator version = scriptJson.find("version");
	if ((version == scriptJson.end()) || (!version->is_string())) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_VersionMissing, "version is a required and must be a string.");
	}
	else if ((*version) != "0.0.1") {
		std::string versionValue = (*version);
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_VersionUnsupported, "version '", versionValue.c_str(), "' is not supported.");
	}

	json::const_iterator timelines = scriptJson.find("timelines");
	if ((timelines != scriptJson.end()) && (timelines->is_array())) {
		location.push_back("timelines");

		int count = 0;
		std::vector<json> timelineElements = (*timelines);
		for (const json& timeline : timelineElements) {
			location.push_back(std::to_string(count));
			script.timelines.push_back(parseTimeline(timeline, validationErrors, location));
			location.pop_back();
			count++;
		}

		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Script_TimelinesMissing, "timelines is required and must be an array.");
	}

	return script;
}

timeseq::ScriptTimeline timeseq::JsonLoader::parseTimeline(const json& timelineJson, std::vector<timeseq::JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptTimeline timeline;

	json::const_iterator timeScale = timelineJson.find("time-scale");
	if ((timeScale != timelineJson.end()) && (timeScale->is_object())) {
		location.push_back("time-scale");
		timeline.timeScale = parseTimeScale(*timeScale, validationErrors, location);
		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Timeline_TimeScaleMissing, "time-scale is a required and must be a time-scale object.");
	}

	json::const_iterator lanes = timelineJson.find("lanes");
	if ((lanes != timelineJson.end()) && (lanes->is_array())) {
		location.push_back("lanes");

		int count = 0;
		std::vector<json> laneElements = (*lanes);
		for (const json& lane : laneElements) {
			location.push_back(std::to_string(count));
			timeline.lanes.push_back(parseLane(lane, validationErrors, location));
			location.pop_back();
			count++;
		}

		location.pop_back();
	} else {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Timeline_LanesMissing, "lanes is a required and must be an array.");
	}

	json::const_iterator loopLock = timelineJson.find("loop-lock");
	if (loopLock != timelineJson.end()) {
		if (!loopLock->is_boolean()) {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::Timeline_LoopLockBoolean, "looplock must be a boolean.");
		} else {
			timeline.loopLock = (*loopLock);
		}
	}

	return timeline;
}

timeseq::ScriptTimeScale timeseq::JsonLoader::parseTimeScale(const json& timeScaleJson, std::vector<timeseq::JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptTimeScale timeScale;

	json::const_iterator sampleRate = timeScaleJson.find("sample-rate");
	if (sampleRate != timeScaleJson.end()) {
		if (sampleRate->is_number_unsigned()) {
			timeScale.sampleRate.reset(new int(*sampleRate));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_SampleRateNumber, "sample-rate must be an unsigned number.");
		}
	}

	json::const_iterator bpm = timeScaleJson.find("bpm");
	if (bpm != timeScaleJson.end()) {
		if (bpm->is_number_unsigned()) {
			timeScale.bpm.reset(new int(*bpm));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_BpmNumber, "bpm must be an unsigned number.");
		}
	}

	json::const_iterator bpb = timeScaleJson.find("bpb");
	if (bpb != timeScaleJson.end()) {
		if (bpb->is_number_unsigned()) {
			timeScale.bpb.reset(new int(*bpb));
		} else {
			ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_BpbNumber, "bpb must be an unsigned number.");
		}
	}

	if (!(timeScale.sampleRate) && !(timeScale.bpm)) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_SampleRateOnly, "One of sample-rate or bpm is required.");
	}
	if ((timeScale.sampleRate) && (timeScale.bpm)) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_SampleRateOnly, "No bpm can be set if sample-rate is set.");
	} else if ((timeScale.bpb) && !(timeScale.bpm)) {
		ADD_VALIDATION_ERROR(validationErrors, location, ValidationErrorCode::TimeScale_BpbRequiresBpm, "bpm  must be set if bpb is set.");
	}

	return timeScale;
}

timeseq::ScriptLane timeseq::JsonLoader::parseLane(const json& laneJson, std::vector<timeseq::JsonValidationError> *validationErrors, std::vector<std::string> location) {
	ScriptLane lane;

	return lane;
}