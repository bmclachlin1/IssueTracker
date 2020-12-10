#include <ctime>
#include <string>

#include "MutableEntity.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class MutableEntityTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Set up current time to run on every test
    time_t currentTime;
    time(&currentTime);
    now = gmtime(&currentTime);
  }

  struct tm* now;
  MutableEntity entity;
};

TEST_F(MutableEntityTest, Constructor) {
  EXPECT_EQ(now->tm_mday, entity.createdAt.tm_mday);
  EXPECT_EQ(now->tm_mon, entity.createdAt.tm_mon);
  EXPECT_EQ(now->tm_year, entity.createdAt.tm_year);
  // EXPECT_EQ(nullptr, entity.updatedAt);
}

TEST_F(MutableEntityTest, ConvertStringToTime_ValidTime) {
  // This Test Fixture needs to be FIXED!
  std::string stringTime = "Wed Feb 13 15:46:11 2013";
  struct tm convertedTime = TimeUtilities::ConvertStringToTime(stringTime);

  EXPECT_EQ(convertedTime.tm_year, 113);
  EXPECT_EQ(convertedTime.tm_mday, 13);
  EXPECT_EQ(convertedTime.tm_mon, 1);
  EXPECT_EQ(convertedTime.tm_hour, 15);
  EXPECT_EQ(convertedTime.tm_min, 46);
  EXPECT_EQ(convertedTime.tm_wday, 3);
}

TEST_F(MutableEntityTest, ConvertStringToTime_EmptyStr) {
  std::string stringTime = "";
  EXPECT_THROW(TimeUtilities::ConvertStringToTime(stringTime),
               InternalServerError);
}

TEST_F(MutableEntityTest, ConvertStringToTime_InvalidString) {
  // Unix timestamp
  std::string stringTime = "1605474390";
  EXPECT_THROW(TimeUtilities::ConvertStringToTime(stringTime),
               InternalServerError);

  // ISO 8601 format
  stringTime = "2020-11-15T21:06:30+00:00";
  EXPECT_THROW(TimeUtilities::ConvertStringToTime(stringTime),
               InternalServerError);
}

TEST_F(MutableEntityTest, ConvertTimeToString_ValidTime) {
  now->tm_hour = 15;
  now->tm_min = 30;
  now->tm_sec = 11;
  now->tm_mday = 25;
  now->tm_mon = 4;
  now->tm_year = 100;
  now->tm_wday = 1;
  std::string result = TimeUtilities::ConvertTimeToString(*now);
  EXPECT_EQ("Mon May 25 15:30:11 2000", result);

  now->tm_mday = 1;
  result = TimeUtilities::ConvertTimeToString(*now);
  EXPECT_EQ("Mon May 01 15:30:11 2000", result);
}

TEST_F(MutableEntityTest, Serializer_NoUpdate) {
  entity.id = "entity";
  User user;
  user.id = "user";
  entity.createdBy = user;
  json entityJson = entity;
  EXPECT_EQ(entityJson.value("id", ""), entity.id);
  std::string createdAt = TimeUtilities::ConvertTimeToString(entity.createdAt);
  EXPECT_EQ(entityJson.value("createdAt", ""), createdAt);
  EXPECT_EQ(entityJson.value("createdBy", ""), user.id);
  // When we get the value from the json object, we want to make sure that it
  // is explicitly set to an empty string. Thus we specify the default value
  // in the .value() method to not be the empty string
  EXPECT_EQ(entityJson.value("updatedAt", "notempty"), "");
  EXPECT_EQ(entityJson.value("updatedBy", "notempty"), "");
}

TEST_F(MutableEntityTest, Serializer_HasBeenUpdated) {
  entity.id = "entity";
  User creator;
  creator.id = "creator";
  entity.createdBy = creator;

  User updater;
  updater.id = "updater";
  entity.updatedBy = updater;
  std::string updatedTime = "Wed Feb 13 15:46:11 2020";
  entity.updatedAt = TimeUtilities::ConvertStringToTime(updatedTime);

  json entityJson = entity;
  EXPECT_EQ(entityJson.value("id", ""), entity.id);
  std::string createdAt = TimeUtilities::ConvertTimeToString(entity.createdAt);
  EXPECT_EQ(entityJson.value("createdAt", ""), createdAt);
  EXPECT_EQ(entityJson.value("createdBy", ""), creator.id);
  EXPECT_EQ(entityJson.value("updatedAt", "notempty"), updatedTime);
  EXPECT_EQ(entityJson.value("updatedBy", "notempty"), updater.id);
}

TEST_F(MutableEntityTest, Deserializer_NeverUpdated) {
  std::string entityString =
      "{\"id\": \"5555\", \"createdAt\": \"Wed Feb 13 15:46:11 2020\","
      "\"createdBy\": \"1234\", \"updatedAt\": \"\", \"updatedBy\": \"\"}";

  json entityJson = json::parse(entityString);
  entity = entityJson.get<MutableEntity>();

  EXPECT_EQ(entity.id, "5555");
  EXPECT_EQ(entity.createdAt.tm_mday, 13);
  EXPECT_EQ(entity.createdAt.tm_mon, 1);
  EXPECT_EQ(entity.createdAt.tm_year, 120);
  EXPECT_EQ(entity.createdAt.tm_hour, 15);
  EXPECT_EQ(entity.createdAt.tm_min, 46);
  EXPECT_EQ(entity.createdBy.id, "1234");
  // EXPECT_EQ(entity.updatedAt, nullptr);
  EXPECT_EQ(entity.updatedBy.id, "");
}

TEST_F(MutableEntityTest, Deserializer_HasBeenUpdated) {
  std::string entityString =
      "{\"id\": \"5555\", \"createdAt\": \"Wed Feb 01 15:46:11 2020\","
      "\"createdBy\": \"1234\", \"updatedAt\": \"Fri Mar 15 09:16:11 2020\", "
      "\"updatedBy\": \"4321\"}";

  json entityJson = json::parse(entityString);
  entity = entityJson.get<MutableEntity>();

  EXPECT_EQ(entity.id, "5555");
  EXPECT_EQ(entity.createdAt.tm_mday, 1);
  EXPECT_EQ(entity.createdAt.tm_mon, 1);
  EXPECT_EQ(entity.createdAt.tm_year, 120);
  EXPECT_EQ(entity.createdAt.tm_hour, 15);
  EXPECT_EQ(entity.createdAt.tm_min, 46);
  EXPECT_EQ(entity.createdBy.id, "1234");
  EXPECT_EQ(entity.updatedAt.tm_mday, 15);
  EXPECT_EQ(entity.updatedAt.tm_mon, 2);
  EXPECT_EQ(entity.updatedAt.tm_year, 120);
  EXPECT_EQ(entity.updatedAt.tm_hour, 9);
  EXPECT_EQ(entity.updatedAt.tm_min, 16);
  EXPECT_EQ(entity.updatedBy.id, "4321");
}
