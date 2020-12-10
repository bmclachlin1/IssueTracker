#include <map>
#include <memory>
#include <string>
#include <vector>

#include "FileHandler.h"
#include "MockIStreamFileHandler.h"
#include "MockUserService.h"
#include "User.h"
#include "UserService.h"
#include "Vote.h"
#include "VoteService.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::StrNe;
using ::testing::Throw;

class TestVoteService : public ::testing::Test {
 protected:
  User fakeUser;
  std::stringstream ss;
  std::shared_ptr<MockIStreamFileHandler> fileHandler;
  std::shared_ptr<VoteService> voteService;
  std::shared_ptr<MockUserService> userService;
  json fakeJsonData;

  void SetUp() override {
    userService = std::make_shared<MockUserService>();

    fakeUser.id = "1234";
    fakeUser.name = "HussJess";
    fakeUser.role = "Developer";

    ss << R"(
         [
          {
            "id": "2222",
            "createdAt": "Mon May 25 15:30:11 2000",
            "createdBy": "1234",
            "updatedAt": "",
            "updatedBy": "",
            "issueId": "123456"
          },
          {
            "id": "2223",
            "createdAt": "Mon May 25 15:30:11 2000",
            "createdBy": "1234",
            "updatedAt": "",
            "updatedBy": "",
            "issueId": "1234567"
          }
         ]
      )";
    fakeJsonData = json::parse(ss);

    fileHandler = std::make_shared<MockIStreamFileHandler>();
    voteService = std::make_shared<VoteService>(fileHandler, userService);
  }
};

TEST_F(TestVoteService, GetVotes_NoQuery) {
  // The user service should be called twice. We fake it returning the fake user
  EXPECT_CALL(*userService, Get(fakeUser.id))
      .Times(2)
      .WillRepeatedly(Return(fakeUser));

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::vector<Vote> votes = voteService->Get();
  EXPECT_EQ(2, votes.size());
}

TEST_F(TestVoteService, GetVotes_WithQuery_ExpectOneResult) {
  // The user service should be called once. We fake it returning the fake user
  EXPECT_CALL(*userService, Get(fakeUser.id))
      .Times(1)
      .WillOnce(Return(fakeUser));

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::vector<Vote> votes = voteService->Get({{"issueId", "123456"}});
  EXPECT_EQ(1, votes.size());
}

TEST_F(TestVoteService, GetVotes_WithQuery_ExpectNoResults) {
  // The user service should not be called
  EXPECT_CALL(*userService, Get(::testing::A<std::string>())).Times(0);

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::vector<Vote> votes = voteService->Get({{"issueId", "some-nonsense-id"}});
  EXPECT_TRUE(votes.empty());
}

TEST_F(TestVoteService, GetVote_ValidId) {
  // The user service should be called once. We fake it returning the fake user
  EXPECT_CALL(*userService, Get(fakeUser.id))
      .Times(1)
      .WillOnce(Return(fakeUser));

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  Vote vote = voteService->Get(std::string("2222"));
  EXPECT_EQ("2222", vote.id);
  EXPECT_EQ("123456", vote.issueId);
  EXPECT_EQ(fakeUser.id, vote.createdBy.id);
  EXPECT_EQ(fakeUser.name, vote.createdBy.name);
  EXPECT_EQ("", vote.updatedBy.id);
}

TEST_F(TestVoteService, GetVote_InvalidId) {
  // The user service should not get called
  EXPECT_CALL(*userService, Get(::testing::A<std::string>())).Times(0);

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::string id = "4321";
  EXPECT_THROW(voteService->Get(id), NotFoundError);
}

TEST_F(TestVoteService, GetVote_NoId) {
  // The user service should not get called
  EXPECT_CALL(*userService, Get(::testing::A<std::string>())).Times(0);

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::string id = "";
  EXPECT_THROW(voteService->Get(id), NotFoundError);
}

