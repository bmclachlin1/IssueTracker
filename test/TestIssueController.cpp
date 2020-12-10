#include <restbed>

#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Exceptions.h"
#include "MockSession.h"
#include "MockIssueService.h"
#include "Issue.h"
#include "IssueController.hpp"
#include "IssueService.h"
#include "Utilities.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

using ::testing::_;
using ::testing::HasSubstr;
using ::testing::InvokeArgument;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::StrNe;
using ::testing::Throw;
using ::testing::TypedEq;

using json = nlohmann::json;

/**
 * Fixture class for bootstrapping the test process
 */
class TestIssueController : public ::testing::Test {
 protected:
  // Variables used throughout the tests
  IssueController<MockSession> controller;
  std::shared_ptr<MockSession> mockSession;
  std::shared_ptr<MockIssueService> mockService;
  std::shared_ptr<restbed::Request> request;
  Issue issue;
  json jsonIssue;
  std::string body;

  void SetUp() override {
    // give the issue an id
    issue.id = "testIssue";

    // Instantiate the test variables
    mockSession = std::make_shared<MockSession>();
    mockService = std::make_shared<MockIssueService>();
    request = std::make_shared<restbed::Request>();

    controller.SetEntityService(mockService);
  }
};

TEST_F(TestIssueController, Get_OneIssue_ProperRequest) {
  // Set up the request body
  jsonIssue = issue;
  body = jsonIssue.dump();

  // Add the issue id to the request path
  request->set_path("/issues/" + issue.id);

  // Controller should call the IssueService once with the passed in issue id
  EXPECT_CALL(*mockService, Get(issue.id)).WillOnce(Return(issue));

  // Controller should send back the issue as a JSON string, with a status of OK
  // if the request is valid
  EXPECT_CALL(*mockSession, close(restbed::OK, StrEq(body), _)).Times(1);

  // Controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  controller.Get(mockSession);
}

TEST_F(TestIssueController, Get_OneIssue_NotFound) {
  // Fake the issue service throwing a not found error
  EXPECT_CALL(*mockService, Get(issue.id))
      .WillOnce(Throw(NotFoundError("fake not found exception")));

  // Add the issue id to the request path
  request->set_path("/issues/" + issue.id);

  // The controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back a NOT FOUND response if the issue specified
  // doesn't exist, with a non-empty error message in the response body
  EXPECT_CALL(*mockSession, close(restbed::NOT_FOUND, StrNe(""), _)).Times(1);

  controller.Get(mockSession);
}

TEST_F(TestIssueController, Get_OneIssue_ServerError) {
  // Fake the issue service throwing an error while performing the request
  EXPECT_CALL(*mockService, Get(issue.id))
      .WillOnce(Throw(InternalServerError("fake server error")));

  // Add the issue id to the request path
  request->set_path("/issues/" + issue.id);

  // The controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back an INTERNAL_SERVER_ERROR if the issue service
  // encountered an error while getting the issues, with a non-empty error
  // message in the response body
  EXPECT_CALL(*mockSession, close(restbed::INTERNAL_SERVER_ERROR, StrNe(""), _))
      .Times(1);

  controller.Get(mockSession);
}

TEST_F(TestIssueController, Get_AllIssues_NoQuery) {
  jsonIssue = issue;

  // Add the issues endpoint to the path
  request->set_path("/issues/");

  // Fake the issue service sending back a vector of issues
  std::vector<Issue> issues = {issue};
  EXPECT_CALL(*mockService, Get(::testing::A<StringMap>()))
      .WillOnce(Return(issues));

  // Controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back an OK for any get request to /issues/, with the
  // serialized issue in the response body
  EXPECT_CALL(*mockSession, close(restbed::OK, HasSubstr(jsonIssue.dump()), _))
      .Times(1);

  controller.Get(mockSession);
}

TEST_F(TestIssueController, Get_AllIssues_WithValidQuery) {
  jsonIssue = issue;

  // Add the issues endpoint to the path
  request->set_path("/issues/");
  request->set_query_parameter("status", issue.status);

  // Fake the issue service sending back a vector of issues
  std::vector<Issue> issues = {issue};

  EXPECT_CALL(*mockService, Get(::testing::An<StringMap>()))
      .WillOnce(Return(issues));

  // Controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back an OK for any get request to /issues/, with the
  // serialized issue in the response body
  EXPECT_CALL(*mockSession, close(restbed::OK, HasSubstr(jsonIssue.dump()), _))
      .Times(1);

  controller.Get(mockSession);
}

