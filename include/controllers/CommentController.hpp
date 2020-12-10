#ifndef COMMENT_CONTROLLER_H
#define COMMENT_CONTROLLER_H

#include <restbed>

#include <functional>
#include <memory>
#include <string>

#include "Comment.h"
#include "CommentService.h"
#include "EntityController.hpp"

using std::placeholders::_1;

/**
 * @class CommentController
 * @brief Realization of EntityController to allow interactions with Comment
 * entites
 */
template <class Session = restbed::Session>
class CommentController : public EntityController<Comment, Session> {
 public:
  CommentController()
      : EntityController<Comment, Session>(
            "/comments", std::make_shared<CommentService>()) {}
  explicit CommentController(
      const std::shared_ptr<CommentService>& commentService)
      : EntityController<Comment, Session>("/comments", commentService) {
    // Initialize the resource paths
    std::string issuePath = "/issues/{issueId:[a-z0-9]*}/";
    std::string idPath = "/{id:[a-z0-9]*}";
    this->resource->set_paths({
        this->_endpoint,                     /* /comments */
        this->_endpoint + idPath,            /* /comments/:id */
        issuePath + this->_endpoint,         /* /issues/:issueId/comments */
        issuePath + this->_endpoint + idPath /* /issues/:issueId/comments/:id */
    });
    // Set method handler for GET requests
    this->resource->set_method_handler(
        "GET", std::bind(&CommentController::Get, this, _1));
    // Set method handler for PUT requests
    this->resource->set_method_handler(
        "PUT", std::bind(&CommentController::Update, this, _1));
    // Set method handler for DELETE requests
    this->resource->set_method_handler(
        "DELETE", std::bind(&CommentController::Delete, this, _1));
    // Set method handler for POST requests
    this->resource->set_method_handler(
        "POST", std::bind(&CommentController::Create, this, _1));
  }
  virtual ~CommentController() {}
};

#endif  // COMMENT_CONTROLLER_H
