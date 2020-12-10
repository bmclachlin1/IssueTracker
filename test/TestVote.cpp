#include <string>

#include "User.h"
#include "Utilities.h"
#include "Vote.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

TEST(TestVote, Serialize) {
  Vote vote;
  std::tm now = {0};
  now.tm_hour = 15;
  now.tm_min = 30;
  now.tm_sec = 11;
  now.tm_mday = 25;
  now.tm_mon = 4;
  now.tm_year = 100;
  now.tm_wday = 1;
  User created = User();
  User updated = User();
  created.id = "hussjess";
  updated.id = "memleakcity";
  vote.id = "dranvikspecial";
  vote.createdAt = now;
  vote.createdBy = created;
  vote.updatedAt = now;
  vote.updatedBy = updated;
  vote.issueId = "Steven8thPlace";
  json j = vote;
  EXPECT_EQ("dranvikspecial", j["id"]);
  EXPECT_EQ("Mon May 25 15:30:11 2000", j["createdAt"]);
  EXPECT_EQ("hussjess", j["createdBy"]);
  EXPECT_EQ("Mon May 25 15:30:11 2000", j["updatedAt"]);
  EXPECT_EQ("memleakcity", j["updatedBy"]);
  EXPECT_EQ("Steven8thPlace", j["issueId"]);
}

TEST(TestVote, Deserialize_ValidVote) {
  json j =
      "{\"id\":\"abcdefgh23\",\"createdAt\":\"Mon May 25 15:30:11 2000\","
      "\"createdBy\":\"hussjess\",\"updatedAt\":\"Mon May 25 15:30:11 2000\","
      "\"updatedBy\":\"memleakcity\",\"issueId\":\"Steven8thPlace\"}"_json;
  auto entity = j.get<Vote>();
  EXPECT_EQ("abcdefgh23", entity.id);
  auto createdAt = TimeUtilities::ConvertTimeToString(entity.createdAt);
  auto updatedAt = createdAt;
  EXPECT_EQ("Mon May 25 15:30:11 2000", createdAt);
  EXPECT_EQ("hussjess", entity.createdBy.id);
  EXPECT_EQ("Mon May 25 15:30:11 2000", updatedAt);
  EXPECT_EQ("memleakcity", entity.updatedBy.id);
  EXPECT_EQ("Steven8thPlace", entity.issueId);
}

TEST(TestVote, Deserialize_MissingIssueIdField) {
  json j = "{\"createdBy\":\"hussjess\"}"_json;
  // issueId is a required field, so it should throw if we don't have one
  EXPECT_THROW(j.get<Vote>(), std::exception);

  j["issueId"] = "some id";
  auto vote = j.get<Vote>();
  EXPECT_EQ("some id", vote.issueId);
}
