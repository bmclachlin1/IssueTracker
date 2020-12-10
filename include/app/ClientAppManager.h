#ifndef CLIENT_APP_MANAGER_H
#define CLIENT_APP_MANAGER_H

#include <memory>
#include <string>

#include "AppManager.h"
#include "User.h"

/**
 * @class ClientAppManager
 * @brief Handles initializing the views and running the client app
 */
class ClientAppManager {
 public:
  ClientAppManager() {}
  virtual ~ClientAppManager() {}

  /**
   * Parses the command line arguments provided and configures the client app
   * @returns whether or not the configuation was successful
   */
  static bool Init(int argc, char* argv[]);

  /**
   * Runs the client app
   */
  static void Run();

  static User CurrentUser();

  static void SetUser(const User& user);

  static std::string ServerURI();

 private:
  /**
   * The configuration properties for the client app
   */
  static std::string _serverUri;

  /**
   * The currently logged in user
   */
  static User _user;
};

#endif  // CLIENT_APP_MANAGER_H
