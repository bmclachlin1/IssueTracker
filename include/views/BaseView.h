#ifndef BASE_VIEW_H
#define BASE_VIEW_H

#include <exception>
#include <functional>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "ClientAppManager.h"
#include "UserInputValidators.hpp"

/**
 * @class BaseView
 * @brief A base class containing helper methods and members for other view
 * instances
 */
class BaseView {
 public:
  BaseView() {}
  virtual ~BaseView() {}

 protected:
  /**
   * Method for displaying a message to the user and waiting for an integer
   * response response. The method will continue to prompt the user until
   * their input is valid
   * @param prompt The message directing the user what to do
   * @param validator A function defining if the input is valid. Should
   * return true if the input is valid
   * @param min the minimum value the user can enter
   * @param max the maximum value the user can enter
   * @param errorMessage The message to display if an error occurs. Default
   * is the BaseView::genericErrorMessage
   * @return the integer input of the player
   */
  static int PromptUser(const std::string& prompt,
                        const Validator<int>& validator);

  static int PromptUser(const std::stringstream& prompt,
                        const Validator<int>& validators);

  /**
   * Method for displaying a message to the user and waiting for an string
   * response response. The method will continue to prompt the user until their
   * input is valid
   * @param prompt The message directing the user what to do
   * @param validator A function defining if the input is valid. Should return
   * true if the input is valid
   * @param maxCharacters the maximum number of characters the response can be
   * @param errorMessage The message to display if an error occurs. Default is
   * the BaseView::genericErrorMessage
   * @return the integer input of the player
   */
  static std::string PromptUser(const std::string& prompt,
                                const Validator<std::string>& validator);

  static std::string PromptUser(const std::stringstream& prompt,
                                const Validator<std::string>& validator);

  /**
   * A generic error message to be used throughout views
   */
  static constexpr const char* genericErrorMessage =
      "Invalid entry. Please try again.";

 protected:
  const std::string _serverUri = ClientAppManager::ServerURI();
};

#endif  // BASE_VIEW_H
