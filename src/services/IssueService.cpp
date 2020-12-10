#include "IssueService.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Comment.h"
#include "FileHandler.h"
#include "Issue.h"
#include "User.h"
#include "UserService.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

std::vector<Issue> IssueService::Get(
    const std::multimap<std::string, std::string> queryParams) {
  std::vector<Issue> issues;
  json jsonFile = _fileHandler->read();

  json filteredIssues = Filter(jsonFile, queryParams);

  for (auto& _issue : filteredIssues) {
    Issue issue = _issue.get<Issue>();

    // createdBy and reporter are mandotory fields, so we get them automatically
    issue.createdBy = _userService->Get(std::string(issue.createdBy.id));
    issue.reporter = _userService->Get(std::string(issue.reporter.id));

    // updatedBy and assignedTo might not have value, so we need to check first
    if (!issue.updatedBy.id.empty())
      issue.updatedBy = _userService->Get(std::string(issue.updatedBy.id));

    if (!issue.assignedTo.id.empty())
      issue.assignedTo = _userService->Get(std::string(issue.assignedTo.id));

    issue.votes = _voteService->Get(StringMap{{"issueId", issue.id}});
    auto comments = _commentService->Get(StringMap{{"issueId", issue.id}});
    issue.comments = std::multiset<Comment>(comments.begin(), comments.end());

    issues.push_back(issue);
  }

  return issues;
}

Issue IssueService::Get(const std::string id) {
  json jsonFile = _fileHandler->read();

  json filtered = Filter(jsonFile, {{"id", id}});

  if (filtered.empty()) {
    throw NotFoundError(
        std::string("An issue could not be found with the following id: " + id)
            .c_str());
  }

  Issue issue = filtered[0].get<Issue>();

  // createdBy and reporter are mandotory fields, so we get them automatically
  issue.createdBy = _userService->Get(std::string(issue.createdBy.id));
  issue.reporter = _userService->Get(std::string(issue.reporter.id));

  // updatedBy and assignedTo might not have value, so we need to check first
  if (!issue.updatedBy.id.empty())
    issue.updatedBy = _userService->Get(std::string(issue.updatedBy.id));

  if (!issue.assignedTo.id.empty())
    issue.assignedTo = _userService->Get(std::string(issue.assignedTo.id));

  issue.votes = _voteService->Get(StringMap{{"issueId", issue.id}});
  auto comments = _commentService->Get(StringMap{{"issueId", issue.id}});
  issue.comments = std::multiset<Comment>(comments.begin(), comments.end());

  return issue;
}

