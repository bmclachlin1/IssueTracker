#include <set>
#include <string>
#include <vector>

#include "Comment.h"
#include "Issue.h"
#include "User.h"
#include "Utilities.h"
#include "Vote.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

TEST(TestIssue, Serialize) {
  std::multiset<Comment> comments;
  for (unsigned int i = 0; i < 5; i++) {
    Comment comment;
    comment.id = std::to_string(i);
    comment.createdAt.tm_hour = i;
    comments.insert(comment);
  }

  std::vector<Vote> votes;
  for (unsigned int i = 0; i < 5; i++) {
    Vote vote;
    vote.id = std::to_string(i);
    votes.push_back(vote);
  }

  Issue issue;
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
  User reporter = User();
  User assignedTo = User();
  created.id = "hussjess";
  updated.id = "memleakcity";
  reporter.id = "Reporter Example ID";
  assignedTo.id = "Assigned Example User ID";

  issue.id = "dranvikspecial";
  issue.title = "Example title content";
  issue.status = "Example status content";
  issue.createdAt = now;
  issue.createdBy = created;
  issue.updatedAt = now;
  issue.updatedBy = updated;

  issue.assignedTo = assignedTo;
  issue.reporter = reporter;

  issue.comments = comments;
  issue.votes = votes;

  json j = issue;
  EXPECT_EQ("dranvikspecial", j["id"]);
  EXPECT_EQ("Mon May 25 15:30:11 2000", j["createdAt"]);
  EXPECT_EQ("hussjess", j["createdBy"]);
  EXPECT_EQ("Mon May 25 15:30:11 2000", j["updatedAt"]);
  EXPECT_EQ("memleakcity", j["updatedBy"]);
  EXPECT_EQ("Example title content", j["title"]);
  EXPECT_EQ("Example status content", j["status"]);
  EXPECT_EQ("Reporter Example ID", j["reporter"]);
  EXPECT_EQ("Assigned Example User ID", j["assignedTo"]);

  EXPECT_EQ("0", j["comments"][0].get<std::string>());
  EXPECT_EQ("1", j["comments"][1].get<std::string>());
  EXPECT_EQ("2", j["comments"][2].get<std::string>());
  EXPECT_EQ("3", j["comments"][3].get<std::string>());
  EXPECT_EQ("4", j["comments"][4].get<std::string>());

  EXPECT_EQ("0", j["votes"][0].get<std::string>());
  EXPECT_EQ("1", j["votes"][1].get<std::string>());
  EXPECT_EQ("2", j["votes"][2].get<std::string>());
  EXPECT_EQ("3", j["votes"][3].get<std::string>());
  EXPECT_EQ("4", j["votes"][4].get<std::string>());
}

TEST(TestIssue, Deserialize) {
  json j =
      "{\"id\":\"abcdefgh23\",\"createdAt\":\"Mon May 25 15:30:11 2000\","
      "\"createdBy\":\"hussjess\",\"updatedAt\":\"Mon May 25 15:30:11 2000\","
      "\"updatedBy\":\"memleakcity\",\"title\":\"Example title content\","
      "\"reporter\":\"Reporter Example ID\",\"assignedTo\":\"Assigned Example "
      "User ID\","
      "\"comments\": [\"0\", \"1\", \"2\", \"3\", \"4\"],"
      "\"votes\": [\"0\", \"1\", \"2\", \"3\", \"4\"],"
      "\"status\":\"Example status content\"}"_json;
  auto entity = j.get<Issue>();
  EXPECT_EQ("abcdefgh23", entity.id);
  auto createdAt = TimeUtilities::ConvertTimeToString(entity.createdAt);
  auto updatedAt = createdAt;
  EXPECT_EQ("Mon May 25 15:30:11 2000", createdAt);
  EXPECT_EQ("hussjess", entity.createdBy.id);
  EXPECT_EQ("Mon May 25 15:30:11 2000", updatedAt);
  EXPECT_EQ("memleakcity", entity.updatedBy.id);
  EXPECT_EQ("Example title content", entity.title);
  EXPECT_EQ("Example status content", entity.status);
  EXPECT_EQ("Reporter Example ID", entity.reporter.id);
  EXPECT_EQ("Assigned Example User ID", entity.assignedTo.id);
  EXPECT_EQ(5, entity.comments.size());
  EXPECT_EQ(5, entity.votes.size());
}

TEST(TestIssue, CommentComparison) {
  Issue issue{};
  Comment comment1{};
  comment1.id = "abc";
  comment1.createdAt.tm_hour = 1;
  issue.comments.insert(comment1);
  Comment comment2{};
  comment2.id = "def";
  comment2.createdAt.tm_hour = 10;
  issue.comments.insert(comment2);
  auto it = issue.comments.begin();
  EXPECT_EQ(it->id, "abc");
  std::advance(it, 1);
  EXPECT_EQ(it->id, "def");
}
