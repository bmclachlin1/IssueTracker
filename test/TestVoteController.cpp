
#include <restbed>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "CommentService.h"
#include "Exceptions.h"
#include "MockSession.h"
#include "MockVoteService.h"
#include "VoteController.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

using ::testing::_;
using ::testing::InvokeArgument;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::StrNe;
using ::testing::Throw;

using json = nlohmann::json;

/**
 * Fixture for bootstrapping the test process
 */
class TestVoteController : public ::testing::Test {
 protected:
  // Test variables used throughout the tests
  VoteController<MockSession> controller;
  std::shared_ptr<MockSession> mockSession;
  std::shared_ptr<restbed::Request> request;
  std::shared_ptr<MockVoteService> mockService;

  Vote vote;
  json jsonVote;
  User fakeUser;
  std::string timeString;
  std::string requestBody;
  std::string responseBody;
  std::string fakeIssueId;

  void SetUp() override {
    // Instantiate the test varialbes
    fakeIssueId = "fakeissue";
    fakeUser.id = "fakeuser";
    vote.id = "fakevote";
    vote.issueId = fakeIssueId;
    vote.createdBy = fakeUser;
    timeString = "Mon May 25 15:30:11 2000";
    request = std::make_shared<restbed::Request>();
    mockSession = std::make_shared<MockSession>();
    mockService = std::make_shared<MockVoteService>();

    // Set up the controller with the mocked service
    controller.SetEntityService(mockService);
  }
};

TEST_F(TestVoteController, Create_ValidVote) {
  // Set up the request body to contain a user id
  json requestJson;
  requestJson["createdBy"] = fakeUser.id;
  requestBody = requestJson.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(requestBody);

  // Set up the response body to be the created Vote
  jsonVote = vote;
  responseBody = jsonVote.dump();

  // Set the necessary request parameters
  request->set_path("/issues/" + fakeIssueId + "/votes");
  request->set_body(requestBody);
  request->set_header("Content-Length", std::to_string(requestBody.size()));

  // The controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // The controller should fetch the request body and invoke the callback to
  // process the body
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  // We should call the service to get any votes by the user for the issue. We
  // fake there being none, thus we will try to create a new vote
  StringMap queryParams = {{"issueId", fakeIssueId},
                           {"createdBy", fakeUser.id}};
  EXPECT_CALL(*mockService, Get(queryParams))
      .Times(1)
      .WillOnce(Return(std::vector<Vote>{}));

  // The service should successfully create the vote
  EXPECT_CALL(*mockService, Create(requestBody)).WillOnce(Return(vote));

  // The controller should close the session with a status of CREATED and the
  // vote in the body of the response
  EXPECT_CALL(*mockSession, close(restbed::CREATED, StrEq(responseBody), _))
      .Times(1);

  controller.Create(mockSession);
}

