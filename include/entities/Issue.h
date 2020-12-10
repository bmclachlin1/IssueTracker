#ifndef Issue_H
#define Issue_H

#include <ctime>
#include <set>
#include <string>
#include <vector>

#include "Comment.h"
#include "MutableEntity.h"
#include "User.h"
#include "Vote.h"

/**
 * @class Issue
 * @brief Model class for an issue
 */
class Issue : public MutableEntity {
 public:
  Issue() {}
  virtual ~Issue() {}

  Issue& operator=(const MutableEntity& entity) {
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
   * the title of the Issue
   */
  std::string title;

  /**
   * the status of the Issue
   */
  std::string status;

  /**
   * the User who is assigned the Issue
   */
  User assignedTo;

  /**
   * the User reporting the Issue
   */
  User reporter;

  /**
   * the comments associated with the Issue
   */
  std::multiset<Comment> comments;

  /**
   * the votes associated with the Issue
   */
  std::vector<Vote> votes;
};

/**
 * Serializes a Issue object as a nlohmann::json object, as defined by the
 * [nlohmann::json
 * documentation](https://github.com/nlohmann/json/blob/develop/README.md#arbitrary-types-conversions)
 * @param j the nlohmann::json object the output the results to
 * @param issue the Issue to be serialized
 */
inline void to_json(json& j, const Issue& issue) {
  j = static_cast<MutableEntity>(issue);
  j["title"] = issue.title;
  j["status"] = issue.status;
  j["reporter"] = issue.reporter.id;
  j["assignedTo"] = issue.assignedTo.id;

  auto jsonCommentIds = json::array();
  auto jsonVoteIds = json::array();
  for (auto comment : issue.comments) {
    jsonCommentIds.push_back(comment.id);
  }
  for (auto vote : issue.votes) {
    jsonVoteIds.push_back(vote.id);
  }

  j["comments"] = jsonCommentIds;
  j["votes"] = jsonVoteIds;
}

/**
 * De-serializes a Issue object to a nlohnmann::json object, as defined by the
 * [nlohmann::json
 * documentation](https://github.com/nlohmann/json/blob/develop/README.md#arbitrary-types-conversions)
 * @param j the nlohman::json out the Issue is being created from
 * @param issue the Issue the result will be stored in
 */
inline void from_json(const json& j, Issue& issue) {
  User reporter;
  User assignedTo;
  std::multiset<Comment> comments;
  std::vector<Vote> votes;
  auto entity = j.get<MutableEntity>();
  issue = entity;
  issue.title = j.value("title", "");
  issue.status = j.value("status", "");

  issue.reporter = reporter;
  issue.assignedTo = assignedTo;
  issue.assignedTo.id = j.value("assignedTo", "");
  issue.reporter.id = j.value("reporter", "");

  if (j.find("comments") != j.end()) {
    for (auto commentId : j.at("comments")) {
      Comment comment;
      comment.id = commentId;
      comments.insert(comment);
    }
  }
  issue.comments = comments;

  if (j.find("votes") != j.end()) {
    for (auto voteId : j.at("votes")) {
      Vote vote;
      vote.id = voteId;
      votes.push_back(vote);
    }
  }

  issue.votes = votes;
}

#endif
