#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Comment.h"
#include "CommentService.h"
#include "FileHandler.h"
#include "MockIStreamFileHandler.h"
#include "User.h"
#include "UserService.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;
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

class TestCommentService : public ::testing::Test {
 protected:
  User fakeUser;
  User fakeUser2;
  std::stringstream ss;
  std::shared_ptr<MockIStreamFileHandler> fileHandler;
  std::shared_ptr<CommentService> commentService;
  std::shared_ptr<MockUserService> userService;
  json fakeJsonData;

  void SetUp() override {
    userService = std::make_shared<MockUserService>();

    fakeUser.id = "1234";
    fakeUser.name = "HussJess";
    fakeUser.role = "Developer";

    fakeUser2.id = "4567";
    fakeUser2.name = "StevenTrinh";
    fakeUser2.role = "Developer";

    ss << R"(
         [
          {
            "id": "2222",
            "createdAt": "Mon May 25 15:30:11 2000",
            "createdBy": "1234",
            "updatedAt": "Mon May 25 18:30:11 2000",
            "updatedBy": "1234",
            "issueId": "123456",
            "body": "Example body content1"
          },
          {
            "id": "2223",
            "createdAt": "Mon May 25 15:30:11 2000",
            "createdBy": "4567",
            "updatedAt": "Mon May 25 18:30:11 2000",
            "updatedBy": "4567",
            "issueId": "1234567",
            "body": "Example body content2"
          }
         ]
      )";
    fakeJsonData = json::parse(ss);

    fileHandler = std::make_shared<MockIStreamFileHandler>();
    commentService = std::make_shared<CommentService>(fileHandler, userService);
  }
};

TEST_F(TestCommentService, GetComments_NoQuery) {
  // The user service should be called twice (getting the users for : 1x
  // createdBy, 1x updatedBy) * 2 JSON comments
  EXPECT_CALL(*userService, Get(fakeUser.id))
      .Times(2)
      .WillRepeatedly(Return(fakeUser));

  EXPECT_CALL(*userService, Get(fakeUser2.id))
      .Times(2)
      .WillRepeatedly(Return(fakeUser2));

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::vector<Comment> comments = commentService->Get();
  EXPECT_EQ(2, comments.size());
}

TEST_F(TestCommentService, GetComments_WithQuery_ExpectOneResult) {
  // The user service should be called once. (1x createdBy 1x updatedBy) * 1
  // (issueID query = 1 JSON comment)
  EXPECT_CALL(*userService, Get(fakeUser.id))
      .Times(2)
      .WillRepeatedly(Return(fakeUser));

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::vector<Comment> comments = commentService->Get({{"issueId", "123456"}});
  EXPECT_EQ(1, comments.size());
}

TEST_F(TestCommentService, GetComments_WithQuery_ExpectNoResults) {
  // The user service should not be called
  EXPECT_CALL(*userService, Get).Times(0);

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::vector<Comment> comments =
      commentService->Get({{"issueId", "some-nonsense-id"}});
  EXPECT_TRUE(comments.empty());
}

TEST_F(TestCommentService, GetComment_ValidId) {
  // The user service should be called once. (1x createdBy, 1x updatedBy) * 1 (1
  // JSON comment with id 2222)
  EXPECT_CALL(*userService, Get(fakeUser.id))
      .Times(2)
      .WillRepeatedly(Return(fakeUser));

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  Comment comment = commentService->Get(std::string("2222"));
  EXPECT_EQ("2222", comment.id);
  EXPECT_EQ("123456", comment.issueId);
  EXPECT_EQ(fakeUser.id, comment.createdBy.id);
  EXPECT_EQ(fakeUser.name, comment.createdBy.name);

  EXPECT_EQ(fakeUser.id, comment.updatedBy.id);
  EXPECT_EQ(fakeUser.name, comment.updatedBy.name);

  EXPECT_EQ("Example body content1", comment.body);
}

TEST_F(TestCommentService, GetComment_InvalidId) {
  // The user service should not get called
  EXPECT_CALL(*userService, Get).Times(0);

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::string id = "4321";
  EXPECT_THROW(commentService->Get(id), NotFoundError);
}

TEST_F(TestCommentService, GetComment_NoId) {
  // The user service should not get called
  EXPECT_CALL(*userService, Get).Times(0);

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::string id = "";
  EXPECT_THROW(commentService->Get(id), NotFoundError);
}

TEST_F(TestCommentService, CreateComment_ValidComment) {
  std::string body =
      "{\"issueId\": \"123456\", \"createdBy\": \"1234\", \"body\": \"Example "
      "body content\"}";

  // We should check to verify the user if provided is valid
  EXPECT_CALL(*userService, Get(std::string("1234")))
      .Times(1)
      .WillRepeatedly(Return(fakeUser));

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  Comment result = commentService->Create(body);

  // The provided id should be ignored and a new one should be generated
  EXPECT_THAT(result.id, StrNe("5555"));
  // TODO(someone): Add check from Issue Service that this is a valid Issue ID
  EXPECT_THAT(result.issueId, StrEq("123456"));
  EXPECT_EQ(result.createdBy.id, fakeUser.id);
  EXPECT_EQ(result.createdBy.name, fakeUser.name);

  EXPECT_EQ(result.body, "Example body content");

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
}

