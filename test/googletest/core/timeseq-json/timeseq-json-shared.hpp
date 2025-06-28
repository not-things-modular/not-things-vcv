#pragma once

#include <gtest/gtest.h>
#include <istream>

#include "core/timeseq-json.hpp"

using namespace timeseq;
using namespace std;

#define SCRIPT_VERSION_1_0_0 "1.0.0"
#define SCRIPT_VERSION_1_1_0 "1.1.0"


shared_ptr<Script> loadScript(JsonLoader& jsonLoader, json& json, vector<ValidationError> *validationErrors);
json getMinimalJson();
json getMinimalJson(const char* scriptVersion);
void expectError(vector<ValidationError>& validationErrors, int errorCode, string errorLocation);
void expectNoErrors(vector<ValidationError>& validationErrors);

#define EXPECT_NO_ERRORS(validationErrors) \
	expectNoErrors(validationErrors); \
	if (::testing::Test::HasFailure()) return;
