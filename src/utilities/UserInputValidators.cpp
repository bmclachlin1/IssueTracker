#include "UserInputValidators.hpp"

#include <iostream>
#include <string>

using string = std::string;

namespace Validators {
Validator<int> MinValidator(const int& minimum, const string& message) {
  string outputMessage = !message.empty() ? message
                                          : "Selection must be less than " +
                                                std::to_string(minimum);
  return Validator<int>(
      [minimum](int selection) { return selection >= minimum; }, outputMessage);
}

Validator<int> MaxValidator(const int& maximum, const string& message) {
  string outputMessage = !message.empty() ? message
                                          : "Selection must be less than " +
                                                std::to_string(maximum);
  return Validator<int>(
      [maximum](int selection) { return selection <= maximum; }, outputMessage);
}

Validator<int> RangeValidator(const int& minimum, const int& maximum,
                              const string& message) {
  string outputMessage = !message.empty()
                             ? message
                             : "Selection must be between " +
                                   std::to_string(minimum) + " and " +
                                   std::to_string(maximum);
  return {MinValidator(minimum, outputMessage),
          MaxValidator(maximum, outputMessage)};
}

Validator<string> IdValidator() {
  auto validator =
      Validator<string>([](string input) { return input.size() == 10; },
                        "Id must be 10 characters long");

  // Add validator to check that Ids only contain lowercase alphanumeric
  // characters
  string alphanumeric = "abcdefghijklmnopqrstuvwxyz1234567890";
  validator.Add(Validator<string>(
      [=](string input) {
        return input.find_first_not_of(alphanumeric) == string::npos;
      },
      "Id must only contain lowercase alphanumeric characters"));
  return validator;
}

Validator<string> StringNotEmptyValidator(const string& message) {
  string outputMessage = !message.empty() ? message : "Input cannout be empty";
  return Validator<string>([](string input) { return !input.empty(); },
                           outputMessage);
}

Validator<string> StringLengthValidator(const uint32_t& minimumLength,
                                        const uint32_t& maximumLength,
                                        const string& message) {
  string outputMessage =
      !message.empty()
          ? message
          : "Input must be between " + std::to_string(minimumLength) + " and " +
                std::to_string(maximumLength) + " characters";

  Validator<string> validator = Validator<string>(
      [minimumLength, maximumLength](string input) {
        return input.size() >= minimumLength && input.size() <= maximumLength;
      },
      outputMessage);

  return validator;
}
}  // namespace Validators