Issue IssueService::Create(std::string body) {
  json issueToCreate;
  std::string description;
  try {
    issueToCreate = json::parse(body);

    // Create the description from the the optional description field of the
    // request json
    if (issueToCreate.find("description") != issueToCreate.end()) {
      description = issueToCreate["description"].get<std::string>();
    }
  } catch (const std::exception& exp) {
    std::cout << exp.what() << std::endl;
    throw BadRequestError(
        std::string(
            "An error occured parsing the following issue information: " + body)
            .c_str());
  }

  // We explicitly ignore these values when creating issues
  issueToCreate["updatedAt"] = "";
  issueToCreate["updatedBy"] = "";

  // We also ignore any specified date and compute this ourselves
  issueToCreate["createdAt"] = "";

  json jsonFile = _fileHandler->read();

  Issue issue;
  try {
    issue = issueToCreate.get<Issue>();
  } catch (const std::exception& exp) {
    std::cout << exp.what() << std::endl;
    throw BadRequestError(
        std::string(
            "Unable to create an issue using the following information: " +
            body)
            .c_str());
  }

  issue.id = GenerateId();

  while (!Filter(jsonFile, {{"id", issue.id}}).empty()) {
    issue.id = GenerateId();
  }

  // Get the user from the User Service
  issue.createdBy = _userService->Get(std::string(issue.createdBy.id));

  // Set the created time to right now (in UTC time)
  issue.createdAt = TimeUtilities::CurrentTimeUTC();

  // Newly created issues have not been updated, so we set this to an
  // uninitialized User object, similar for others below
  issue.updatedBy = User{};

  // If we are provided assignedTo, set issue.assignedTo field to that
  if (!issue.assignedTo.id.empty()) {
    issue.assignedTo = _userService->Get(std::string(issue.assignedTo.id));
  }

  // If we are provided a status, set issue.status field to that
  if (issue.status.empty()) {
    issue.status = "New";
  }

  // If we are provided a reporter different than createdBy, set it
  // Else, reporter set to createdBy
  if (issue.reporter.id != issue.createdBy.id && !issue.reporter.id.empty()) {
    issue.reporter = _userService->Get(std::string(issue.reporter.id));
  } else {
    issue.reporter = issue.createdBy;
  }

  // Votes shouldn't exist for newly created issue.
  issue.votes = {};

  // If a description was provided, create a comment with it
  if (!description.empty()) {
    Comment temp;
    temp.body = description;
    temp.issueId = issue.id;
    temp.createdBy = issue.createdBy;
    temp.createdAt = issue.createdAt;
    temp = _commentService->Create(json(temp).dump());
    // Set the comments set to have the newly created comment
    issue.comments = {};
    issue.comments.insert(temp);
  }

  // Similarly, we set the updated time to the "null time"
  issue.updatedAt = TimeUtilities::NullTimeUTC();

  jsonFile.push_back(issue);
  _fileHandler->write(jsonFile);

  return issue;
}

Issue IssueService::Update(std::string body) {
  json updatedIssue;
  try {
    updatedIssue = json::parse(body);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    throw BadRequestError(
        std::string(
            "The Issue could not be created because of a parsing error. "
            "The invalid Issue information is: " +
            body)
            .c_str());
  }

  // Ignore the updatedAt date provided, calculate it ourself to ensure
  // consistency
  updatedIssue["updatedAt"] = "";

  Issue updated = updatedIssue.get<Issue>();
  json jsFile = _fileHandler->read();
  auto itIssue = std::find_if(
      jsFile.begin(), jsFile.end(),
      [&](const json& item) { return item["id"] == updatedIssue["id"]; });
  // If the issue we want to update exists
  if (itIssue != jsFile.end()) {
    // Get the creator, updater, assigned, and reporter users from the
    // UserService
    updated.createdBy = _userService->Get(std::string(updated.createdBy.id));
    updated.updatedBy = _userService->Get(std::string(updated.updatedBy.id));
    updated.assignedTo = _userService->Get(std::string(updated.assignedTo.id));
    updated.reporter = _userService->Get(std::string(updated.reporter.id));

    // Get the votes from our vote service
    updated.votes = _voteService->Get({{"issueId", updated.id}});
    // Get the comments from our comment service
    std::vector<Comment> comments =
        _commentService->Get({{"issueId", updated.id}});
    std::multiset<Comment> ms(comments.begin(), comments.end());
    updated.comments = ms;

    updated.updatedAt = TimeUtilities::CurrentTimeUTC();

    *itIssue = updated;

    _fileHandler->write(jsFile);
  } else {
    throw NotFoundError(
        std::string("The Issue could not be found with the following id: " +
                    updatedIssue["id"].get<std::string>())
            .c_str());
  }
  return updated;
}

bool IssueService::Delete(std::string id) {
  json jsonFile = _fileHandler->read();

  // Find the issue with the passed in id
  auto itissue =
      std::find_if(jsonFile.begin(), jsonFile.end(),
                   [&](const json& item) { return item["id"] == id; });

  // If the issue was found
  if (itissue != jsonFile.end()) {
    jsonFile.erase(itissue);
    _fileHandler->write(jsonFile);
    return true;
  } else {
    throw NotFoundError(
        std::string("The issue could not be found with the following id: " + id)
            .c_str());
  }
  return false;
}
