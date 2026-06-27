#include "core/timeseq-processor.hpp"
#include "core/timeseq-script.hpp"
#include "core/timeseq-core.hpp"

using namespace std;
using namespace timeseq;


IfProcessor::IfProcessor(const ScriptIf* scriptIf, const pair<const shared_ptr<ValueProcessor>, const shared_ptr<ValueProcessor>>& values, const vector<shared_ptr<IfProcessor>>& ifs) : m_scriptIf(scriptIf), m_values(values), m_ifs(ifs) {}

bool IfProcessor::process(string* message) {
	if (message == nullptr) {
		// If no message needs to be returned upon failure, we can do a simple check for the different operator types
		switch (m_scriptIf->ifOperator) {
			case ScriptIf::IfOperator::EQ: {
				if (m_scriptIf->tolerance) {
					return fabs(m_values.first->process() - m_values.second->process()) <= *m_scriptIf->tolerance.get();
				} else {
					return m_values.first->process() == m_values.second->process();
				}
			}
			case ScriptIf::IfOperator::NE: {
				if (m_scriptIf->tolerance) {
					return fabs(m_values.first->process() - m_values.second->process()) > *m_scriptIf->tolerance.get();
				} else {
					return m_values.first->process() != m_values.second->process();
				}
			}
			case ScriptIf::IfOperator::LT:
				return m_values.first->process() < m_values.second->process();
			case ScriptIf::IfOperator::LTE:
				return m_values.first->process() <= m_values.second->process();
			case ScriptIf::IfOperator::GT:
				return m_values.first->process() > m_values.second->process();
			case ScriptIf::IfOperator::GTE:
				return m_values.first->process() >= m_values.second->process();
			case ScriptIf::IfOperator::AND:
				for (const shared_ptr<IfProcessor>& condition : m_ifs) {
					if (!condition->process(nullptr)) {
						return false;
					}
				}
				return true;
			case ScriptIf::IfOperator::OR:
				for (const shared_ptr<IfProcessor>& condition : m_ifs) {
					if (condition->process(nullptr)) {
						return true;
					}
				}
				return false;
		}

		return false;
	} else {
		// We'll need to return the details of the comparison if it failed, so we'll need to do some additional work...
		ostringstream oss;
		oss.precision(10);
		if (m_scriptIf->ifOperator == ScriptIf::IfOperator::AND) {
			bool result = true;
			bool addAnd = false;
			oss << "(";
			for (const shared_ptr<IfProcessor>& condition : m_ifs) {
				string message;
				if (!condition->process(&message)) {
					result = false;
				}

				if (addAnd) {
					oss << " and ";
				} else {
					addAnd = true;
				}
				oss << message;
			}

			oss << ")";
			*message = oss.str();

			return result;
		} else if (m_scriptIf->ifOperator == ScriptIf::IfOperator::OR) {
			bool result = false;
			bool addOr = false;
			oss << "(";
			for (const shared_ptr<IfProcessor>& condition : m_ifs) {
				string message;
				if (condition->process(&message)) {
					result = true;
				}

				if (addOr) {
					oss << " or ";
				} else {
					addOr = true;
				}
				oss << message;
			}

			oss << ")";
			*message = oss.str();

			return result;
		} else {
			double value1 = m_values.first->process();
			double value2 = m_values.second->process();
			string operatorName;
			bool result = false;

			switch (m_scriptIf->ifOperator) {
				case ScriptIf::IfOperator::EQ: {
					if (m_scriptIf->tolerance) {
						result = fabs(value1 - value2) <= *m_scriptIf->tolerance.get();
					} else {
						result = value1 == value2;
					}
					operatorName = " eq ";
					break;
				}
				case ScriptIf::IfOperator::NE: {
					if (m_scriptIf->tolerance) {
						result = fabs(value1 - value2) > *m_scriptIf->tolerance.get();
					} else {
						result = value1 != value2;
					}
					operatorName = " ne ";
					break;
				}
				case ScriptIf::IfOperator::LT: {
					result = value1 < value2;
					operatorName = " lt ";
					break;
				}
				case ScriptIf::IfOperator::LTE: {
					result = value1 <= value2;
					operatorName = " lte ";
					break;
				}
				case ScriptIf::IfOperator::GT: {
					result = value1 > value2;
					operatorName = " gt ";
					break;
				}
				case ScriptIf::IfOperator::GTE: {
					result = value1 >= value2;
					operatorName = " gte ";
					break;
				}
				case ScriptIf::IfOperator::AND:
				case ScriptIf::IfOperator::OR: {
						// Shouldn't come here anymore, AND and OR have been handled earlier
					break;
				}
			}

			oss << "(" << value1 << operatorName << value2 << ")";
			*message = oss.str();

			return result;
		}
	}
}