TEST_F(TestIssueController, Get_AllIssues_ServerError) {
  // Add the issues endpoint to the path
  request->set_path("/issues/");

  // Specify the query params (which should be empty)
  StringMap queryParams = request->get_query_parameters();

  // Fake the issue service throwing an error while retrieving the issues
  EXPECT_CALL(*mockService, Get(::testing::An<StringMap>()))
      .WillOnce(Throw(InternalServerError("fake error")));

  // Controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back an INTERNAL_SERVER_ERROR if an error occurs,
  // with a nonempty error message in the body
  EXPECT_CALL(*mockSession, close(restbed::INTERNAL_SERVER_ERROR, StrNe(""), _))
      .Times(1);

  controller.Get(mockSession);
}

TEST_F(TestIssueController, Get_NoRequest) {
  // Fake the session not having a request
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(nullptr));

  // Controller should send back a BAD REQUEST response if there is no request
  // passed in
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, _, _)).Times(1);

  controller.Get(mockSession);
}

TEST_F(TestIssueController, Create_ProperRequest) {
  // Set up the test issue
  issue.title = "Testy McTesterton";
  jsonIssue = issue;
  body = jsonIssue.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);

  // Fake the issue service sending back the issue upon creating it
  EXPECT_CALL(*mockService, Create(StrEq(body)))
      .Times(1)
      .WillOnce(Return(issue));

  // The controller should get the request from the session
  request->add_header("Content-Length", std::to_string(body.size()));
  EXPECT_CALL(*mockSession, get_request()).Times(1).WillOnce(Return(request));

  // The controller should fetch the request body and invoke the callback to
  // process the body
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  // Controller should send back a CREATED for a valid post to /issues/, with the
  // serialized issue in the body of the response
  EXPECT_CALL(*mockSession, close(restbed::CREATED, StrEq(body), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestIssueController, Create_NoName) {
  // Make sure the test issue has no name
  issue.title = "";
  jsonIssue = issue;
  body = jsonIssue.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);

  // FaKe the issue service throwing an error when finding a issue with no name
  EXPECT_CALL(*mockService, Create(StrEq(body)))
      .Times(1)
      .WillOnce(Throw(BadRequestError("fake no name error")));

  // Set the body of the request to the serialized issue
  request->set_body(body);
  request->add_header("Content-Length", std::to_string(body.size()));

  // The controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).Times(1).WillOnce(Return(request));

  // The controller should fetch the request body and invoke the callback to
  // process the body
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  // Controller should send back a BAD REQUEST if the issue service throws an
  // exception, with a non-empty error message in the body
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestIssueController, Create_EmptyBody) {
  // Create an empty json object for the empty body
  jsonIssue = json::object();
  body = jsonIssue.dump();

  // Set the request body to the empty json object
  request->set_body(body);

  // The controller should exit before calling the issue service
  EXPECT_CALL(*mockService, Create(_)).Times(0);

  // The controller should get the request from the sesssion
  EXPECT_CALL(*mockSession, get_request()).Times(1).WillOnce(Return(request));

  // Controller should send back a BAD_REQUEST for an empty request body, with a
  // non-empty error message in the reponse body
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestIssueController, Create_ServerError) {
  // Add something to the request body
  std::string body = "some kind of body";
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);
  request->set_body(body);
  request->set_header("Content-Length", std::to_string(body.size()));

  // The controller should attempt to create the issue, but the service will
  // throw an error in the process
  EXPECT_CALL(*mockService, Create(body))
      .WillOnce(Throw(InternalServerError("fake service error")));

  // The controller should get the request from the session once
  EXPECT_CALL(*mockSession, get_request()).Times(1).WillOnce(Return(request));

  // The controller should fetch the request body and invoke the callback to
  // process the body, which will throw the error
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  // Controller should send back an INTERNAL_SERVER_ERROR if something else went
  // wrong with the request, with a non-empty error message in the body
  EXPECT_CALL(*mockSession, close(restbed::INTERNAL_SERVER_ERROR, StrNe(""), _))
      .Times(1);

  controller.Create(mockSession);
}

TEST_F(TestIssueController, Create_NoRequest) {
  // The controller should get the request from the session once, we fake the
  // session having no request
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(nullptr));

  // Controller should send back a BAD REQUEST response if there is no request
  // passed in, with a non-empty error message in the response body
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestIssueController, Update_ProperRequest) {
  // Make sure the issue has a name, in order for a proper request to be made
  issue.title = "Testy McTestface";
  jsonIssue = issue;
  body = jsonIssue.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);

  // The controller should call the issue service once, we fake the issue service
  // returning the issue once its updated
  EXPECT_CALL(*mockService, Update(StrEq(body)))
      .Times(1)
      .WillOnce(Return(issue));

  // Set up the body and header for the request
  request->add_header("Content-Length", std::to_string(body.size()));
  request->set_body(body);

  // The controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back an OK if the update is valid
  EXPECT_CALL(*mockSession, close(restbed::OK, StrEq(body), _)).Times(1);

  // The controller should fetch the request body and invoke the callback to
  // process the body
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  controller.Update(mockSession);
}

