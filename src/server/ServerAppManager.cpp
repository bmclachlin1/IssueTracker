#include "ServerAppManager.h"

#include <restbed>

#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include "CommentController.hpp"
#include "CommentService.h"
#include "IssueController.hpp"
#include "IssueService.h"
#include "Logger.hpp"
#include "UserController.hpp"
#include "UserService.h"
#include "Utilities.h"
#include "VoteController.hpp"
#include "VoteService.h"
#include "cxxopts.hpp"
#include "nlohmann/json.hpp"

using Session = std::shared_ptr<restbed::Session>;

bool ServerAppManager::Init(int argc, char* argv[]) {
  ServerConfig config;
  // Create the options parser
  cxxopts::Options options("hotTicket-server", HotTicket::AppInfo);

  // Add options for enabling logging and specifiying the port to run on
  // clang-format off
  options.add_options()
    ("d,debug", "Enable debug log messages",
                cxxopts::value<bool>()->default_value("false"))
    ("p,port", "Port to run the server on",
               cxxopts::value<int>()->default_value("8080"))
    ("h, help", "Print help text");
  // clang-format om

  // Parse the options
  auto result = options.parse(argc, argv);

  // If the user requested help, display the help and then exit
  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return false;
  }

  // Get the config values
  try {
    config.debug = result["debug"].as<bool>();
    config.port = result["port"].as<int>();
    this->_config = config;
    return true;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl << "Exiting..." << std::endl;
    return false;
  }
}

void ServerAppManager::Run() {
  // Create the services
  std::shared_ptr<UserService> userService = std::make_shared<UserService>();
  std::shared_ptr<VoteService> voteService =
      std::make_shared<VoteService>(userService);
  std::shared_ptr<CommentService> commentService =
      std::make_shared<CommentService>(userService);
  std::shared_ptr<IssueService> issueService =
      std::make_shared<IssueService>(userService, commentService, voteService);

  // Create the controllers
  UserController<restbed::Session> userController(userService);
  VoteController<restbed::Session> voteController(voteService);
  CommentController<restbed::Session> commentController(commentService);
  IssueController<restbed::Session> issueController(issueService);

  // Get the resources from the controllers
  auto usersResource = userController.resource;
  auto votesResource = voteController.resource;
  auto commentsResource = commentController.resource;
  auto issuesResource = issueController.resource;

  // Create a resource that the client will call to see if the server is alive
  std::shared_ptr<restbed::Resource> alive =
      std::make_shared<restbed::Resource>();
  alive->set_path("/alive");
  alive->set_method_handler("GET", [&](const Session& session) {
    session->close(restbed::OK, "", ResponseUtilities::BuildResponseHeaders());
  });

  // Create the service settings
  auto settings = std::make_shared<restbed::Settings>();
  std::string address = "127.0.0.1";
  // Set the address and port of the service
  settings->set_bind_address(address);
  settings->set_port(_config.port);

  // Create the service
  restbed::Service service;

  // Publish the resources
  service.publish(usersResource);
  service.publish(votesResource);
  service.publish(commentsResource);
  service.publish(issuesResource);
  service.publish(alive);

  // Create a logger, if the user requested it
  if (_config.debug) {
    service.set_logger(std::make_shared<CustomLogger>());
  } else {
  std::cout << "Server listening on http://" << address << ":" << _config.port
            << "..." << std::endl;
  }
  service.start(settings);
}
