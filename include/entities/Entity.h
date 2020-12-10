#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * @class Entity
 * @brief A generalized Entity from which our other
 * entities will inherit from
 */
class Entity {
 public:
  /**
   * Constructor for an Entity object
   * @return an instantiation of an Entity object
   */
  Entity() {}

  /**
   * Destructor for an Entity Object
   */
  virtual ~Entity() {}

  /**
   * ID which can uniquely identify any Entity object
   */
  std::string id = "";
};

#endif  // ENTITY_H
