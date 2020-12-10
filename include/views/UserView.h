#ifndef USERVIEW_H
#define USERVIEW_H

#include <string>

#include "BaseView.h"

enum class UserViewState { CreateUser = 1, SelectUser, GoBack };

enum class ManageUserState { Update = 1, Delete, GoBack };

/**
 * @class UserView
 * @brief Handles user input relating to users
 */
class UserView : public BaseView {
 public:
  UserView() {}
  virtual ~UserView() {}

  /**
   * Displays a menu of choices for the user to pick from
   * @returns the integer representing their choice
   */
  UserViewState MainMenu();

  /**
   * Prompts the user for information to create a new user
   * @return the user's inputted values
   */
  void CreateUserView();

  /**
   * Prompts the user for information to update a user
   * @return the user's input
   */
  void UpdateUserView(const User& user);

  /**
   * Prompts the user for the information required to delete a User from the
   * system
   * @return std::string the user's input
   */
  void DeleteUserView(const User& user);

  /**
   * Displays a list of users to select from
   * @returns the user selected
   */
  User SelectUserView();

  /**
   * Displays a menu for the user to decide to Update or Delete a user
   */
  ManageUserState ManageUserMenu();

  inline void RunUserView(UserViewState choice) {
    bool quit = false;
    while (!quit) {
      switch (choice) {
        case UserViewState::CreateUser:
          CreateUserView();
          break;
        case UserViewState::SelectUser: {
          User selectedUser = SelectUserView();
          // If they decided to cancel, go back
          if (selectedUser.id.empty()) break;
          ManageUserState nextMenu = ManageUserMenu();
          switch (nextMenu) {
            default:
              break;
            case ManageUserState::Update:
              UpdateUserView(selectedUser);
              break;
            case ManageUserState::Delete:
              DeleteUserView(selectedUser);
              break;
          }
        } break;
        default:
          quit = true;
          break;
      }
      if (!quit) choice = MainMenu();
    }
  }

  /**
   * Displays the user information in a nice format
   */
  static void DisplayUser(const User& user);

  /**
   * @param prompt the prompt shown to the user
   * @param serverUri the uri to the server
   * @returns the userId based on the provided User's name
   */
  static std::string GetUserIdFromName(const std::string& prompt);
};

#endif  // USERVIEW_H
