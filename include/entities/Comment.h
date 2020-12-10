#ifndef Comment_H
#define Comment_H

#include <ctime>
#include <string>
#include "MutableEntity.h"
#include "Utilities.h"

/**
 * @class Comment
 * @brief Model class for a comment
 */
class Comment : public MutableEntity {
 public:
  Comment() {}
  virtual ~Comment() {}

  /**
   * Overloaded assignment operator for creating a Comment from a MutableEntity
   * @param entity the Entity to compare to
   */
  Comment& operator=(const MutableEntity& entity) {
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
   * Comparison operator, compares the createdAt fields of two Comments.
   * @returns true if the lvalue Comment was created strictly before the rvalue
   * Comment
   */
  bool operator<(const Comment& rhs) const {
    return TimeUtilities::DateComparator(createdAt, rhs.createdAt);
  }

  /**
   * the id of the Issue
   */
  std::string issueId;

  /**
   * the contents of the comment
   */
  std::string body;
};

/**
 * Serializes a Comment object as a nlohmann::json object, as defined by the
 * [nlohmann::json
 * documentation](https://github.com/nlohmann/json/blob/develop/README.md#arbitrary-types-conversions)
 * @param j the nlohmann::json object the output the results to
 * @param comment the Comment to be serialized
 */
inline void to_json(json& j, const Comment& comment) {
  j = static_cast<MutableEntity>(comment);
  j["issueId"] = comment.issueId;
  j["body"] = comment.body;
}

/**
 * De-serializes a Comment object to a nlohnmann::json object, as defined by the
 * [nlohmann::json
 * documentation](https://github.com/nlohmann/json/blob/develop/README.md#arbitrary-types-conversions)
 * @param j the nlohman::json out the Comment is being created from
 * @param comment the Comment the result will be stored in
 */
inline void from_json(const json& j, Comment& comment) {
  auto entity = j.get<MutableEntity>();
  comment = entity;
  comment.issueId = j.at("issueId").get<std::string>();
  comment.body = j.value("body", "");
}

#endif
