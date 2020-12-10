#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "Comment.h"
#include "CommentService.h"
#include "FileHandler.h"
#include "User.h"
#include "UserService.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

std::vector<Comment> CommentService::Get(
    const std::multimap<std::string, std::string> queryParams) {
  std::vector<Comment> comments;
  json jsonFile = _fileHandler->read();

  json filtered = Filter(jsonFile, queryParams);

  for (auto& comment : filtered) {
    Comment temp = comment.get<Comment>();

    User createdBy = _userService->Get(std::string(temp.createdBy.id));
    temp.createdBy = createdBy;

    if (!temp.updatedBy.id.empty()) {
      temp.updatedBy = _userService->Get(std::string(temp.updatedBy.id));
    }
    comments.push_back(temp);
  }

  return comments;
}

Comment CommentService::Get(std::string id) {
  json jsonFile = _fileHandler->read();

  // Get the comment by the id
  json filtered = Filter(jsonFile, {{"id", id}});

  // If the user was not found
  if (filtered.empty()) {
    throw NotFoundError(
        std::string("A comment could not be found with the following id: " + id)
            .c_str());
  }
  Comment comment;
  comment = filtered[0].get<Comment>();
  comment.createdBy = _userService->Get(std::string(comment.createdBy.id));
  if (!comment.updatedBy.id.empty()) {
    comment.updatedBy = _userService->Get(std::string(comment.updatedBy.id));
  }
  return comment;
}

Comment CommentService::Create(std::string body) {
  json commentToCreate;
  try {
    commentToCreate = json::parse(body);
  } catch (std::exception&) {
    throw BadRequestError(
        std::string(
            "An error occurred parsing the following comment information: " +
            body)
            .c_str());
  }

  // We explicitly ignore these values when creating comments
  commentToCreate["updatedAt"] = "";
  commentToCreate["updatedBy"] = "";

  // We also ignore any specified date and compute this ourselves
  commentToCreate["createdAt"] = "";

  json jsonFile = _fileHandler->read();

  Comment comment;
  try {
    // Attempt to deserialize the json body.
    comment = commentToCreate.get<Comment>();
  } catch (std::exception& e) {
    throw BadRequestError(
        std::string(
            "Unable to create a comment using the following information: " +
            body)
            .c_str());
  }
  // Ignore any id value provided
  comment.id = GenerateId();
  // If the file has a comment with that id, generate a new one
  while (!Filter(jsonFile, {{"id", comment.id}}).empty()) {
    comment.id = GenerateId();
  }

  // Get the user from the User Service
  comment.createdBy = _userService->Get(std::string(comment.createdBy.id));

  // Set the created time to right now (in UTC time)
  comment.createdAt = TimeUtilities::CurrentTimeUTC();

  // Newly created comments have not been updated, so we set this to an
  // uninitialized User object
  comment.updatedBy = User{};
  // Similarly, we set the updated time to the "null time"
  comment.updatedAt = TimeUtilities::NullTimeUTC();

  jsonFile.push_back(comment);
  _fileHandler->write(jsonFile);

  return comment;
}

Comment CommentService::Update(std::string body) {
  json updatedComment;
  try {
    updatedComment = json::parse(body);
  }

  catch (std::exception& e) {
    throw BadRequestError(
        std::string(
            "The Comment could not be created because of a parsing error. "
            "The invalid Comment information is: " +
            body)
            .c_str());
  }

  // Ignore the updatedAt date provided, calculate it ourself to ensure
  // consistency
  updatedComment["updatedAt"] = "";

  Comment updated = updatedComment.get<Comment>();
  json jsFile = _fileHandler->read();
  auto itComment = std::find_if(
      jsFile.begin(), jsFile.end(),
      [&](const json& item) { return item["id"] == updatedComment["id"]; });
  // If the Comment we want to update exists
  if (itComment != jsFile.end()) {
    // Get the creator and updater users from the UserService
    updated.createdBy = _userService->Get(std::string(updated.createdBy.id));
    updated.updatedBy = _userService->Get(std::string(updated.updatedBy.id));

    updated.updatedAt = TimeUtilities::CurrentTimeUTC();

    *itComment = updated;

    _fileHandler->write(jsFile);
  } else {
    throw NotFoundError(
        std::string("The User could not be found with the following id: " +
                    updatedComment["id"].get<std::string>())
            .c_str());
  }
  return updated;
}

bool CommentService::Delete(std::string id) {
  json jsonFile = _fileHandler->read();

  // Find the comment with the passed in id
  auto itcomment =
      std::find_if(jsonFile.begin(), jsonFile.end(),
                   [&](const json& item) { return item["id"] == id; });

  // If the comment was found
  if (itcomment != jsonFile.end()) {
    jsonFile.erase(itcomment);
    _fileHandler->write(jsonFile);
    return true;
  } else {
    throw NotFoundError(
        std::string("The comment could not be found with the following id: " +
                    id)
            .c_str());
  }
  return false;
}
