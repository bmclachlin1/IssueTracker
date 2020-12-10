#ifndef MOCK_SESSION_H
#define MOCK_SESSION_H

#include <restbed>

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "gmock/gmock.h"

/**
 * A fake session that will be used as a template parameter for the controller
 * classes
 */
class MockSession {
 public:
  MockSession() {}
  ~MockSession() {}
  MOCK_METHOD0(get_request, const std::shared_ptr<restbed::Request>());
  MOCK_METHOD1(get_path_parameter, std::string(std::string));
  MOCK_METHOD3(close, void(const int status, const std::string&,
                           const std::multimap<std::string, std::string>&));
  MOCK_METHOD2(
      fetch, void(const std::size_t,
                  const std::function<void(const std::shared_ptr<MockSession>&,
                                           const restbed::Bytes&)>&));
};

#endif  // MOCK_SESSION_H