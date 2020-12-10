#ifndef MOCK_ISSUE_SERVICE_H
#define MOCK_ISSUE_SERVICE_H

#include <map>
#include <string>
#include <memory>
#include <vector>

#include "Issue.h"
#include "IssueService.h"
#include "UserService.h"
#include "CommentService.h"
#include "VoteService.h"
#include "gmock/gmock.h"

/**
 * @class MockIssueService
 * Mock of the IssueService to allow testing the controller
 */
class MockIssueService : public IssueService {
 public:
  MockIssueService() 
  : IssueService(nullptr, nullptr, nullptr) {}
  virtual ~MockIssueService() {}

  MOCK_METHOD1(Get, Issue(const std::string));
  MOCK_METHOD1(
      Get, std::vector<Issue>(const std::multimap<std::string, std::string>));
  MOCK_METHOD1(Create, Issue(std::string));
  MOCK_METHOD1(Update, Issue(std::string));
  MOCK_METHOD1(Delete, bool(std::string));
};

#endif  // MOCK_ISSUE_SERVICE_H