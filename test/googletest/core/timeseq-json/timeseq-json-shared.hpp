#pragma once

#include <gtest/gtest.h>
#include <istream>

#include "core/timeseq-json.hpp"

using namespace timeseq;
using namespace std;

#define SCRIPT_VERSION "0.0.1"


shared_ptr<Script> loadScript(JsonLoader& jsonLoader, json& json, vector<ValidationError> *validationErrors);
json getMinimalJson();
void expectError(vector<ValidationError>& validationErrors, int errorCode, string errorLocation);
void expectNoErrors(vector<ValidationError>& validationErrors);

#define EXPECT_NO_ERRORS(validationErrors) \
	expectNoErrors(validationErrors); \
	if (::testing::Test::HasFailure()) return;
