#ifndef VOTE_H
#define VOTE_H

#include <ctime>
#include <string>
#include "MutableEntity.h"

/**
 * @class Vote
 * Model class for a vote
 */
class Vote : public MutableEntity {
 public:
  Vote() {}
  virtual ~Vote() {}

  /**
   * Overloaded assignment operator for creating a Vote from a MutableEntity
   * @param entity the Entity to compare to
   */
  Vote& operator=(const MutableEntity& entity) {
    // self-assignment guard
    if (this == &entity) {
      return *this;
    }

    // set the fields
    createdAt = entity.createdAt;
    createdBy = entity.createdBy;
    updatedAt = entity.updatedAt;
    updatedBy = entity.updatedBy;
    id = entity.id;
    return *this;
  }

  /**
   * the id of the Issue
   */
  std::string issueId;
};

/**
 * Serializes a Vote object as a nlohmann::json object, as defined by the
 * [nlohmann::json
 * documentation](https://github.com/nlohmann/json/blob/develop/README.md#arbitrary-types-conversions)
 * @param j the nlohmann::json object the output the results to
 * @param vote the Vote to be serialized
 */
inline void to_json(json& j, const Vote& vote) {
  j = static_cast<MutableEntity>(vote);
  j["issueId"] = vote.issueId;
}

/**
 * De-serializes a Vote object to a nlohnmann::json object, as defined by the
 * [nlohmann::json
 * documentation](https://github.com/nlohmann/json/blob/develop/README.md#arbitrary-types-conversions)
 * @param j the nlohman::json out the Vote is being created from
 * @param vote the Vote the result will be stored in
 */
inline void from_json(const json& j, Vote& vote) {
  auto entity = j.get<MutableEntity>();
  vote = entity;
  vote.issueId = j.at("issueId").get<std::string>();
}

#endif
