#include "ClientAppManager.h"

#include <restbed>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "ClientAppManager.h"
#include "CommentView.h"
#include "IssueView.h"
#include "MainView.h"
#include "User.h"
#include "UserView.h"
#include "Utilities.h"
#include "VoteView.h"
#include "cxxopts.hpp"
#include "nlohmann/json.hpp"

// Declare the serverUri and user variables so they can be used statically
// Kinda smelly, but 🤷🏻‍♂️
std::string ClientAppManager::_serverUri = "";
User ClientAppManager::_user = User{};

bool ClientAppManager::Init(int argc, char* argv[]) {
  std::string serverUri;

  // Create the options parser
  cxxopts::Options options("hotTicket", HotTicket::AppInfo);

  // Add options for specify server connection parameters
  // clang-format off
  options.add_options()
    ("H,host", "Host IP address for the server",
               cxxopts::value<std::string>()->default_value("localhost"))
    ("p,port", "Host Port for the server",
              cxxopts::value<int>()->default_value("8080"))
    ("u,url", "Full URL to the server. If specified, --host and --port are "
              "ignored",
              cxxopts::value<std::string>())
    ("h, help", "Print help text");
  // clang-format on

  // Parse the options
  auto result = options.parse(argc, argv);

  // If the user requested help, display the help and then exit
  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return false;
  }

  // Get the config values from the parsed options
  try {
    if (result.count("url")) {
      serverUri = result["url"].as<std::string>();
    } else {
      auto host = result["host"].as<std::string>();
      auto port = result["port"].as<int>();

      serverUri.append("http://");
      serverUri.append(host);
      serverUri.append(":");
      serverUri.append(std::to_string(port));
    }
    ClientAppManager::_serverUri = serverUri;

    // If the input values were correct, then the server uri shouldn't be empty
    // We return the result that assertion to verify that the client
    // initialization worked correctly
    return !ClientAppManager::_serverUri.empty();
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl << "Exiting..." << std::endl;
    return false;
  }
}

void ClientAppManager::Run() {
  MainView mainView;
  UserView userView;
  VoteView voteView;
  CommentView commentView;
  IssueView issueView;

  // If we couldn't connect to the server, we exit
  if (!mainView.WelcomeMessage()) {
    exit(EXIT_FAILURE);
  }

  auto user = mainView.Login();
  // If the current user doesn't exist, exit
  if (user.id.empty()) {
    exit(EXIT_FAILURE);
  }
  ClientAppManager::SetUser(user);

  MainViewState choice = mainView.MainMenu();
  bool quit = false;

  while (!quit) {
    switch (choice) {
      case MainViewState::ManageIssues: {
        IssueViewState issueViewChoice = issueView.MainMenu();
        issueView.RunIssueView(issueViewChoice);
      } break;

      case MainViewState::ManageUsers: {
        UserViewState userViewChoice = userView.MainMenu();
        userView.RunUserView(userViewChoice);
      } break;
      default:
        quit = true;
        break;
    }
    if (!quit) choice = mainView.MainMenu();
  }
  std::cout << "Goodbye!\n";
}

User ClientAppManager::CurrentUser() { return ClientAppManager::_user; }

void ClientAppManager::SetUser(const User& user) {
  ClientAppManager::_user = user;
}

std::string ClientAppManager::ServerURI() {
  return ClientAppManager::_serverUri;
}
