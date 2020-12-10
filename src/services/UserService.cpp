#include "UserService.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Exceptions.h"
#include "User.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

std::vector<User> UserService::Get(
    const std::multimap<std::string, std::string> queryParams) {
  std::vector<User> users;
  json jsFile = _fileHandler->read();

  json filteredUsers = Filter(jsFile, queryParams);

  // For each User in the filtered results
  for (auto& person : filteredUsers) {
    User temp = person.get<User>();
    users.push_back(temp);
  }

  return users;
}

User UserService::Get(const std::string id) {
  User user;
  json jsFile = _fileHandler->read();

  // Get the user by ID - as a string
  json filtered = Filter(jsFile, {{"id", id}});

  // If the user was not found
  if (filtered.empty()) {
    throw NotFoundError(
        std::string("A User could not be found with the following id: " + id)
            .c_str());
  }
  user = filtered[0].get<User>();
  return user;
}

User UserService::Create(std::string body) {
  json userToCreate;
  try {
    userToCreate = json::parse(body);
  }

  catch (std::exception& e) {
    throw BadRequestError(
        std::string("The User could not be created because of a parsing error. "
                    "The invalid User information is: " +
                    body)
            .c_str());
  }
  json jsFile = _fileHandler->read();

  User temp = userToCreate.get<User>();
  // Ignore any value for id passed in from the request body
  temp.id = this->GenerateId();
  // If the file does not already have a user with that id
  while (!Filter(jsFile, {{"id", temp.id}}).empty()) {
    temp.id = this->GenerateId();
  }
  // Check to make sure there isn't a User with the same name
  if (Filter(jsFile, {{"name", temp.name}}).empty()) {
    jsFile.push_back(temp);
    _fileHandler->write(jsFile);
  } else {
    throw AlreadyExistsError(
        std::string("The User already exists with the following name: " +
                    userToCreate["name"].get<std::string>())
            .c_str());
  }
  return temp;
}

User UserService::Update(std::string body) {
  json updatedUser;
  try {
    updatedUser = json::parse(body);
  }

  catch (std::exception& e) {
    throw BadRequestError(
        std::string("The User could not be created because of a parsing error. "
                    "The invalid User information is: " +
                    body)
            .c_str());
  }

  User temp = updatedUser.get<User>();
  json jsFile = _fileHandler->read();
  auto itUser = std::find_if(
      jsFile.begin(), jsFile.end(),
      [&](const json& item) { return item["id"] == updatedUser["id"]; });
  // If the user we want to update exists
  if (itUser != jsFile.end()) {
    // Check if they are updating their name to something that already exists
    json filtered = Filter(jsFile, {{"name", temp.name}});
    if (!filtered.empty() && filtered[0]["id"] != (*itUser)["id"]) {
      throw AlreadyExistsError(
          std::string("The User already exists with the following name: " +
                      updatedUser["name"].get<std::string>())
              .c_str());
    }
    *itUser = temp;
    _fileHandler->write(jsFile);
  } else {
    throw NotFoundError(
        std::string("The User could not be found with the following id: " +
                    updatedUser["id"].get<std::string>())
            .c_str());
  }
  return temp;
}

// body is just the id
bool UserService::Delete(std::string id) {
  json jsFile = _fileHandler->read();

  // Find the User with the passed in id
  auto itUser =
      std::find_if(jsFile.begin(), jsFile.end(),
                   [&](const json& item) { return item["id"] == id; });

  // If the user was found
  if (itUser != jsFile.end()) {
    jsFile.erase(itUser);
    _fileHandler->write(jsFile);
    return true;
  } else {
    throw NotFoundError(
        std::string("The User could not be found with the following id: " + id)
            .c_str());
  }
  return false;
}
