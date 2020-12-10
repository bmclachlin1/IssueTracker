#ifndef ISSUE_CONTROLLER_H
#define ISSUE_CONTROLLER_H

#include <restbed>

#include <functional>
#include <memory>

#include "EntityController.hpp"
#include "Issue.h"
#include "IssueService.h"

using std::placeholders::_1;

/**
 * @class IssueController
 * @brief Realization of EntityController to allow interactions with Issue
 * entites
 */
template <class Session = restbed::Session>
class IssueController : public EntityController<Issue, Session> {
 public:
  IssueController() : EntityController<Issue, Session>("/issues", nullptr) {}

  explicit IssueController(const std::shared_ptr<IssueService>& issueService)
      : EntityController<Issue, Session>("/issues", issueService) {
    // Initialize the resource paths to /issues and /issues/:id
    this->resource->set_paths({
        this->_endpoint,                    /* /issues      */
        this->_endpoint + "/{id:[a-z0-9]*}" /* /issues/:id  */
    });
    // Set method handler for GET requests
    this->resource->set_method_handler(
        "GET", std::bind(&IssueController::Get, this, _1));
    // Set method handler for PUT requests
    this->resource->set_method_handler(
        "PUT", std::bind(&IssueController::Update, this, _1));
    // Set method handler for DELETE requests
    this->resource->set_method_handler(
        "DELETE", std::bind(&IssueController::Delete, this, _1));
    // Set method handler for POST requests
    this->resource->set_method_handler(
        "POST", std::bind(&IssueController::Create, this, _1));
  }

  virtual ~IssueController() {}
};

#endif  // ISSUE_CONTROLLER_H
