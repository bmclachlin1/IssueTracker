#ifndef MOCK_VOTE_SERVICE_H
#define MOCK_VOTE_SERVICE_H

#include "VoteService.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Vote.h"
#include "gmock/gmock.h"

/**
 * @class MockVoteService
 * Mock of the VoteService class. Internally, creates a default UserService,
 * which should never be used
 */
class MockVoteService : public VoteService {
 public:
  MockVoteService() : VoteService(nullptr) {}
  virtual ~MockVoteService() {}

  MOCK_METHOD1(Get, std::vector<Vote>(std::multimap<std::string, std::string>));
  MOCK_METHOD1(Get, Vote(std::string));
  MOCK_METHOD1(Create, Vote(std::string));
  MOCK_METHOD1(Delete, bool(std::string));
};

#endif  // MOCK_VOTE_SERVICE_H