TEST_F(TestVoteController, Create_UserNotFound) {
  // Set up the request body to contain a user id
  json requestJson;
  requestJson["createdBy"] = fakeUser.id;
  requestBody = requestJson.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(requestBody);

  // Set the necessary request parameters
  request->set_path("/issues/" + fakeIssueId + "/votes");
  request->set_body(requestBody);
  request->set_header("Content-Length", std::to_string(requestBody.size()));

  // The controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // The controller should fetch the request body and invoke the callback to
  // process the body
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  // We should call the service to get any votes by the user for the issue. We
  // fake there being none, thus we will try to create a new vote
  StringMap queryParams = {{"issueId", fakeIssueId},
                           {"createdBy", fakeUser.id}};
  EXPECT_CALL(*mockService, Get(queryParams))
      .Times(1)
      .WillOnce(Return(std::vector<Vote>{}));

  // Fake the UserService within the VoteService not finding the user
  EXPECT_CALL(*mockService, Create(requestBody))
      .WillOnce(Throw(NotFoundError("fake user not found error")));

  // The controller should close the session with a NOT_FOUND error response
  EXPECT_CALL(*mockSession, close(restbed::NOT_FOUND, StrNe(""), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestVoteController, Create_ServerError) {
  // Set up the request body to contain a user id
  json requestJson;
  requestJson["createdBy"] = fakeUser.id;
  requestBody = requestJson.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(requestBody);

  // Set the necessary request parameters
  request->set_path("/issues/" + fakeIssueId + "/votes");
  request->set_body(requestBody);
  request->set_header("Content-Length", std::to_string(requestBody.size()));

  // The controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // The controller should fetch the request body and invoke the callback to
  // process the body
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  // we fake the service throwing a server error
  StringMap queryParams = {{"issueId", fakeIssueId},
                           {"createdBy", fakeUser.id}};
  EXPECT_CALL(*mockService, Get(queryParams))
      .WillOnce(Throw(InternalServerError("fake server error")));

  // We should close the session with an INTERNAL_SERVER_ERROR status
  EXPECT_CALL(*mockSession, close(restbed::INTERNAL_SERVER_ERROR, StrNe(""), _))
      .Times(1);

  controller.Create(mockSession);
}

TEST_F(TestVoteController, Create_BadRequest) {
  // Set up the request body to contain a user id
  json requestJson;
  requestJson["createdBy"] = fakeUser.id;
  requestBody = requestJson.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(requestBody);

  // Set the necessary request parameters
  request->set_path("/issues/" + fakeIssueId + "/votes");
  request->set_body(requestBody);
  request->set_header("Content-Length", std::to_string(requestBody.size()));

  // The controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // The controller should fetch the request body and invoke the callback to
  // process the body
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  // we fake the service throwing a server error
  StringMap queryParams = {{"issueId", fakeIssueId},
                           {"createdBy", fakeUser.id}};
  EXPECT_CALL(*mockService, Get(queryParams))
      .WillOnce(Throw(BadRequestError("fake bad request error")));

  // We should close the session with an BAD_REQUEST status
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestVoteController, Create_InvalidPath) {
  // Set the request path to have an id attached
  request->set_path("/issues/" + fakeIssueId + "/votes/1234");

  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // The controller should close the session with a BAD_REQUEST and some kind of
  // error message
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestVoteController, Create_NoRequest) {
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(nullptr));

  // The controller should close the session with a BAD_REQUEST and some kind of
  // error message
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestVoteController, Delete_ValidDelete) {
  // Set up the request body
  json requestJson;
  requestJson["createdBy"] = fakeUser.id;
  requestBody = requestJson.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(requestBody);

  // Set the necessary request parameters
  request->set_path("/issues/" + fakeIssueId + "/votes");
  request->set_body(requestBody);
  request->set_header("Content-Length", std::to_string(requestBody.size()));

  // The controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // The controller should fetch the request body and invoke the callback to
  // process the body
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  // We should call the service to get any votes by the user for the issue. We
  // fake there being one, thus we will delete it
  StringMap queryParams = {{"issueId", fakeIssueId},
                           {"createdBy", fakeUser.id}};
  std::vector<Vote> fakeVotes = {vote};
  EXPECT_CALL(*mockService, Get(queryParams))
      .Times(1)
      .WillOnce(Return(fakeVotes));

  // The service should successfully delete the vote
  EXPECT_CALL(*mockService, Delete(vote.id)).WillOnce(Return(true));

  // The controller should close the session with a status of NO_CONTENT to
  // notify the caller it was deleted successfully
  EXPECT_CALL(*mockSession, close(restbed::NO_CONTENT, StrEq(""), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestVoteController, UnusedMethods) {
  // This test just makes sure the unused methods don't do anything
  EXPECT_CALL(*mockSession, get_request).Times(0);
  EXPECT_CALL(*mockSession, get_path_parameter).Times(0);
  EXPECT_CALL(*mockSession, close).Times(0);
  EXPECT_CALL(*mockSession, fetch).Times(0);

  controller.Update(mockSession);
  controller.Delete(mockSession);
}
