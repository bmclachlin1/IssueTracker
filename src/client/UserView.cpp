#include "UserView.h"

#include <restbed>

#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "ClientAppManager.h"
#include "User.h"
#include "Utilities.h"
#include "nlohmann/json.hpp"

using nlohmann::json;

using std::string;

std::vector<string> roles = {"Developer", "Tester", "Manager"};

UserViewState UserView::MainMenu() {
  std::cout << std::endl;
  int selection;
  std::vector<std::pair<UserViewState, string>> options = {
      std::make_pair(UserViewState::CreateUser, "Create a new user"),
      std::make_pair(UserViewState::SelectUser, "Manage an existing user"),
      std::make_pair(UserViewState::GoBack, "Back to main menu")};

  std::stringstream prompt;
  prompt << "Users menu\n";
  for (int i = 0; i < options.size(); i++) {
    prompt << i + 1 << ": " << options[i].second << std::endl;
  }
  prompt << "What would you like to do: ";

  selection = PromptUser(
      prompt.str(), Validators::RangeValidator(1, options.size(),
                                               "Please select a valid option"));

  return static_cast<UserViewState>(options[selection - 1].first);
}

void UserView::CreateUserView() {
  std::cout << std::endl << "Create a new user" << std::endl;

  string namePrompt = "Name of the user: ";

  string name = PromptUser(
      namePrompt, Validators::StringLengthValidator(
                      1, 256, "Name must be between 1 and 256 characters"));

  std::stringstream rolePrompt;
  rolePrompt << "Select a role\n";
  for (int i = 1; i <= roles.size(); i++) {
    rolePrompt << i << ": " << roles[i - 1] << std::endl;
  }
  rolePrompt << "Please select a role: ";
  int selection =
      PromptUser(rolePrompt, Validators::RangeValidator(1, roles.size()));
  string role = roles[selection - 1];

  StringMap user = {{"name", name}, {"role", role}};

  auto request =
      RequestUtilities::CreatePostRequest(_serverUri, "/users", user);
  auto response = restbed::Http::sync(request);

  std::cout << std::endl;
  if (response->get_status_code() == restbed::CREATED) {
    std::cout << "New user successfully created!" << std::endl;
  } else {
    json jsonResponse = ResponseUtilities::HandleResponse(response);
    std::cout << "Something went wrong with your request: "
              << jsonResponse.dump() << std::endl;
  }
}

User UserView::SelectUserView() {
  std::cout << std::endl;
  auto request = RequestUtilities::CreateGetRequest(_serverUri, "/users");
  auto response = restbed::Http::sync(request);
  json usersJson = ResponseUtilities::HandleResponse(response);
  int selection;
  std::cout << std::endl;
  if (usersJson.empty()) {
    std::cout << "No users to select\n\n";
  } else {
    std::vector<User> users = usersJson;

    std::stringstream prompt;
    prompt << "All available users: " << std::endl;
    // Display the list of users
    for (int i = 1; i <= users.size(); i++) {
      prompt << i << ": Name: " << users[i - 1].name
             << ", Role: " << users[i - 1].role << std::endl;
    }
    prompt << users.size() + 1 << ": Cancel\n";
    prompt << "Please select a user to manage: ";

    selection =
        PromptUser(prompt, Validators::RangeValidator(1, users.size() + 1));

    // If they select a user, return it
    if (selection >= 1 && selection <= users.size()) {
      return users[selection - 1];
    }
  }

  // Else, return a "null user"
  return User{};
}

ManageUserState UserView::ManageUserMenu() {
  std::cout << std::endl;
  std::stringstream prompt;
  prompt << "Manage User\n"
         << "1: Update User\n"
         << "2: Delete User\n"
         << "3: Go back\n"
         << "Please select and option: ";
  int selection = PromptUser(prompt, Validators::RangeValidator(1, 3));
  return static_cast<ManageUserState>(selection);
}

void UserView::UpdateUserView(const User& user) {
  std::cout << std::endl;

  std::cout << "Requested user: \n";
  UserView::DisplayUser(user);

  string namePrompt = "Name (default: " + user.name + "): ";
  string newName =
      PromptUser(namePrompt, Validators::StringLengthValidator(0, 256));

  std::stringstream rolePrompt;
  rolePrompt << "Select a role\n";
  for (int i = 1; i <= roles.size(); i++) {
    rolePrompt << i << ": " << roles[i - 1] << std::endl;
  }
  rolePrompt << "Please select a role: ";
  int selection =
      PromptUser(rolePrompt, Validators::RangeValidator(1, roles.size()));
  string role = roles[selection - 1];

  StringMap body = {{"id", user.id},
                    {"name", newName.empty() ? user.name : newName},
                    {"role", role}};

  auto request =
      RequestUtilities::CreatePutRequest(_serverUri, "/users/" + user.id, body);

  auto response = restbed::Http::sync(request);

  if (response->get_status_code() == restbed::OK) {
    std::cout << "\nUser updated successfully!\n";
  } else {
    json jsonResponse = ResponseUtilities::HandleResponse(response);
    std::cout << "\nAn error occurred with your request:\n"
              << jsonResponse.dump();
  }
}

void UserView::DeleteUserView(const User& user) {
  std::cout << std::endl;
  auto request =
      RequestUtilities::CreateDeleteRequest(_serverUri, "/users/" + user.id);
  auto response = restbed::Http::sync(request);

  json jsonResponse = ResponseUtilities::HandleResponse(response);
  if (response->get_status_code() == restbed::OK) {
    std::cout << "User successfully deleted!\n";
  } else {
    std::cout << "\nAn error occurred with your request:\n"
              << jsonResponse.dump() << std::endl;
  }
}

void UserView::DisplayUser(const User& user) {
  std::cout << "Name: " << user.name << ", Role: " << user.role << std::endl;
}

std::string UserView::GetUserIdFromName(const std::string& prompt) {
  // Validate that the User we want to assign the issue to exists
  string userName;
  userName = PromptUser(prompt, Validators::StringLengthValidator(1, 256));

  const StringMap queryUserParam = {{"name", userName}};

  // get the User where the name is equal to the inputted name
  auto userRequest = RequestUtilities::CreateGetRequest(
      ClientAppManager::ServerURI(), "/users");
  userRequest->set_query_parameters(queryUserParam);

  auto userResponse = restbed::Http::sync(userRequest);

  // If we couldn't find the user, return nothing
  if (userResponse->get_status_code() == restbed::NOT_FOUND) {
    return "";
  }

  json matchedUsers = ResponseUtilities::HandleResponse(userResponse);

  string userId;
  if (!matchedUsers.empty()) {
    userId = matchedUsers[0]["id"];
  }

  return userId;
}
