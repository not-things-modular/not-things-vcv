#include "save-data.hpp"
#include <gmock/gmock.h>

void checkJsonSolimOutputMode(json_t* jsonData, SolimOutputMode expectedOutputMode) {
    EXPECT_TRUE(json_is_object(jsonData));

    json_t* jsonSolimOutputMode = json_object_get(jsonData, "ntSolimOutputMode");
    EXPECT_TRUE(json_is_integer(jsonSolimOutputMode));

    EXPECT_EQ(json_integer_value(jsonSolimOutputMode), static_cast<int>(expectedOutputMode));
}
