#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Comment.h"
#include "CommentService.h"
#include "FileHandler.h"
#include "Issue.h"
#include "IssueService.h"
#include "MockIStreamFileHandler.h"
#include "User.h"
#include "UserService.h"
#include "Utilities.h"
#include "Vote.h"
#include "VoteService.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::StrNe;
using ::testing::Throw;

class MockUserService : public UserService {
 public:
  MockUserService() {}
  virtual ~MockUserService() {}

  MOCK_METHOD1(Get, User(std::string));
};

class MockCommentService : public CommentService {
 public:
  MockCommentService() : CommentService(nullptr) {}
  virtual ~MockCommentService() {}

  MOCK_METHOD1(
      Get, std::vector<Comment>(const std::multimap<std::string, std::string>));
  MOCK_METHOD1(Create, Comment(std::string));
};

class MockVoteService : public VoteService {
 public:
  MockVoteService() : VoteService(nullptr) {}
  virtual ~MockVoteService() {}
  MOCK_METHOD1(
      Get, std::vector<Vote>(const std::multimap<std::string, std::string>));
};

class TestIssueService : public ::testing::Test {
 protected:
  User fakeUser;
  User fakeUser2;
  std::stringstream ss;
  std::shared_ptr<MockIStreamFileHandler> fileHandler;
  std::shared_ptr<MockCommentService> commentService;
  std::shared_ptr<MockUserService> userService;
  std::shared_ptr<MockVoteService> voteService;
  std::shared_ptr<IssueService> issueService;
  json fakeJsonData;
  Comment fakeComment1;
  Comment fakeComment2;
  Comment fakeComment3;

  Vote fakeVote1;
  Vote fakeVote2;
  Vote fakeVote3;

  void SetUp() override {
    fakeUser.id = "1234";
    fakeUser.name = "HussJess";
    fakeUser.role = "Developer";

    fakeUser2.id = "4567";
    fakeUser2.name = "StevenTrinh";
    fakeUser2.role = "Developer";

    fakeComment1.id = "5342";
    fakeComment1.body = "Description of the issue";
    // fakeComment1.issueId = "abcdefgh23";
    fakeComment2.id = "5343";
    // fakeComment2.issueId = "abcdefgh23";
    fakeVote1.id = "6543";
    // fakeVote1.issueId = "abcdefgh23";
    fakeVote2.id = "6573";
    // fakeVote2.issueId = "abcdefgh23";
    fakeComment3.id = "7777";
    fakeComment3.issueId = "2223";
    fakeVote3.id = "8888";
    fakeVote3.issueId = "2223";

    ss << R"(
         [
          {
            "id": "2222",
            "createdAt": "Mon May 25 15:30:11 2000",
            "createdBy": "1234",
            "updatedAt": "Mon May 25 18:30:11 2000",
            "updatedBy": "1234",
            "title": "Fake Title 1",
            "status": "Fake Status 1",
            "assignedTo": "1234",
            "reporter": "4567",
            "comments": ["0","1","2","3","4"],
            "votes": ["0","1","2","3","4"]
          },
          {
            "id": "2223",
            "createdAt": "Mon May 25 15:30:11 2000",
            "createdBy": "4567",
            "updatedAt": "Mon May 25 18:30:11 2000",
            "updatedBy": "4567",
            "title": "Fake Title 2",
            "status": "Fake Status 2",
            "assignedTo": "1234",
            "reporter": "4567",
            "comments": ["0","1","2","3","4"],
            "votes": ["0","1","2","3","4"]
          }
         ]
      )";
    fakeJsonData = json::parse(ss);

    userService = std::make_shared<MockUserService>();
    fileHandler = std::make_shared<MockIStreamFileHandler>();
    commentService = std::make_shared<MockCommentService>();
    voteService = std::make_shared<MockVoteService>();
    issueService = std::make_shared<IssueService>(fileHandler, userService,
                                                  commentService, voteService);
  }
};

