#include "timeseq-script-parser.hpp"

using namespace std;
using namespace timeseq;
using namespace nlohmann;

#define VERSION_1_0_0 100
#define VERSION_1_1_0 110
#define VERSION_1_2_0 120
#define VERSION_1_3_0 130

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

