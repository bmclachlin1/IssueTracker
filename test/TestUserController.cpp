#include <restbed>

#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Exceptions.h"
#include "MockSession.h"
#include "MockUserService.h"
#include "User.h"
#include "UserController.hpp"
#include "UserService.h"
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
class TestUserController : public ::testing::Test {
 protected:
  // Variables used throughout the tests
  UserController<MockSession> controller;
  std::shared_ptr<MockSession> mockSession;
  std::shared_ptr<MockUserService> mockService;
  std::shared_ptr<restbed::Request> request;
  User user;
  json jsonUser;
  std::string body;

  void SetUp() override {
    // give the user an id
    user.id = "testUser";

    // Instantiate the test variables
    mockSession = std::make_shared<MockSession>();
    mockService = std::make_shared<MockUserService>();
    request = std::make_shared<restbed::Request>();

    // Set the user service to the mocked user service
    controller.SetEntityService(mockService);
  }
};

TEST_F(TestUserController, Get_OneUser_ProperRequest) {
  // Set up the request body
  jsonUser = user;
  body = jsonUser.dump();

  // Add the user id to the request path
  request->set_path("/users/" + user.id);

  // Controller should call the UserService once with the passed in user id
  EXPECT_CALL(*mockService, Get(user.id)).WillOnce(Return(user));

  // Controller should send back the user as a JSON string, with a status of OK
  // if the request is valid
  EXPECT_CALL(*mockSession, close(restbed::OK, StrEq(body), _)).Times(1);

  // Controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  controller.Get(mockSession);
}

TEST_F(TestUserController, Get_OneUser_NotFound) {
  // Fake the user service throwing a not found error
  EXPECT_CALL(*mockService, Get(user.id))
      .WillOnce(Throw(NotFoundError("fake not found exception")));

  // Add the user id to the request path
  request->set_path("/users/" + user.id);

  // The controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back a NOT FOUND response if the user specified
  // doesn't exist, with a non-empty error message in the response body
  EXPECT_CALL(*mockSession, close(restbed::NOT_FOUND, StrNe(""), _)).Times(1);

  controller.Get(mockSession);
}

TEST_F(TestUserController, Get_OneUser_ServerError) {
  // Fake the user service throwing an error while performing the request
  EXPECT_CALL(*mockService, Get(user.id))
      .WillOnce(Throw(InternalServerError("fake server error")));

  // Add the user id to the request path
  request->set_path("/users/" + user.id);

  // The controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back an INTERNAL_SERVER_ERROR if the user service
  // encountered an error while getting the users, with a non-empty error
  // message in the response body
  EXPECT_CALL(*mockSession, close(restbed::INTERNAL_SERVER_ERROR, StrNe(""), _))
      .Times(1);

  controller.Get(mockSession);
}

TEST_F(TestUserController, Get_AllUsers_NoQuery) {
  jsonUser = user;

  // Add the users endpoint to the path
  request->set_path("/users/");

  // Fake the user service sending back a vector of users
  std::vector<User> users = {user};
  EXPECT_CALL(*mockService, Get(::testing::A<StringMap>()))
      .WillOnce(Return(users));

  // Controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back an OK for any get request to /users/, with the
  // serialized user in the response body
  EXPECT_CALL(*mockSession, close(restbed::OK, HasSubstr(jsonUser.dump()), _))
      .Times(1);

  controller.Get(mockSession);
}

TEST_F(TestUserController, Get_AllUsers_WithValidQuery) {
  jsonUser = user;

  // Add the users endpoint to the path
  request->set_path("/users/");
  request->set_query_parameter("role", user.role);

  // Fake the user service sending back a vector of users
  std::vector<User> users = {user};

  EXPECT_CALL(*mockService, Get(::testing::An<StringMap>()))
      .WillOnce(Return(users));

  // Controller should get the request from the session
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back an OK for any get request to /users/, with the
  // serialized user in the response body
  EXPECT_CALL(*mockSession, close(restbed::OK, HasSubstr(jsonUser.dump()), _))
      .Times(1);

  controller.Get(mockSession);
}