TEST_F(TestIssueService, GetIssues_NoQuery) {
  EXPECT_CALL(*userService, Get(fakeUser.id))
      .Times(AtLeast(3))
      .WillRepeatedly(Return(fakeUser));

  EXPECT_CALL(*userService, Get(fakeUser2.id))
      .Times(AtLeast(3))
      .WillRepeatedly(Return(fakeUser2));

  StringMap issue1IdMap = {{"issueId", "2222"}};
  EXPECT_CALL(*commentService, Get(issue1IdMap))
      .WillOnce(Return(std::vector<Comment>{fakeComment1}));

  StringMap issue2IdMap = {{"issueId", "2223"}};
  EXPECT_CALL(*commentService, Get(issue2IdMap))
      .WillOnce(Return(std::vector<Comment>{fakeComment2}));

  EXPECT_CALL(*voteService, Get(issue1IdMap))
      .WillOnce(Return(std::vector<Vote>{fakeVote1}));

  EXPECT_CALL(*voteService, Get(issue2IdMap))
      .WillOnce(Return(std::vector<Vote>{fakeVote2}));

  EXPECT_CALL(*fileHandler, read()).WillOnce(Return(fakeJsonData));

  std::vector<Issue> issues = issueService->Get();
  EXPECT_EQ(2, issues.size());
  EXPECT_EQ(1, issues[0].comments.size());
  EXPECT_EQ(1, issues[1].comments.size());
  EXPECT_EQ(1, issues[0].votes.size());
  EXPECT_EQ(1, issues[1].votes.size());
  EXPECT_EQ("2222", issues[0].id);
  EXPECT_EQ("2223", issues[1].id);
  EXPECT_EQ(fakeUser.id, issues[0].createdBy.id);
  EXPECT_EQ(fakeUser2.id, issues[1].createdBy.id);
}

TEST_F(TestIssueService, GetIssues_WithQuery_ExpectOneResult) {
  EXPECT_CALL(*userService, Get(fakeUser.id))
      .Times(AtLeast(3))
      .WillRepeatedly(Return(fakeUser));

  EXPECT_CALL(*userService, Get(fakeUser2.id)).WillOnce(Return(fakeUser2));

  StringMap issue1IdMap = {{"issueId", "2222"}};
  EXPECT_CALL(*commentService, Get(issue1IdMap))
      .WillOnce(Return(std::vector<Comment>{fakeComment1}));
  EXPECT_CALL(*voteService, Get(issue1IdMap))
      .WillOnce(Return(std::vector<Vote>{fakeVote1}));

  EXPECT_CALL(*fileHandler, read()).WillOnce(Return(fakeJsonData));

  Issue queriedIssue = issueService->Get(std::string("2222"));
  EXPECT_EQ("2222", queriedIssue.id);
  EXPECT_EQ("Mon May 25 15:30:11 2000",
            TimeUtilities::ConvertTimeToString(queriedIssue.createdAt));
  EXPECT_EQ("1234", queriedIssue.createdBy.id);
  EXPECT_EQ("Mon May 25 18:30:11 2000",
            TimeUtilities::ConvertTimeToString(queriedIssue.updatedAt));
  EXPECT_EQ("1234", queriedIssue.updatedBy.id);
  EXPECT_EQ("Fake Title 1", queriedIssue.title);
  EXPECT_EQ("Fake Status 1", queriedIssue.status);
  EXPECT_EQ("1234", queriedIssue.assignedTo.id);
  EXPECT_EQ("4567", queriedIssue.reporter.id);
}

TEST_F(TestIssueService, GetIssues_WithQuery_ExpectNoResults) {
  EXPECT_CALL(*userService, Get).Times(0);
  EXPECT_CALL(*fileHandler, read()).WillOnce(Return(fakeJsonData));
  std::vector<Issue> issues = issueService->Get({{"id", "01234abc"}});
  EXPECT_TRUE(issues.empty());
}

