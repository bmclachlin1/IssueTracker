#ifndef MOCK_USER_SERVICE_H
#define MOCK_USER_SERVICE_H

#include <map>
#include <string>

#include "User.h"
#include "UserService.h"
#include "gmock/gmock.h"

/**
 * @class MockUserService
 * Mock of the UserService to allow testing the controller
 */
class MockUserService : public UserService {
 public:
  MockUserService() {}
  ~MockUserService() {}

  MOCK_METHOD1(Get, User(const std::string));
  MOCK_METHOD1(
      Get, std::vector<User>(const std::multimap<std::string, std::string>));
  MOCK_METHOD1(Create, User(std::string));
  MOCK_METHOD1(Update, User(std::string));
  MOCK_METHOD1(Delete, bool(std::string));
};

#endif  // MOCK_USER_SERVICE_H