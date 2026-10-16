#include "timeseq-script-parser.hpp"

using namespace std;
using namespace timeseq;
using namespace nlohmann;

#define VERSION_1_0_0 100
#define VERSION_1_1_0 110
#define VERSION_1_2_0 120
#define VERSION_1_3_0 130
#define VERSION_1_4_0 140

void verifyVersion(int expectedVersion, JsonScriptParseContext& context, const char* feature);
bool verifyAllowedProperties(const json& json, const vector<string>& propertyNames, bool allowRef, JsonScriptParseContext& context);
ScriptSequenceMoveDirection parseScriptSequenceMoveDirection(const json& moveDirectionJson, const char* property, ValidationErrorCode enumErrorCode, ValidationErrorCode stringErrorCode, JsonScriptParseContext& context);

template<size_t N>
bool hasOneOf(const json& json, const char* (&propertyNames)[N]) {
	for (const char* propertyName : propertyNames) {
		if (json.find(propertyName) != json.end()) {
			return true;
		}
	}
	return false;
}

template<class ScriptType, bool HasUniqueId>
struct UniqueIdChecker;

template<class ScriptType>
struct UniqueIdChecker<ScriptType, false> {
	static void checkUniqueId(JsonScriptParseContext& context, vector<string>& ids, const ScriptType& item) {
		// Nothing to do if there is no id
	}
};

template<class ScriptType>
struct UniqueIdChecker<ScriptType, true> {
	static void checkUniqueId(JsonScriptParseContext& context, vector<string>& ids, const ScriptType& item) {
		if (find(ids.begin(), ids.end(), item.id) != ids.end()) {
			addValidationError(&context.validationErrors, context.location, ValidationErrorCode::Id_Duplicate, "Id '", item.id.c_str(), "' has already been used. Ids must be unique within the object type.");
		} else if (item.id.size() > 0) {
			ids.push_back(item.id);
		}
	}
};

template<class ScriptType, bool hasUniqueId>
void parseChildArray(JsonScriptParseContext& context, const json& parent, const std::string jsonTag, int version, vector<ScriptType>& scriptArray, const std::function<ScriptType(const json&)> parseFunc, ValidationErrorCode objectErrorCode, ValidationErrorCode arrayErrorCode, ValidationErrorCode requiredErrorCode) {
	json::const_iterator items = parent.find(jsonTag);
	if (items != parent.end()) {
		if (version > 0) {
			verifyVersion(version, context, (std::string("'") + jsonTag + "'").c_str());
		}
		if (items->is_array()) {
			context.location.push_back(jsonTag);

			int count = 0;
			vector<string> ids;
			vector<json> elements = (*items);
			for (const json& element : elements) {
				context.location.push_back(to_string(count));
				if (element.is_object()) {
					scriptArray.push_back(parseFunc(element));
					UniqueIdChecker<ScriptType, hasUniqueId>::checkUniqueId(context, ids, scriptArray.back());
				} else {
					addValidationError(&context.validationErrors, context.location, objectErrorCode, "'", jsonTag.c_str(), "' elements must be objects.");
				}
				context.location.pop_back();
				count++;
			}

			context.location.pop_back();
		} else {
			addValidationError(&context.validationErrors, context.location, arrayErrorCode, "'", jsonTag.c_str(), "' must be an array.");
		}
	} else if (requiredErrorCode != ValidationErrorCode::NoError) {
		addValidationError(&context.validationErrors, context.location, requiredErrorCode, "'", jsonTag.c_str(), "' is required and must be an array.");
	}
}