TEST_F(TestIssueService, CreateIssue_ValidIssue_WithDescription) {
  std::string body =
      R"(
    {
      "id": "something",
      "title": "title of the issue",
      "createdBy": "1234",
      "description": "Description of the issue"         
    }
  )";

  EXPECT_CALL(*userService, Get(std::string("1234")))
      .Times(AtLeast(1))
      .WillRepeatedly(Return(fakeUser));

  EXPECT_CALL(*fileHandler, read()).WillOnce(Return(fakeJsonData));

  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  // We should create a comment with the "description" comment provided
  EXPECT_CALL(*commentService, Create(_)).WillOnce(Return(fakeComment1));

  Issue result = issueService->Create(body);

  EXPECT_THAT(result.id, StrNe("abcdefgh23"));
  // EXPECT_EQ("Mon May 25 15:30:11 2000",
  // TimeUtilities::ConvertTimeToString(result.createdAt)); EXPECT_EQ("Mon May
  // 28 15:30:11 2000", TimeUtilities::ConvertTimeToString(result.updatedAt));

  // The created date should be ignored and the current UTC time should be used
  struct tm now = TimeUtilities::CurrentTimeUTC();
  EXPECT_EQ(result.createdAt.tm_year, now.tm_year);
  EXPECT_EQ(result.createdAt.tm_mon, now.tm_mon);
  EXPECT_EQ(result.createdAt.tm_mday, now.tm_mday);
  EXPECT_EQ(result.createdAt.tm_hour, now.tm_hour);

  // The updated date should be the "null date"
  struct tm nullTime = TimeUtilities::NullTimeUTC();
  EXPECT_EQ(result.updatedAt.tm_year, nullTime.tm_year);
  EXPECT_EQ(result.updatedAt.tm_mon, nullTime.tm_mon);
  EXPECT_EQ(result.updatedAt.tm_mday, nullTime.tm_mday);
  EXPECT_EQ(result.updatedAt.tm_hour, nullTime.tm_hour);

  // The updated user should also be uninitialized
  EXPECT_EQ(result.updatedBy.id, "");
  EXPECT_EQ(result.updatedBy.name, "");

  // The assigned user should also be uninitialized
  EXPECT_EQ(result.assignedTo.id, "");
  EXPECT_EQ(result.assignedTo.name, "");

  // The reporter should also be uninitialized
  EXPECT_EQ(result.reporter.id, result.createdBy.id);
  EXPECT_EQ(result.reporter.name, result.createdBy.name);

  // There should be one comment, with the body of the description that we
  // provided
  EXPECT_EQ(result.comments.size(), 1);
  EXPECT_EQ(result.comments.begin()->body, "Description of the issue");
  EXPECT_EQ(result.votes.size(), 0);

  EXPECT_EQ(result.status, "New");
  EXPECT_EQ(result.title, "title of the issue");
}

TEST_F(TestIssueService, CreateIssue_HandleOptionalAndIgnoredFields) {
  // Status, Reporter, and Assigned To are all optional fields. So if they are
  // not specified, then we should be able to handle that. Similarly, Id,
  // Updated At, Updated By, and Votes are all ignored. So providing their value
  // on creation should be ignored
  std::string body =
      "{\"id\":\"abcdefgh23\",\"createdAt\":\"Mon May 25 15:30:11 2000\","
      "\"createdBy\":\"1234\",\"updatedAt\":\"Mon May 28 15:30:11 2000\","
      "\"updatedBy\":\"1234\",\"title\":\"Example title content\","
      "\"reporter\":\"1234\",\"assignedTo\":\"1234\","
      "\"votes\": [\"0\", \"1\", \"2\", \"3\", \"4\"],"
      "\"status\":\"Example status content\","
      "\"title\":\"Example title\"}";

  // We should read the json file once
  EXPECT_CALL(*fileHandler, read).Times(1).WillOnce(Return(fakeJsonData));

  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  // We should only call the user service once
  EXPECT_CALL(*userService, Get(std::string("1234")))
      .Times(2)
      .WillRepeatedly(Return(fakeUser));

  // No "description" was provided, so we shouldn't call the comment service
  EXPECT_CALL(*commentService, Create).Times(0);

  Issue result = issueService->Create(body);

  EXPECT_THAT(result.id, StrNe("abcdefgh23"));

  EXPECT_EQ(result.createdBy.id, fakeUser.id);
  EXPECT_EQ(result.createdBy.name, fakeUser.name);

  // The created date should be ignored and the current UTC time should be used
  struct tm now = TimeUtilities::CurrentTimeUTC();
  EXPECT_EQ(result.createdAt.tm_year, now.tm_year);
  EXPECT_EQ(result.createdAt.tm_mon, now.tm_mon);
  EXPECT_EQ(result.createdAt.tm_mday, now.tm_mday);
  EXPECT_EQ(result.createdAt.tm_hour, now.tm_hour);

  // The updated date should be the "null date"
  struct tm nullTime = TimeUtilities::NullTimeUTC();
  EXPECT_EQ(result.updatedAt.tm_year, nullTime.tm_year);
  EXPECT_EQ(result.updatedAt.tm_mon, nullTime.tm_mon);
  EXPECT_EQ(result.updatedAt.tm_mday, nullTime.tm_mday);
  EXPECT_EQ(result.updatedAt.tm_hour, nullTime.tm_hour);

  // The updated user should also be uninitialized
  EXPECT_EQ(result.updatedBy.id, "");
  EXPECT_EQ(result.updatedBy.name, "");

  // The assigned user should also be uninitialized
  EXPECT_EQ(result.assignedTo.id, "1234");
  EXPECT_EQ(result.assignedTo.name, "HussJess");

  // The reporter should also be uninitialized
  EXPECT_EQ(result.reporter.id, fakeUser.id);
  EXPECT_EQ(result.reporter.name, fakeUser.name);

  EXPECT_EQ(0, result.votes.size());
  EXPECT_EQ(0, result.comments.size());
}

