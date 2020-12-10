#ifndef MAIN_VIEW_H
#define MAIN_VIEW_H

#include <string>

#include "BaseView.h"
#include "User.h"

enum class MainViewState {
  MainMenu = 1,
  Login,
  ManageIssues,
  ManageUsers,
  Exit
};

/**
 * @class MainView
 * @brief Main Menu for our Issue Tracking Service
 */
class MainView : public BaseView {
 public:
  MainView() {}
  virtual ~MainView() {}

  bool WelcomeMessage();

  User Login();

  MainViewState MainMenu();
};

#endif  // MAIN_VIEW_H
