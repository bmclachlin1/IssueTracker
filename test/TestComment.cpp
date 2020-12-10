#include <string>

#include "Comment.h"
#include "User.h"
#include "Utilities.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

TEST(TestComment, serialize) {
  Comment comment;
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
  comment.id = "dranvikspecial";
  comment.issueId = "Steven8thPlace";
  comment.body = "Example body content";
  comment.createdAt = now;
  comment.createdBy = created;
  comment.updatedAt = now;
  comment.updatedBy = updated;

  json j = comment;
  EXPECT_EQ("dranvikspecial", j["id"]);
  EXPECT_EQ("Mon May 25 15:30:11 2000", j["createdAt"]);
  EXPECT_EQ("hussjess", j["createdBy"]);
  EXPECT_EQ("Mon May 25 15:30:11 2000", j["updatedAt"]);
  EXPECT_EQ("memleakcity", j["updatedBy"]);
  EXPECT_EQ("Steven8thPlace", j["issueId"]);
  EXPECT_EQ("Example body content", j["body"]);
}

TEST(TestComment, deserialize) {
  json j =
      "{\"id\":\"abcdefgh23\",\"createdAt\":\"Mon May 25 15:30:11 2000\","
      "\"createdBy\":\"hussjess\",\"updatedAt\":\"Mon May 25 15:30:11 2000\","
      "\"updatedBy\":\"memleakcity\",\"issueId\":\"Steven8thPlace\","
      "\"body\":\"Example body content\"}"_json;
  auto entity = j.get<Comment>();
  EXPECT_EQ("abcdefgh23", entity.id);
  auto createdAt = TimeUtilities::ConvertTimeToString(entity.createdAt);
  auto updatedAt = createdAt;
  EXPECT_EQ("Mon May 25 15:30:11 2000", createdAt);
  EXPECT_EQ("hussjess", entity.createdBy.id);
  EXPECT_EQ("Mon May 25 15:30:11 2000", updatedAt);
  EXPECT_EQ("memleakcity", entity.updatedBy.id);
  EXPECT_EQ("Steven8thPlace", entity.issueId);
  EXPECT_EQ("Example body content", entity.body);
}