TEST_F(TestUserController, Get_AllUsers_ServerError) {
  // Add the users endpoint to the path
  request->set_path("/users/");

  // Specify the query params (which should be empty)
  StringMap queryParams = request->get_query_parameters();

  // Fake the user service throwing an error while retrieving the users
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

TEST_F(TestUserController, Get_NoRequest) {
  // Fake the session not having a request
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(nullptr));

  // Controller should send back a BAD REQUEST response if there is no request
  // passed in
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, _, _)).Times(1);

  controller.Get(mockSession);
}

TEST_F(TestUserController, Create_ProperRequest) {
  // Set up the test user
  user.name = "Testy McTesterton";
  jsonUser = user;
  body = jsonUser.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);

  // Fake the user service sending back the user upon creating it
  EXPECT_CALL(*mockService, Create(StrEq(body)))
      .Times(1)
      .WillOnce(Return(user));

  // The controller should get the request from the session
  request->add_header("Content-Length", std::to_string(body.size()));
  EXPECT_CALL(*mockSession, get_request()).Times(1).WillOnce(Return(request));

  // The controller should fetch the request body and invoke the callback to
  // process the body
  EXPECT_CALL(*mockSession, fetch(_, _))
      .Times(1)
      .WillOnce(
          InvokeArgument<1>(std::ref(mockSession), std::ref(bodyAsBytes)));

  // Controller should send back a CREATED for a valid post to /users/, with the
  // serialized user in the body of the response
  EXPECT_CALL(*mockSession, close(restbed::CREATED, StrEq(body), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestUserController, Create_NoName) {
  // Make sure the test user has no name
  user.name = "";
  jsonUser = user;
  body = jsonUser.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);

  // FaKe the user service throwing an error when finding a user with no name
  EXPECT_CALL(*mockService, Create(StrEq(body)))
      .Times(1)
      .WillOnce(Throw(BadRequestError("fake no name error")));

  // Set the body of the request to the serialized user
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

  // Controller should send back a BAD REQUEST if the user service throws an
  // exception, with a non-empty error message in the body
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestUserController, Create_EmptyBody) {
  // Create an empty json object for the empty body
  jsonUser = json::object();
  body = jsonUser.dump();

  // Set the request body to the empty json object
  request->set_body(body);

  // The controller should exit before calling the user service
  EXPECT_CALL(*mockService, Create(_)).Times(0);

  // The controller should get the request from the sesssion
  EXPECT_CALL(*mockSession, get_request()).Times(1).WillOnce(Return(request));

  // Controller should send back a BAD_REQUEST for an empty request body, with a
  // non-empty error message in the reponse body
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestUserController, Create_ServerError) {
  // Add something to the request body
  std::string body = "some kind of body";
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);
  request->set_body(body);
  request->set_header("Content-Length", std::to_string(body.size()));

  // The controller should attempt to create the user, but the service will
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

TEST_F(TestUserController, Create_NoRequest) {
  // The controller should get the request from the session once, we fake the
  // session having no request
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(nullptr));

  // Controller should send back a BAD REQUEST response if there is no request
  // passed in, with a non-empty error message in the response body
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  controller.Create(mockSession);
}

TEST_F(TestUserController, Update_ProperRequest) {
  // Make sure the user has a name, in order for a proper request to be made
  user.name = "Testy McTestface";
  jsonUser = user;
  body = jsonUser.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);

  // The controller should call the user service once, we fake the user service
  // returning the user once its updated
  EXPECT_CALL(*mockService, Update(StrEq(body)))
      .Times(1)
      .WillOnce(Return(user));

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

