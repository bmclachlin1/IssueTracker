#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Exceptions.h"
#include "FileHandler.h"
#include "IStreamableFileHandler.h"
#include "MockIStreamFileHandler.h"
#include "User.h"
#include "UserService.h"
#include "nlohmann/json.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::StrNe;

using json = nlohmann::json;
class TestUserService : public ::testing::Test {
 protected:
  std::stringstream ss;
  std::shared_ptr<MockIStreamFileHandler> fileHandler =
      std::make_shared<MockIStreamFileHandler>();
  std::shared_ptr<UserService> userService;
  json fakeJsonData;

  virtual void SetUp() {
    ss << R"(
         [
          {
            "id": "2222",
            "name": "Steven Trinh",
            "role": "Developer"
          },
          {
            "id": "3333",
            "name": "TestName",
            "role": "Developer"
          }
         ]
      )";

    fakeJsonData = json::parse(ss);
    userService = std::make_shared<UserService>(fileHandler);
  }
};

TEST_F(TestUserService, TestFilter_Expect_SingleParamIdIs2222) {
  json filtered = userService->Filter(fakeJsonData, {{"role", "Developer"}});
  EXPECT_EQ(filtered.size(), 2);
}

TEST_F(TestUserService, TestFilter_Expect_MultipleParamRoleDeveloperId2222) {
  json filtered = userService->Filter(
      fakeJsonData, {{"id", "2222"}, {"name", "Steven Trinh"}});
  EXPECT_EQ(filtered.size(), 1);
}

TEST_F(TestUserService, TestFilter_Expect_NoResults_KeyDoesntExist) {
  json filtered = userService->Filter(fakeJsonData, {{"place", "8"}});
  EXPECT_EQ(filtered.size(), 0);
}

TEST_F(TestUserService, TestFilter_Expect_NoResults_EmptyValues) {
  json filtered = userService->Filter(fakeJsonData, {{"name", ""}});
  EXPECT_EQ(filtered.size(), 0);
}

TEST_F(TestUserService, TestFilter_Expect_NoResults) {
  json filtered =
      userService->Filter(fakeJsonData, {{"role", "SupremeMemLeak"}});
  EXPECT_EQ(filtered.size(), 0);
}

TEST_F(TestUserService, TestFilter_EmptyFilterProvided) {
  json filtered = userService->Filter(fakeJsonData, {});
  EXPECT_EQ(filtered.size(), 2);
}

TEST_F(TestUserService, GetUsers_Expect_TwoValidUsers) {
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::vector<User> testUsers = userService->Get();
  EXPECT_EQ(testUsers.size(), 2);
  EXPECT_EQ(testUsers[0].id, "2222");
  EXPECT_EQ(testUsers[0].name, "Steven Trinh");
  EXPECT_EQ(testUsers[0].role, "Developer");

  EXPECT_EQ(testUsers[1].id, "3333");
  EXPECT_EQ(testUsers[1].name, "TestName");
  EXPECT_EQ(testUsers[1].role, "Developer");
}

TEST_F(TestUserService, GetUsers_Expect_TwoUsers) {
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  auto testUsers = userService->Get({{"role", "Developer"}});
  EXPECT_EQ(testUsers.size(), 2);
}

TEST_F(TestUserService, GetUsers_Expect_OneUser) {
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  auto testUsers = userService->Get({{"name", "TestName"}});
  EXPECT_EQ(testUsers.size(), 1);
}

TEST_F(TestUserService, GetUser_ExpectValidUser) {
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  User testUser = userService->Get("2222");
  EXPECT_EQ(testUser.id, "2222");
  EXPECT_EQ(testUser.name, "Steven Trinh");
  EXPECT_EQ(testUser.role, "Developer");
}

TEST_F(TestUserService, GetUser_ExpectNotFoundError) {
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  EXPECT_THROW(userService->Get("1234"), NotFoundError);
}

