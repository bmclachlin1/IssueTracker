#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include <restbed>

#include <functional>
#include <memory>

#include "EntityController.hpp"
#include "User.h"
#include "UserService.h"
#include "Utilities.h"

// To allow for placeholder parameters in bound method templates
using std::placeholders::_1;

/**
 * @class UserController
 * @brief Realization of EntityController to allow interactions with User
 * entites
 */
template <class Session = restbed::Session>
class UserController : public EntityController<User, Session> {
 public:
  /**
   * Default constructor. Sets the endpoint to /users and created a new
   * UserService. Leaves the internal restbed::Resource uninitialized
   */
  UserController()
      : EntityController<User, Session>("/users",
                                        std::make_shared<UserService>()) {}

  /**
   * Constructor. Sets the endpoint to /users, and initializes the internal
   * restbed::Resource appropriately
   * @param userService the UserService to serve and modify User entities
   */
  explicit UserController(const std::shared_ptr<UserService>& userService)
      : EntityController<User, Session>("/users", userService) {
    // Initialize the resource paths to /users and /users/:id
    this->resource->set_paths(
        {this->_endpoint, this->_endpoint + "/{id:[a-z0-9]*}"});
    // Set method handler for GET requests
    this->resource->set_method_handler(
        "GET", std::bind(&UserController::Get, this, _1));
    // Set method handler for PUT requests
    this->resource->set_method_handler(
        "PUT", std::bind(&UserController::Update, this, _1));
    // Set method handler for DELETE requests
    this->resource->set_method_handler(
        "DELETE", std::bind(&UserController::Delete, this, _1));
    // Set method handler for POST requests
    this->resource->set_method_handler(
        "POST", std::bind(&UserController::Create, this, _1));
  }
  virtual ~UserController() {}
};

#endif  // USERCONTROLLER_H
