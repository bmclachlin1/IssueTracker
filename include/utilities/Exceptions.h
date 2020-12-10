#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>

/**
 * @class NotFoundError
 * @brief Implements an exception for when: we cannot
 * find an Entity request (such as no matching id)
 */
class NotFoundError : public std::runtime_error {
 public:
  /**
   * @param errMessage An error message.
   */
  explicit NotFoundError(const char* errMessage)
      : std::runtime_error(errMessage) {}
};

/**
 * @class InternalServerError
 * @brief Implements an exception for errors when: saving/reading/parsing
 * the JSON file
 */
class InternalServerError : public std::runtime_error {
 public:
  /**
   * @param errMessage An error message.
   */
  explicit InternalServerError(const char* errMessage)
      : std::runtime_error(errMessage) {}
};

/**
 * @class BadRequestError
 * @brief Implements an exception for errors when: creating a bad Entity
 * based on the information provided
 */
class BadRequestError : public std::runtime_error {
 public:
  /**
   * @param errMessage An error message.
   */
  explicit BadRequestError(const char* errMessage)
      : std::runtime_error(errMessage) {}
};

/**
 * @class AlreadyExistsError
 * @brief Implements an exception for errors when: An entity
 * already exists based on the information provided
 */
class AlreadyExistsError : public std::runtime_error {
 public:
  /**
   * @param errMessage An error message.
   */
  explicit AlreadyExistsError(const char* errMessage)
      : std::runtime_error(errMessage) {}
};

/**
 * @class NotImplementedError
 * @brief Alerts the caller that the method they're calling is not implemented
 */
class NotImplementedError : public std::runtime_error {
 public:
  /**
   * @param errMessage An error message.
   */
  explicit NotImplementedError(const char* errMessage)
      : std::runtime_error(errMessage) {}
};
#endif
