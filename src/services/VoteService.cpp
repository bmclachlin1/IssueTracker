#include "VoteService.h"
#include "IssueService.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "FileHandler.h"
#include "User.h"
#include "Issue.h"
#include "UserService.h"
#include "Vote.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

std::vector<Vote> VoteService::Get(
    const std::multimap<std::string, std::string> queryParams) {
  std::vector<Vote> votes;
  json jsonFile = _fileHandler->read();

  json filtered = Filter(jsonFile, queryParams);

  for (auto& vote : filtered) {
    Vote temp = vote.get<Vote>();

    // Get the user from the user service
    User createdBy = _userService->Get(std::string(temp.createdBy.id));
    votes.push_back(temp);
  }

  return votes;
}

Vote VoteService::Get(std::string id) {
  json jsonFile = _fileHandler->read();

  // Get the vote by the id
  json filtered = Filter(jsonFile, {{"id", id}});

  // If the user was not found
  if (filtered.empty()) {
    throw NotFoundError(
        std::string("A Vote could not be found with the following id: " + id)
            .c_str());
  }
  Vote vote;
  vote = filtered[0].get<Vote>();
  vote.createdBy = _userService->Get(std::string(vote.createdBy.id));
  return vote;
}

Vote VoteService::Create(std::string body) {
  json voteToCreate;
  try {
    voteToCreate = json::parse(body);
  } catch (std::exception&) {
    throw BadRequestError(
        std::string(
            "An error occurred parsing the following Vote information: " + body)
            .c_str());
  }

  // We explicitly ignore these values for votes
  voteToCreate["updatedBy"] = "";
  voteToCreate["updatedAt"] = "";

  // We also ignore any specified date and compute this ourselves
  voteToCreate["createdAt"] = "";

  json jsonFile = _fileHandler->read();

  Vote vote;
  try {
    // Attempt to deserialize the json body.
    vote = voteToCreate.get<Vote>();
  } catch (std::exception& e) {
    throw BadRequestError(
        std::string(
            "Unable to create a Vote using the following information: " + body)
            .c_str());
  }
  // Ignore any id value provided
  vote.id = GenerateId();
  // If the file has a vote with that id, generate a new one
  while (!Filter(jsonFile, {{"id", vote.id}}).empty()) {
    vote.id = GenerateId();
  }

  // Get the user from the User Service
  vote.createdBy = _userService->Get(std::string(vote.createdBy.id));

 /*TODO(someone) : we need to initialize an issueService.
  if (!vote.issueId.empty()) {
      json tempIssue;
      Issue issue = _issueService->Get(std::string(vote.issueId));
      issue.votes.push_back(vote);

      tempIssue = issue;
      _issueService->Update(tempIssue.dump());
  }
 */
  // Set the created time to right now (in UTC time)
  time_t t = time(0);
  struct tm now = *gmtime(&t);
  vote.createdAt = now;

  jsonFile.push_back(vote);
  _fileHandler->write(jsonFile);

  return vote;
}

Vote VoteService::Update(std::string body) {
  throw NotImplementedError("Votes cannot be updated. Only created or deleted");
}

bool VoteService::Delete(std::string id) {
  json jsonFile = _fileHandler->read();

  // Find the Vote with the passed in id
  auto itVote =
      std::find_if(jsonFile.begin(), jsonFile.end(),
                   [&](const json& item) { return item["id"] == id; });

  // If the Vote was found
  if (itVote != jsonFile.end()) {
    jsonFile.erase(itVote);
    _fileHandler->write(jsonFile);
    return true;
  } else {
    throw NotFoundError(
        std::string("The Vote could not be found with the following id: " + id)
            .c_str());
  }
  return false;
}