TEST_F(TestUserController, Update_EmptyBody) {
  // Create an empty json object for the empty body
  jsonUser = json::object();
  body = jsonUser.dump();

  // The user service should not be called with an empty request body
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

TEST_F(TestUserController, Update_UserNotFound) {
  // Create the test user
  user.name = "Testy McTestface";
  jsonUser = user;
  body = jsonUser.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);

  // Fake the user service not finding the user
  EXPECT_CALL(*mockService, Update(StrEq(body)))
      .Times(1)
      .WillOnce(Throw(NotFoundError("fake user not found error")));

  // Set the request body and header
  request->add_header("Content-Length", std::to_string(body.size()));
  request->set_body(body);

  // The controller should get the request from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should return a NOT FOUND if the user in the request body could
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

TEST_F(TestUserController, Update_ServerError) {
  // Create the test user
  user.name = "Testy McTestface";
  jsonUser = user;
  body = jsonUser.dump();
  restbed::Bytes bodyAsBytes = restbed::String::to_bytes(body);

  // Fake the user service throwing an error
  EXPECT_CALL(*mockService, Update(StrEq(body)))
      .Times(1)
      .WillOnce(Throw(InternalServerError("Fake server error")));

  // Set the request body to the serialized user
  request->add_header("Content-Length", std::to_string(body.size()));
  request->set_body(body);

  // The controller should get the requst from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should return an INTERNAL_SERVER_ERROR if the user service
  // encounters an error while updating the user, with a non-empty error message
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

TEST_F(TestUserController, Delete_ProperRequest) {
  // The controller should call the user service once
  EXPECT_CALL(*mockService, Delete(StrEq(user.id)))
      .Times(1)
      .WillOnce(Return(true));

  // Set the request path the user id
  request->set_path("/users/" + user.id);

  // The controller should get the request from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back an OK if the delete was successful, with an
  // empty resposne body
  EXPECT_CALL(*mockSession, close(restbed::OK, StrEq(""), _)).Times(1);

  controller.Delete(mockSession);
}

TEST_F(TestUserController, Delete_NoId) {
  // The user service shouldn't be called if there is no id provided
  EXPECT_CALL(*mockService, Delete(_)).Times(0);

  // Set the request path without the user id
  request->set_path("/users/");

  // The controller should get the request from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back a BAD_REQUEST if the request path does not
  // contain an id parameter, with a non-empty error message on the response
  // body
  EXPECT_CALL(*mockSession, close(restbed::BAD_REQUEST, StrNe(""), _)).Times(1);

  controller.Delete(mockSession);
}

TEST_F(TestUserController, Delete_NotFound) {
  // Fake the user service throwing an error if the user is not found
  EXPECT_CALL(*mockService, Delete(StrEq(user.id)))
      .Times(1)
      .WillOnce(Throw(NotFoundError("fake not found error")));

  // Set the request path the user id
  request->set_path("/users/" + user.id);

  // The controller should get the request from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back a NOT_FOUND if the requested user could not be
  // found
  EXPECT_CALL(*mockSession, close(restbed::NOT_FOUND, StrNe(""), _)).Times(1);

  controller.Delete(mockSession);
}

TEST_F(TestUserController, Delete_ServerError) {
  // Fake the user service throwing an error during the deletion proces
  EXPECT_CALL(*mockService, Delete(StrEq(user.id)))
      .Times(1)
      .WillOnce(Throw(InternalServerError("fake service error")));

  // Set the request path the user id
  request->set_path("/users/" + user.id);

  // The controller should get the request from the session once
  EXPECT_CALL(*mockSession, get_request()).WillOnce(Return(request));

  // Controller should send back a INTERNAL_SERVER_ERROR if the server
  // encountered an error while processing the delete, with a non-empty error
  // message on the response body
  EXPECT_CALL(*mockSession, close(restbed::INTERNAL_SERVER_ERROR, StrNe(""), _))
      .Times(1);

  controller.Delete(mockSession);
}