TEST_F(TestVoteService, CreateVote_ValidVote) {
  std::string body = "{\"issueId\": \"123456\", \"createdBy\": \"1234\"}";

  // We should check to verify the user if provided is valid
  EXPECT_CALL(*userService, Get(std::string("1234")))
      .Times(1)
      .WillOnce(Return(fakeUser));

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  Vote result = voteService->Create(body);

  // The provided id should be ignored and a new one should be generated
  EXPECT_THAT(result.id, StrNe("5555"));
  // TODO(anyone): Add check from Issue Service that this is a valid Issue ID
  EXPECT_THAT(result.issueId, StrEq("123456"));
  EXPECT_EQ(result.createdBy.id, fakeUser.id);
  EXPECT_EQ(result.createdBy.name, fakeUser.name);

  // The created date should be ignored and the current UTC time should be used
  time_t t = time(0);
  struct tm now = *gmtime(&t);
  EXPECT_EQ(result.createdAt.tm_year, now.tm_year);
  EXPECT_EQ(result.createdAt.tm_mon, now.tm_mon);
  EXPECT_EQ(result.createdAt.tm_mday, now.tm_mday);
  EXPECT_EQ(result.createdAt.tm_hour, now.tm_hour);

  // The updated by user should be a "null" user
  EXPECT_THAT(result.updatedBy.id, StrEq(""));
}

TEST_F(TestVoteService, CreateVote_ValidVote_ExtraFieldsIgnored) {
  std::string body =
      "{\"id\": \"5555\", \"issueId\": \"123456\", \"createdBy\": \"1234\", "
      "\"createdAt\": \"Mon May 25 15:30:11 2000\", \"updatedBy\": \"1234\", "
      "\"updatedAt\": \"Some invalid ridiculous date string that will never "
      "be read\"}";

  // We should check to verify the user if provided is valid
  EXPECT_CALL(*userService, Get(std::string("1234")))
      .Times(1)
      .WillOnce(Return(fakeUser));

  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  Vote result = voteService->Create(body);

  // The provided id should be ignored and a new one should be generated
  EXPECT_THAT(result.id, StrNe("5555"));
  // TODO(anyone): Add check from Issue Service that this is a valid Issue ID
  EXPECT_THAT(result.issueId, StrEq("123456"));
  EXPECT_EQ(result.createdBy.id, fakeUser.id);
  EXPECT_EQ(result.createdBy.name, fakeUser.name);

  // The created date should be ignored and the current UTC time should be used
  time_t t = time(0);
  struct tm now = *gmtime(&t);
  EXPECT_EQ(result.createdAt.tm_year, now.tm_year);
  EXPECT_EQ(result.createdAt.tm_mon, now.tm_mon);
  EXPECT_EQ(result.createdAt.tm_mday, now.tm_mday);
  EXPECT_EQ(result.createdAt.tm_hour, now.tm_hour);

  // The updated by user should be a "null" user
  EXPECT_THAT(result.updatedBy.id, StrEq(""));
}

TEST_F(TestVoteService, CreateVote_InvalidVote_UserDoesntExist) {
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

  // The Vote Service (via its user service) should throw an error
  EXPECT_THROW(voteService->Create(body), NotFoundError);
}

TEST_F(TestVoteService, CreateVote_InvalidVote_NoIssueId) {
  std::string body = "{\"createdBy\": \"9999\"}";

  // The user service shouldn't be called at all
  EXPECT_CALL(*userService, Get(::testing::A<std::string>())).Times(0);

  // We should read the json file once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  // We shouldn't write to the json file at all
  EXPECT_CALL(*fileHandler, write).Times(0);

  // Missing the issue id constitudes a bad request, so that should be thrown
  EXPECT_THROW(voteService->Create(body), BadRequestError);
}

TEST_F(TestVoteService, UpdateVote_ShouldThrow) {
  EXPECT_THROW(voteService->Update("anything"), NotImplementedError);
}

TEST_F(TestVoteService, DeleteVote_ValidDelete) {
  std::string id = "2222";

  // We should read the json file once
  EXPECT_CALL(*fileHandler, read).Times(1).WillOnce(Return(fakeJsonData));

  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  EXPECT_TRUE(voteService->Delete(id));
}

TEST_F(TestVoteService, DeleteVote_InvalidId) {
  std::string id = "9999";

  // We should read the json file once
  EXPECT_CALL(*fileHandler, read).Times(1).WillOnce(Return(fakeJsonData));

  // We shouldn't write to the json file at all
  EXPECT_CALL(*fileHandler, write).Times(0);

  EXPECT_THROW(voteService->Delete(id), NotFoundError);
}