TEST_F(TestIssueController, Update_EmptyBody) {
  // Create an empty json object for the empty body
  jsonIssue = json::object();
  body = jsonIssue.dump();

  // The issue service should not be called with an empty request body
  EXPECT_CALL(*mockService, Update(StrEq(body))).Times(0);

  // The controller should get the request from the session once
  // We explicitly didn't set a body or headers here so it will fail
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // The controller should send back a BAD_REQUEST if the request is empty, with
  // a non-empty error message in the response body
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  // The controller shouldn't call the fetch method
  EXPECT_CALL(*mockSession, fetch(_, _)).Times(0);

  controller.Update(mockSession);
}

TEST_F(TestIssueController, Update_IssueNotFound) {
  // Create the test issue
  issue.title = "Testy McTestface";
  jsonIssue = issue;
  body = jsonIssue.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);

  // Fake the issue service not finding the issue
  EXPECT_CALL(*mockService, Update(StrEq(body)))
      .Times(1)
      .WillOnce(Throw(NotFoundError("fake issue not found error")));

  // Set the request body and header
  request->add_header("Content-Length", std::to_string(body.size()));
  request->set_body(body);

  // The controller should get the request from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should return a NOT FOUND if the issue in the request body could
  // not be found
  EXPECT_CALL(*mockSession, close(restbed::NOT_FOUND, StrNe(""), _)).Times(1);

  // The controller should fetch the request body and invoke the callback to
  // process the body
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  controller.Update(mockSession);
}

TEST_F(TestIssueController, Update_ServerError) {
  // Create the test issue
  issue.title = "Testy McTestface";
  jsonIssue = issue;
  body = jsonIssue.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);

  // Fake the issue service throwing an error
  EXPECT_CALL(*mockService, Update(StrEq(body)))
      .Times(1)
      .WillOnce(Throw(InternalServerError("Fake server error")));

  // Set the request body to the serialized issue
  request->add_header("Content-Length", std::to_string(body.size()));
  request->set_body(body);

  // The controller should get the requst from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should return an INTERNAL_SERVER_ERROR if the issue service
  // encounters an error while updating the issue, with a non-empty error message
  // in the response body
  EXPECT_CALL(*mockSession, close(restbed::INTERNAL_SERVER_ERROR, StrNe(""), _))
      .Times(1);

  // The controller should fetch the request body and invoke the callback to
  // process the body
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  controller.Update(mockSession);
}

TEST_F(TestIssueController, Delete_ProperRequest) {
  // The controller should call the issue service once
  EXPECT_CALL(*mockService, Delete(StrEq(issue.id)))
      .Times(1)
      .WillOnce(Return(true));

  // Set the request path the issue id
  request->set_path("/issues/" + issue.id);

  // The controller should get the request from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back an OK if the delete was successful, with an
  // empty resposne body
  EXPECT_CALL(*mockSession, close(restbed::OK, StrEq(""), _)).Times(1);

  controller.Delete(mockSession);
}

TEST_F(TestIssueController, Delete_NoId) {
  // The issue service shouldn't be called if there is no id provided
  EXPECT_CALL(*mockService, Delete(_)).Times(0);

  // Set the request path without the issue id
  request->set_path("/issues/");

  // The controller should get the request from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back a BAD_REQUEST if the request path does not
  // contain an id parameter, with a non-empty error message on the response
  // body
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  controller.Delete(mockSession);
}

TEST_F(TestIssueController, Delete_NotFound) {
  // Fake the issue service throwing an error if the issue is not found
  EXPECT_CALL(*mockService, Delete(StrEq(issue.id)))
      .Times(1)
      .WillOnce(Throw(NotFoundError("fake not found error")));

  // Set the request path the issue id
  request->set_path("/issues/" + issue.id);

  // The controller should get the request from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back a NOT_FOUND if the requested issue could not be
  // found
  EXPECT_CALL(*mockSession, close(restbed::NOT_FOUND, StrNe(""), _)).Times(1);

  controller.Delete(mockSession);
}

TEST_F(TestIssueController, Delete_ServerError) {
  // Fake the issue service throwing an error during the deletion proces
  EXPECT_CALL(*mockService, Delete(StrEq(issue.id)))
      .Times(1)
      .WillOnce(Throw(InternalServerError("fake service error")));

  // Set the request path the issue id
  request->set_path("/issues/" + issue.id);

  // The controller should get the request from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back a INTERNAL_SERVER_ERROR if the server
  // encountered an error while processing the delete, with a non-empty error
  // message on the response body
  EXPECT_CALL(*mockSession, close(restbed::INTERNAL_SERVER_ERROR, StrNe(""), _))
      .Times(1);

  controller.Delete(mockSession);
}
