#include "core/timeseq-validation.hpp"
#include <string>
#include <sstream>
#include <stdarg.h>

using namespace timeseq;
using namespace std;

string timeseq::createValidationErrorMessage(ValidationErrorCode code, ...) {
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

string timeseq::createValidationErrorLocation(vector<string> location) {
	ostringstream errorLocation;
	for (const string& entry : location) {
		errorLocation << "/";
		errorLocation << entry;
	}

	if (errorLocation.tellp() == 0) {
		errorLocation << "/";
	}

	return errorLocation.str();
}