TEST_F(TestIssueService, DeleteIssue_ValidDelete) {
  std::string id = "2223";

  // We should read the json file once
  EXPECT_CALL(*fileHandler, read).Times(1).WillOnce(Return(fakeJsonData));

  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);
  EXPECT_TRUE(issueService->Delete(id));
}

TEST_F(TestIssueService, DeleteIssue_InvalidId) {
  std::string id = "9999";

  // We should read the json file once
  EXPECT_CALL(*fileHandler, read).Times(1).WillOnce(Return(fakeJsonData));

  // We shouldn't write to the json file at all
  EXPECT_CALL(*fileHandler, write).Times(0);
  EXPECT_THROW(issueService->Delete(id), NotFoundError);
}

TEST_F(TestIssueService, Update_ExpectValidUserUpdated) {
  std::string body =
      "{\"id\":\"2223\",\"createdAt\":\"Mon May 25 15:30:11 2000\","
      "\"createdBy\":\"4567\",\"updatedAt\":\"Mon May 28 17:50:11 2000\","
      "\"updatedBy\":\"1234\",\"title\":\"Updated title content\","
      "\"reporter\":\"1234\",\"assignedTo\":\"4567\","
      "\"comments\": [\"0\", \"1\"],"
      "\"votes\": [\"0\", \"1\", \"2\"],"
      "\"status\":\"Updated status content\"}";

  // We should call the user service twice, once for the creating user and
  // once for the updating user
  EXPECT_CALL(*userService, Get("1234"))
      .Times(AtLeast(2))
      .WillRepeatedly(Return(fakeUser));
  EXPECT_CALL(*userService, Get("4567"))
      .Times(AtLeast(2))
      .WillRepeatedly(Return(fakeUser2));
  StringMap issue1IdMap = {{"issueId", "2223"}};

  EXPECT_CALL(*commentService, Get(issue1IdMap))
      .WillOnce(Return(std::vector<Comment>{fakeComment1, fakeComment2}));
  EXPECT_CALL(*voteService, Get(issue1IdMap))
      .WillOnce(Return(std::vector<Vote>{fakeVote1, fakeVote2, fakeVote3}));

  // We should read the json file once
  EXPECT_CALL(*fileHandler, read).Times(1).WillOnce(Return(fakeJsonData));
  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  Issue testIssue = issueService->Update(body);

  auto createdAt = TimeUtilities::ConvertTimeToString(testIssue.createdAt);
  auto updatedAt = TimeUtilities::ConvertTimeToString(testIssue.updatedAt);

  EXPECT_EQ("2223", testIssue.id);
  EXPECT_EQ("Mon May 25 15:30:11 2000", createdAt);
  EXPECT_EQ(fakeUser2.id, testIssue.createdBy.id);
  // The updated at field should be ignored and the current time should be used
  auto now = TimeUtilities::CurrentTimeUTC();
  auto nowString = TimeUtilities::ConvertTimeToString(now);
  EXPECT_EQ(nowString, updatedAt);
  EXPECT_EQ(fakeUser.id, testIssue.updatedBy.id);
  EXPECT_EQ(fakeUser2.id, testIssue.assignedTo.id);
  EXPECT_EQ(fakeUser.id, testIssue.reporter.id);
  EXPECT_EQ("Updated title content", testIssue.title);
  EXPECT_EQ("Updated status content", testIssue.status);

  EXPECT_EQ(testIssue.comments.size(), 2);
  EXPECT_EQ(testIssue.votes.size(), 3);
}
