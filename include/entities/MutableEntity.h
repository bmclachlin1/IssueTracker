#ifndef MUTABLEENTITY_H
#define MUTABLEENTITY_H

#include <ctime>
#include <iostream>
#include <string>

#include "Entity.h"
#include "User.h"
#include "Utilities.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * @class MutableEntity
 * @brief contains common fields for entities that can be updated
 */
class MutableEntity : public Entity {
 public:
  /**
   * Constructor. Creates an entity with the current time in UTC
   */
  MutableEntity(time_t time = time(0)) : createdAt(*gmtime(&time)) {}

  /**
   * Destructor
   */
  virtual ~MutableEntity() {}

  /**
   * Time the Entity was created
   */
  struct tm createdAt;

  /**
   * The User who created the Entity
   */
  User createdBy;

  /**
   * Time the Entity was last updated at. Default value is the "null time"
   * (Sunday January 1 1900 00:00:00)
   */
  struct tm updatedAt = TimeUtilities::NullTimeUTC();

  /**
   * The User who last updated the Entity
   */
  User updatedBy;
};

/**
 * Serializes a MutableEntity to a nlohmann::json object
 */
inline void to_json(json& j, const MutableEntity& entity) {
  j = json{{"id", entity.id},
           {"createdAt", TimeUtilities::ConvertTimeToString(entity.createdAt)},
           {"createdBy", entity.createdBy.id}};
  // We only include the updated information if both fields are available
  if (!entity.updatedBy.id.empty()) {
    j["updatedAt"] = TimeUtilities::ConvertTimeToString(entity.updatedAt);
    j["updatedBy"] = entity.updatedBy.id;
  } else {
    j["updatedAt"] = "";
    j["updatedBy"] = "";
  }
}

/**
 * Deserializes a MutableEntity from a nlohmann::json object
 */
inline void from_json(const json& j, MutableEntity& entity) {
  User createdBy;
  User updatedBy;
  entity.id = j.value("id", entity.id);
  entity.createdBy = createdBy;
  // We can really only append the User id
  entity.createdBy.id = j.value("createdBy", "");
  auto createdAt = j.value("createdAt", "");
  if (createdAt != "") {
    entity.createdAt = TimeUtilities::ConvertStringToTime(createdAt);
  }
  entity.updatedBy = updatedBy;
  entity.updatedBy.id = j.value("updatedBy", "");
  auto updatedAt = j.value("updatedAt", "");
  if (updatedAt != "") {
    entity.updatedAt = TimeUtilities::ConvertStringToTime(updatedAt);
  }
}

#endif  // MUTABLE_ENTITY_H
