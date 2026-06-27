#include "core/timeseq-script-parser-internal.hpp"

ScriptIf JsonScriptParser::parseIf(const json& ifJson, bool allowRefs) {
	static const char* cIfProperties[] = { "eq", "ne", "lt", "lte", "gt", "gte", "and", "or", "tolerance" };
	static const vector<string> vIfProperties(begin(cIfProperties), end(cIfProperties));
	ScriptIf scriptIf;

	populateRef(scriptIf, ifJson, allowRefs);
	if (scriptIf.ref.length() > 0) {
		if (hasOneOf(ifJson, cIfProperties)) {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_RefOrinstance, "A ref if can not be combined other non-ref if properties.");
		}
	} else {
		int operatorCount = 0;

		verifyAllowedProperties(ifJson, vIfProperties, true, m_context);

		json::const_iterator eqValue = ifJson.find("eq");
		if (eqValue != ifJson.end()) {
			operatorCount++;
			if (eqValue->is_array()) {
				m_context.location.push_back("eq");
				scriptIf.ifOperator = ScriptIf::IfOperator::EQ;
				scriptIf.values.reset(new pair<ScriptValue, ScriptValue>(parseIfValues("eq", *eqValue)));
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_EqArray, "'eq' must be an array.");
			}
		}

		json::const_iterator neValue = ifJson.find("ne");
		if (neValue != ifJson.end()) {
			operatorCount++;
			if (neValue->is_array()) {
				m_context.location.push_back("ne");
				scriptIf.ifOperator = ScriptIf::IfOperator::NE;
				scriptIf.values.reset(new pair<ScriptValue, ScriptValue>(parseIfValues("ne", *neValue)));
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_NeArray, "'ne' must be an array.");
			}
		}

		json::const_iterator ltValue = ifJson.find("lt");
		if (ltValue != ifJson.end()) {
			operatorCount++;
			if (ltValue->is_array()) {
				m_context.location.push_back("lt");
				scriptIf.ifOperator = ScriptIf::IfOperator::LT;
				scriptIf.values.reset(new pair<ScriptValue, ScriptValue>(parseIfValues("lt", *ltValue)));
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_LtArray, "'lt' must be an array.");
			}
		}

		json::const_iterator lteValue = ifJson.find("lte");
		if (lteValue != ifJson.end()) {
			operatorCount++;
			if (lteValue->is_array()) {
				m_context.location.push_back("lte");
				scriptIf.ifOperator = ScriptIf::IfOperator::LTE;
				scriptIf.values.reset(new pair<ScriptValue, ScriptValue>(parseIfValues("lte", *lteValue)));
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_LteArray, "'lte' must be an array.");
			}
		}

		json::const_iterator gtValue = ifJson.find("gt");
		if (gtValue != ifJson.end()) {
			operatorCount++;
			if (gtValue->is_array()) {
				m_context.location.push_back("gt");
				scriptIf.ifOperator = ScriptIf::IfOperator::GT;
				scriptIf.values.reset(new pair<ScriptValue, ScriptValue>(parseIfValues("gt", *gtValue)));
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_GtArray, "'gt' must be an array.");
			}
		}

		json::const_iterator gteValue = ifJson.find("gte");
		if (gteValue != ifJson.end()) {
			operatorCount++;
			if (gteValue->is_array()) {
				m_context.location.push_back("gte");
				scriptIf.ifOperator = ScriptIf::IfOperator::GTE;
				scriptIf.values.reset(new pair<ScriptValue, ScriptValue>(parseIfValues("gte", *gteValue)));
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_GteArray, "'gte' must be an array.");
			}
		}

		json::const_iterator andValue = ifJson.find("and");
		if (andValue != ifJson.end()) {
			operatorCount++;
			if (andValue->is_array()) {
				m_context.location.push_back("and");
				scriptIf.ifOperator = ScriptIf::IfOperator::AND;
				scriptIf.ifs = parseIfIfs("and", *andValue);
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_AndArray, "'and' must be an array.");
			}
		}

		json::const_iterator orValue = ifJson.find("or");
		if (orValue != ifJson.end()) {
			operatorCount++;
			if (orValue->is_array()) {
				m_context.location.push_back("or");
				scriptIf.ifOperator = ScriptIf::IfOperator::OR;
				scriptIf.ifs = parseIfIfs("or", *orValue);
				m_context.location.pop_back();
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_OrArray, "'or' must be an array.");
			}
		}

		json::const_iterator toleranceValue = ifJson.find("tolerance");
		if (toleranceValue != ifJson.end()) {
			if (toleranceValue->is_number()) {
				if (toleranceValue->get<float>() >= 0.f) {
					scriptIf.tolerance.reset(new float(toleranceValue->get<float>()));
				} else {
					addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_ToleranceNumber, "'tolerance' must be a positive number.");
				}
			} else {
				addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_ToleranceNumber, "'tolerance' must be a positive number.");
			}
		}

		if (operatorCount == 0) {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_NoOperation, "One of 'eq', 'ne', 'lt', 'lte', 'gt', 'gte', 'and' or 'or' is required.");
		} else if (operatorCount > 1) {
			addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_MultipleOperations, "Only one of 'eq', 'ne', 'lt', 'lte', 'gt', 'gte', 'and' or 'or' is allowed.");
		}
	}

	return scriptIf;
}

pair<ScriptValue, ScriptValue> JsonScriptParser::parseIfValues(const string& ifOperator, const json& valuesJson) {
	pair<ScriptValue, ScriptValue> valuePair;

	vector<json> valueElements = valuesJson.get<vector<json>>();
	if (valueElements.size() == 2) {
		valuePair.first = parseValue(valueElements[0], true, "0", ValidationErrorCode::If_ValueObject, "'" + ifOperator + "' children must be value objects.");
		valuePair.second = parseValue(valueElements[1], true, "1", ValidationErrorCode::If_ValueObject, "'" + ifOperator + "' children must be value objects.");
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_TwoValues, "Exactly two value items are expected in the '", ifOperator.c_str(), "' array");
	}

	return valuePair;
}

unique_ptr<vector<ScriptIf>> JsonScriptParser::parseIfIfs(const string& ifOperator, const json& ifsJson) {
	unique_ptr<vector<ScriptIf>> ifs;

	vector<json> ifElements = ifsJson.get<vector<json>>();
	if (ifElements.size() > 1) {
		int count = 0;
		ifs.reset(new vector<ScriptIf>());
		for (const json& ifElement : ifElements) {
			m_context.location.push_back(to_string(count));
			ifs->push_back(parseIf(ifElement, true));
			m_context.location.pop_back();
			count++;
		}
		if (count > 2) {
			verifyVersion(VERSION_1_2_0, m_context, (ifOperator + " ifs with more then two conditions").c_str());
		}
	} else {
		addValidationError(&m_context.validationErrors, m_context.location, ValidationErrorCode::If_TwoValues, "At least two if items are expected in the '", ifOperator.c_str(), "' array");
	}

	return ifs;
}
