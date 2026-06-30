#pragma once

#include <gtest/gtest.h>
#include <istream>

#include "core/timeseq-script-parser.hpp"
#include "core/timeseq-processor-parser.hpp"

using namespace nlohmann;
using namespace nlohmann::literals;
using namespace timeseq;
using namespace std;

#define SCRIPT_VERSION_1_0_0 "1.0.0"
#define SCRIPT_VERSION_1_1_0 "1.1.0"
#define SCRIPT_VERSION_1_2_0 "1.2.0"
#define SCRIPT_VERSION_1_3_0 "1.3.0"


shared_ptr<Script> loadScript(JsonLoader& jsonLoader, nlohmann::json& json, vector<ValidationError>& validationErrors);
nlohmann::json getMinimalJson();
nlohmann::json getMinimalJson(const char* scriptVersion);
void expectError(vector<ValidationError>& validationErrors, int errorCode, string errorLocation);
void expectNoErrors(vector<ValidationError>& validationErrors);

#define EXPECT_NO_ERRORS(validationErrors) \
	expectNoErrors(validationErrors); \
	if (::testing::Test::HasFailure()) return;

#define PRINT_VALIDATION_ERRORS(validationErrors) \
	for

struct ValidationErrorsPrinter
{
    const std::vector<ValidationError>& errors;
};
std::ostream& operator<<(std::ostream& os, const ValidationErrorsPrinter& printer);
inline ValidationErrorsPrinter printValidationErrors(const std::vector<ValidationError>& errors)
{
    return { errors };
}