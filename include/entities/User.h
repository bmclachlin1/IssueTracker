#ifndef USER_H
#define USER_H

#include <string>
#include "Entity.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * @class User
 * @brief Model class for a User
 */
class User : public Entity {
 public:
  /**
   * Constructor for a User object. Sets the role to the default value of
   * "Developer"
   * @return an instantiation of a User object
   */
  User() { role = "Developer"; }

  /**
   * Destructor for a User object
   */
  virtual ~User() {}

  /**
   * Name of user
   */
  std::string name = "";

  /**
   * Role of user
   */
  std::string role = "";
};

/**
 * Serializes a User object as a nlohmann::json object, as defined by the
 * [nlohmann::json
 * documentation](https://github.com/nlohmann/json/blob/develop/README.md#arbitrary-types-conversions)
 * @param j the nlohmann::json object the output the results to
 * @param user the User to be serialized
 */
inline void to_json(json& j, const User& user) {
  j = json{{"name", user.name}, {"id", user.id}, {"role", user.role}};
}

/**
 * De-serializes a user object to a nlohnmann::json object, as defined by the
 * [nlohmann::json
 * documentation](https://github.com/nlohmann/json/blob/develop/README.md#arbitrary-types-conversions)
 * @param j the nlohman::json out the User is being created from
 * @param user the User the result will be stored in
 */
inline void from_json(const json& j, User& user) {
  user.name = j.value("name", user.name);
  user.id = j.value("id", user.id);
  user.role = j.value("role", user.role);
}

#endif  // USER_H
