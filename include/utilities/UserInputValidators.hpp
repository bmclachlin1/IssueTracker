#ifndef USER_INPUT_VALIDATORS_H
#define USER_INPUT_VALIDATORS_H

#include <functional>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

/**
 * @class Validator
 * @brief Error handling and input validation for commandline inputs
 * from a user
 * @tparam T the type of input to validate (i.e. int, string, double, or bool)
 */
template <class T>
class Validator {
  using Callback = std::function<bool(T)>;
  using CallbackPair = std::pair<Callback, std::string>;
  using CallbackPairs = std::vector<CallbackPair>;

 public:
  /**
   * Default constructor. Will always evaluate to true.
   */
  Validator() = default;
  /**
   * Copy constructor
   */
  Validator(const Validator& copy)
      : _callbacks(copy._callbacks.begin(), copy._callbacks.end()) {}

  /**
   * Constructor. Adds a validator callback and error message to the collection
   * @param callback the validator callback function. Should return true if
   * the input is valid, and false otherwise
   * @param message the message to display if the validator callback returns
   * false
   */
  Validator(const Callback& callback, const std::string& message) {
    _callbacks.push_back(std::make_pair(callback, message));
  }

  Validator(const std::initializer_list<Validator>& validators) {
    for (auto validator : validators) {
      _callbacks.insert(_callbacks.end(), validator._callbacks.begin(),
                        validator._callbacks.end());
    }
  }

  virtual ~Validator() {}

  /**
   * Add a single callback and message pair to the callbacks collection
   * @param callback the validation callback to add. Should return true if the
   * input is valid, and false otherwise.
   * @param message the message to display if the callback returns false
   */
  void Add(const Callback& callback, const std::string& message) {
    _callbacks.push_back(std::make_pair(callback, message));
  }

  /**
   * Add all callback-message pairs from a different Validator
   * @param validator the Validator to take the callback and messages from
   */
  void Add(const Validator& validator) {
    _callbacks.insert(_callbacks.end(), validator._callbacks.begin(),
                      validator._callbacks.end());
  }

  /**
   * Invokes all validation callbacks against a value
   * @param value the value to validate
   * @returns the true if \p value is valid (according to the #_validators),
   * false otherwise
   */
  bool operator()(const T& value) const {
    bool result = true;
    for (auto callback : _callbacks) {
      result &= callback.first(value);
    }
    return result;
  }

  /**
   * @param value the value to validate
   * @returns if the value is invalid, we return #_message . Else,
   * return an empty string
   */
  std::string Validate(const T& value) const {
    std::string result = "";
    for (auto callback : _callbacks) {
      if (!callback.first(value)) {
        result = callback.second;
        break;
      }
    }
    return result;
  }

  /**
   * Overloaded assignment operator
   */
  Validator& operator=(const Validator& rhs) {
    if (this == &rhs) {
      return *this;
    }
    _callbacks = CallbackPairs(rhs._callbacks.begin(), rhs._callbacks.end());
    return *this;
  }

 private:
  /**
   * Collection of pairs of validator callback functions and their associated
   * messages. Each callback should return true if the input is valid, and false
   * otherwise
   */
  CallbackPairs _callbacks;
};

/**
 * @namespace Validators
 * @brief Collection of functions to create common Validator objects
 */
namespace Validators {
/**
 * @param minimum the minimum value allowed to input
 * @param message the message to display of the input is too small. If not
 * specified, the default will be 'Selection must be greater than \p minimum'
 * @returns an int Validator that checks input against the minimum allowed value
 */
Validator<int> MinValidator(const int& minimum,
                            const std::string& message = "");

/**
 * @param maxiumum the maxiumum value allowed to input
 * @param message the message to display of the input is too large. If not
 * specified, the default will be 'Selection must be less than \p maximum'
 * @returns an int Validator that checks against the maxiumum allowed value
 */
Validator<int> MaxValidator(const int& maximum,
                            const std::string& message = "");

/**
 * @param minimum the minimum allowed value to input
 * @param maximum the maxiumum allowed value to input
 * @param messsage the message to display if either validator evaluates to
 * false. If not specified, the default will be 'Selection must be between \p
 * minimum and \p maxiumum'
 * @returns a vector of int Validator objects to validate input against a range
 */
Validator<int> RangeValidator(const int& minimum, const int& maximum,
                              const std::string& message = "");

/**
 * @returns an int Validator for the id property of Entity models within the
 * project
 */
Validator<std::string> IdValidator();

/**
 * @param message message to display if the input is empty. If not specified,
 * the default will be 'Input cannot be empty"
 * @returns a string Validator the check if a user's input is empty
 */
Validator<std::string> StringNotEmptyValidator(const std::string& message = "");

/**
 * @param minimumLength the minimum length the input string can be
 * @param maxiumumLength the maximum length the input string can be
 * @param message the message to display if the input string is outside the
 * specified length. If not specified, the default will be 'Input must be
 * between \p minimumLength and \p maxiumumLength characters'
 * @returns a string Validator to validate the length of a string
 * input
 */
Validator<std::string> StringLengthValidator(const uint32_t& minimumLength,
                                             const uint32_t& maximumLength,
                                             const std::string& message = "");
}  // namespace Validators

#endif  // USER_INPUT_VALIDATORS_H