TEST_F(TestCommentService, CreateComment_ValidComment_ExtraFieldsIgnored) {
  std::string body =
      "{\"id\":\"2223\",\"createdAt\":\"Mon May 25 15:30:11 2000\","
      "\"createdBy\":\"4567\",\"updatedAt\":\"Mon May 25 15:30:11 2000\","
      "\"updatedBy\":\"4567\",\"issueId\":\"1234567\","
      "\"body\":\"Example body content\"}";

  // We should check to verify the user if provided is valid
  EXPECT_CALL(*userService, Get(std::string("4567")))
      .Times(1)
      .WillRepeatedly(Return(fakeUser2));

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  Comment result = commentService->Create(body);

  // The provided id should be ignored and a new one should be generated
  EXPECT_THAT(result.id, StrNe("2223"));
  // TODO(someone): Add check from Issue Service that this is a valid Issue ID
  EXPECT_THAT(result.issueId, StrEq("1234567"));
  EXPECT_EQ(result.createdBy.id, fakeUser2.id);
  EXPECT_EQ(result.createdBy.name, fakeUser2.name);

  EXPECT_EQ(result.body, "Example body content");

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
}

TEST_F(TestCommentService, CreateComment_InvalidComment_UserDoesntExist) {
  std::string body =
      "{\"id\": \"5555\", \"issueId\": \"123456\", \"createdBy\": \"9999\", "
      "\"createdAt\": \"Mon May 25 15:30:11 2000\"}";

  // The user service should be called. We fake it not finding the user
  EXPECT_CALL(*userService, Get(std::string("9999")))
      .Times(1)
      .WillOnce(Throw(NotFoundError("some error message")));

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  // We shouldn't write to the json file at all
  EXPECT_CALL(*fileHandler, write).Times(0);

  // The Comment Service (via its user service) should throw an error
  EXPECT_THROW(commentService->Create(body), NotFoundError);
}

TEST_F(TestCommentService, CreateComment_InvalidComment_NoIssueId) {
  std::string body = "{\"createdBy\": \"9999\"}";

  // The user service shouldn't be called at all
  EXPECT_CALL(*userService, Get).Times(0);

  // We should read the json file once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  // We shouldn't write to the json file at all
  EXPECT_CALL(*fileHandler, write).Times(0);

  // Missing the issue id constitudes a bad request, so that should be thrown
  EXPECT_THROW(commentService->Create(body), BadRequestError);
}

TEST_F(TestCommentService, DeleteComment_ValidDelete) {
  std::string id = "2222";

  // We should read the json file once
  EXPECT_CALL(*fileHandler, read).Times(1).WillOnce(Return(fakeJsonData));

  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  EXPECT_TRUE(commentService->Delete(id));
}

TEST_F(TestCommentService, DeleteComment_InvalidId) {
  std::string id = "9999";

  // We should read the json file once
  EXPECT_CALL(*fileHandler, read).Times(1).WillOnce(Return(fakeJsonData));

  // We shouldn't write to the json file at all
  EXPECT_CALL(*fileHandler, write).Times(0);

  EXPECT_THROW(commentService->Delete(id), NotFoundError);
}

TEST_F(TestCommentService, Update_ExpectValidUserUpdated) {
  std::string body =
      "{\"id\":\"2222\",\"createdAt\":\"Mon May 25 15:30:11 2000\","
      "\"createdBy\":\"1234\",\"updatedAt\":\"Mon May 25 19:30:11 2000\","
      "\"updatedBy\":\"4567\",\"issueId\":\"1234567\","
      "\"body\":\"Updated body content\"}";

  // We should call the user service twice, once for the creating user and once
  // for the updating user
  EXPECT_CALL(*userService, Get("1234")).WillOnce(Return(fakeUser));
  EXPECT_CALL(*userService, Get("4567")).WillOnce(Return(fakeUser2));

  // We should read the json file once
  EXPECT_CALL(*fileHandler, read).Times(1).WillOnce(Return(fakeJsonData));
  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  Comment testComment = commentService->Update(body);
  auto createdAt = TimeUtilities::ConvertTimeToString(testComment.createdAt);
  auto updatedAt = TimeUtilities::ConvertTimeToString(testComment.updatedAt);

  EXPECT_EQ("2222", testComment.id);
  EXPECT_EQ("Mon May 25 15:30:11 2000", createdAt);
  EXPECT_EQ(fakeUser.id, testComment.createdBy.id);
  // The updated at field should be ignored and the current time should be used
  auto now = TimeUtilities::CurrentTimeUTC();
  auto nowString = TimeUtilities::ConvertTimeToString(now);
  EXPECT_EQ(nowString, updatedAt);
  EXPECT_EQ(fakeUser2.id, testComment.updatedBy.id);
  EXPECT_EQ("1234567", testComment.issueId);
  EXPECT_EQ("Updated body content", testComment.body);
}

TEST_F(TestCommentService, Update_ExpectEntityDoesNotExistError) {
  std::string body = "{\"id\": \"8888\", \"issueId\": \"1111111\"}";
  // We should read the json file once
  EXPECT_CALL(*fileHandler, read).Times(1).WillOnce(Return(fakeJsonData));
  EXPECT_THROW(commentService->Update(body), NotFoundError);
}

TEST_F(TestCommentService, UpdateUser_ExpectEntityCreationError) {
  std::string body = "testfake";
  EXPECT_THROW(commentService->Update(body), BadRequestError);
}
