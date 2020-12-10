#ifndef SERVER_ERROR_RESPONSE_H
#define SERVER_ERROR_RESPONSE_H

#include <string>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * @class ServerErrorResponse
 * @brief Data structure for a server error response
 */
class ServerErrorResponse {
 public:
  /**
   * Default constructor
   */
  ServerErrorResponse() = default;
  /**
   * Constructor
   * @param _title the title of the error (i.e. User Not Found)
   * @param _detail a detailed description of the error (i.e. No user could be
   * found with the id '8008135')
   * @param statusCode the HTTP status code of the error
   */
  ServerErrorResponse(const std::string& _title, const std::string& _detail,
                      const int& _statusCode)
      : title(_title), detail(_detail), statusCode(_statusCode) {}
  virtual ~ServerErrorResponse() {}

  /**
   * The title of the error (i.e. User Not Found)
   */
  std::string title;

  /**
   * A detailed description of the error (i.e. No user could be found with the
   * id '8008135')
   */
  std::string detail;

  /**
   * The HTTP status code of the error
   */
  int statusCode;
};

/**
 * Serialize a ServerErrorResponse to a nlohmann::json object
 */
inline void to_json(json& j, const ServerErrorResponse& error) {
  j = {{"title", error.title},
       {"detail", error.detail},
       {"statusCode", error.statusCode}};
}

/**
 * Deserialize a nlohman::json object to a ServerErrorResponse
 */
inline void from_json(const json& j, ServerErrorResponse& error) {
  error.title = j.at("title").get<std::string>();
  error.detail = j.at("detail").get<std::string>();
  error.statusCode = j.at("statusCode").get<int>();
}

#endif  // SERVER_ERROR_RESPONSE_H
