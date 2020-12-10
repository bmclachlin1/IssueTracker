#include "BaseView.h"

#include <exception>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include "Utilities.h"

int BaseView::PromptUser(const std::string& prompt,
                         const Validator<int>& validator) {
  int selection;
  std::string errorMessage = "";

  do {
    std::cout << prompt;
    std::string temp;
    std::getline(std::cin, temp, '\n');
    try {
      selection = std::stoi(temp);
      errorMessage = validator.Validate(selection);
    } catch (const std::invalid_argument&) {
      errorMessage = "Invalid input, please try again";
    } catch (const std::exception& e) {
      errorMessage = "An error occurred: " + std::string(e.what());
    }
    if (!errorMessage.empty())
      std::cout << std::endl << errorMessage << std::endl << std::endl;
    std::cin.clear();
  } while (!errorMessage.empty());

  return selection;
}

int BaseView::PromptUser(const std::stringstream& prompt,
                         const Validator<int>& validator) {
  return PromptUser(prompt.str(), validator);
}

std::string BaseView::PromptUser(const std::string& prompt,
                                 const Validator<std::string>& validator) {
  std::string input;
  std::string errorMessage = "";
  bool error = false;

  do {
    std::cout << prompt;
    std::getline(std::cin, input, '\n');
    errorMessage = validator.Validate(input);
    if (!errorMessage.empty())
      std::cout << std::endl << errorMessage << std::endl;
  } while (!errorMessage.empty());

  return input;
}

std::string BaseView::PromptUser(const std::stringstream& prompt,
                                 const Validator<std::string>& validator) {
  return PromptUser(prompt.str(), validator);
}
