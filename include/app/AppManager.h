#ifndef APP_MANAGER_H
#define APP_MANAGER_H

/**
 * @class AppManager
 * @brief interface for a class to manage an application
 */
class AppManager {
 public:
  AppManager() = default;
  virtual ~AppManager() = default;

  /**
   * Initializes the application
   * @returns whether the initialization was successful
   */
  virtual bool Init(int argc, char* argv[]) = 0;

  /**
   * Runs the application
   */
  virtual void Run() = 0;
};

#endif  // APP_MANAGER_H