TEST_F(TestUserService, CreateUser_ExpectValidUserCreated) {
  std::string body =
      "{\"id\": \"5555\", \"name\": \"Julie Liu\", \"role\": \"Doctor\"}";
  std::shared_ptr<MockIStreamFileHandler> mockFileHandler =
      std::make_shared<MockIStreamFileHandler>();
  std::string testJsonStr = R"(
        [
            {
              "name": "Blake Memleaklin",
              "role": "Doctor Anvik's Student"
             }
        ]
    )";

  json j = json::parse(testJsonStr);

  UserService* us = new UserService(mockFileHandler);
  EXPECT_CALL(*mockFileHandler, read()).Times(1).WillOnce(Return(j));
  EXPECT_CALL(*mockFileHandler, write(_)).Times(1);
  User testUser = us->Create(body);
  EXPECT_THAT(testUser.id, StrNe("5555"));
  EXPECT_EQ(testUser.name, "Julie Liu");
  EXPECT_EQ(testUser.role, "Doctor");

  delete us;
}

TEST_F(TestUserService, CreateUser_ExpectValidCreation_MissingBodyValues) {
  std::string body = "{\"name\": \"Julie Liu\"}";
  std::shared_ptr<MockIStreamFileHandler> mockFileHandler =
      std::make_shared<MockIStreamFileHandler>();
  std::string testJsonStr = R"(
        [
            {
              "name": "Blake Memleaklin",
              "role": "Doctor Anvik's Student"
             }
        ]
    )";

  json j = json::parse(testJsonStr);

  UserService* us = new UserService(mockFileHandler);
  EXPECT_CALL(*mockFileHandler, read()).Times(1).WillOnce(Return(j));
  EXPECT_CALL(*mockFileHandler, write(_)).Times(1);
  User testUser = us->Create(body);
  EXPECT_THAT(testUser.id, StrNe(""));
  EXPECT_EQ(testUser.name, "Julie Liu");
  EXPECT_EQ(testUser.role, "Developer");

  delete us;
}

TEST_F(TestUserService, CreateUser_ExpectEntityAlreadyExistsError) {
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::string body =
      "{\"id\": \"2222\", \"name\": \"Steven Trinh\", \"role\": \"Doctor\"}";
  EXPECT_THROW(userService->Create(body), AlreadyExistsError);
}

TEST_F(TestUserService, CreateUser_ExpectBadRequestError) {
  std::string body = "badJSONStringEntity";
  EXPECT_THROW(userService->Create(body), BadRequestError);
}

TEST_F(TestUserService, Delete_ExpectValidUserDeleted) {
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  std::string body = "2222";
  bool deleted = userService->Delete(body);
  EXPECT_EQ(deleted, true);
}

TEST_F(TestUserService, Delete_ExpectEntityDoesNotExistError) {
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::string body = "8888";
  EXPECT_THROW(userService->Delete(body), NotFoundError);
}

TEST_F(TestUserService, Update_ExpectValidUserUpdated) {
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  // We should write to the json file once
  EXPECT_CALL(*fileHandler, write(_)).Times(1);

  std::string body =
      "{\"id\": \"2222\", \"name\": \"UpdatedName\", \"role\": "
      "\"UpdatedRole\"}";
  User testUser = userService->Update(body);
  EXPECT_EQ(testUser.id, "2222");
  EXPECT_EQ(testUser.name, "UpdatedName");
  EXPECT_EQ(testUser.role, "UpdatedRole");
}

TEST_F(TestUserService, Update_ExpectNameAlreadyExistsError) {
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::string body =
      "{\"id\": \"2222\", \"name\": \"TestName\", \"role\": "
      "\"UpdatedRole\"}";
  EXPECT_THROW(userService->Update(body), AlreadyExistsError);
}

TEST_F(TestUserService, Update_ExpectEntityDoesNotExistError) {
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));

  std::string body =
      "{\"id\": \"8888\", \"name\": \"Julie Liu\", \"role\": \"Doctor\"}";
  EXPECT_THROW(userService->Update(body), NotFoundError);
}

TEST_F(TestUserService, UpdateUser_ExpectEntityCreationError) {
  std::string body = "testfake";
  EXPECT_THROW(userService->Update(body), BadRequestError);
}
