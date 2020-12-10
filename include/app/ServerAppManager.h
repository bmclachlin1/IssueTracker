#ifndef SERVER_APP_MANAGER_H
#define SERVER_APP_MANAGER_H

#include "AppManager.h"

/**
 * Server configuration type
 */
struct ServerConfig {
  bool debug = false;  // No logging enabled by default
  int port = 8080;     // Default port
};

/**
 * @class ServerAppManager
 * @brief Handles initializing and running the service
 */
class ServerAppManager : public AppManager {
 public:
  ServerAppManager() = default;
  virtual ~ServerAppManager() = default;

  /**
   * Parses the command line arguments provided and initializes the server
   * configuration
   * @returns whether or not the initialization was successful
   */
  bool Init(int argc, char* argv[]);

  /**
   * Launches the service to listen for requests.
   */
  void Run();

 private:
  /**
   * The configuration options for the server app
   */
  ServerConfig _config;
};

#endif  // SERVER_APP_MANAGER_H
