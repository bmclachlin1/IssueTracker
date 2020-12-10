#include "MainView.h"

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "ClientAppManager.h"
#include "User.h"
#include "UserInputValidators.hpp"
#include "Utilities.h"

using std::string;

bool MainView::WelcomeMessage() {
  // Create the request object
  auto request = RequestUtilities::CreateGetRequest(_serverUri, "/alive");

  // Start the client
  std::cout << HotTicket::AppInfo << std::endl;
  std::cout << "Connecting the to server at '" << _serverUri << "'...\n";

  // Make a synchronous GET request to the server
  auto response = restbed::Http::sync(request);
  if (response->get_status_code() == restbed::OK) {
    std::cout << "Connection successful!\n\n";
    return true;
  } else {
    std::cout << "Connection unsuccesful. Please verify that the server is "
                 "running at '"
              << _serverUri << "' to continue\n";
    return false;
  }
}

User MainView::Login() {
  string prompt = "Please enter your name to login: ";
  auto validator = Validators::StringLengthValidator(
      1, 256, "Name must be between 1 and 256 characters.");
  string input = PromptUser(prompt, validator);

  // Create request to get the user from the server
  auto request = RequestUtilities::CreateGetRequest(
      _serverUri, string("/users?name=" + restbed::Uri::encode(input)));

  // Send the request and get the response
  auto response = restbed::Http::sync(request);

  // Parse the reseponse
  json responseJson = ResponseUtilities::HandleResponse(response);

  // If we found the user, they are now "logged in"
  if (response->get_status_code() == restbed::OK && responseJson.size() > 0) {
    // Create the user object for the response
    User currentUser = responseJson[0];
    std::cout << "Welcome back, " << currentUser.name << std::endl;
    return currentUser;
  } else {
    // If not, we simply create the user for them
    StringMap userMap = {{"name", input}, {"role", "Developer"}};
    // Create the request to create the user
    request =
        RequestUtilities::CreatePostRequest(_serverUri, "/users", userMap);
    // Send the request and get the response
    response = restbed::Http::sync(request);
    // Parse the response
    responseJson = ResponseUtilities::HandleResponse(response);
    if (response->get_status_code() == restbed::CREATED) {
      // Create the user object for the response
      User currentUser = responseJson[0];
      std::cout << "Welcome, " << input << std::endl;
      return currentUser;
    } else {
      std::cout << "Something strange happened." << std::endl;
      return User{};
    }
  }
}

MainViewState MainView::MainMenu() {
  std::cout << std::endl;
  int selection;
  // Kinda werid, but hear me out
  std::vector<std::pair<MainViewState, string>> options = {
      std::make_pair(MainViewState::ManageIssues, "Manage Issues"),
      std::make_pair(MainViewState::ManageUsers, "Manage Users"),
      std::make_pair(MainViewState::Exit, "Exit")};

  std::stringstream prompt;
  prompt << "Main menu\n";
  for (int i = 0; i < options.size(); i++) {
    prompt << i + 1 << ": " << options[i].second << std::endl;
  }
  prompt << "Please select an option: ";

  selection = PromptUser(prompt, Validators::RangeValidator(1, options.size()));

  return static_cast<MainViewState>(options[selection - 1].first);
}
